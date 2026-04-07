# 搜索引擎项目 v5 - 技术总结

## 目录
1. [项目整体架构](#1-项目整体架构)
2. [离线预处理层](#2-离线预处理层)
3. [在线服务层](#3-在线服务层)
4. [核心算法详解](#4-核心算法详解)
5. [并发网络架构](#5-并发网络架构)
6. [缓存架构设计](#6-缓存架构设计)
7. [C++ 语法与设计要点](#7-c++-语法与设计要点)
8. [内存管理与安全](#8-内存管理与安全)
9. [并发编程要点](#9-并发编程要点)
10. [面试高频问题](#10-面试高频问题)

---

## 1. 项目整体架构

### 1.1 架构分层

项目分为两大层次：

```
┌─────────────────────────────────────────────────────────────┐
│                    在线服务层 (Online Server)                 │
│  用户请求 → HTTP解析 → 缓存查询 → 倒排索引搜索 → 返回结果  │
├─────────────────────────────────────────────────────────────┤
│                    离线预处理层 (Offline Preprocess)          │
│  RSS网页解析 → 去重 → 分词 → 统计词频 → 构建倒排索引 →  计算TF-IDF │
└─────────────────────────────────────────────────────────────┘
```

**离线预处理层职责：**
- 读取RSS格式的网页语料
- 网页清洗提取正文
- Simhash算法进行文章去重
- 使用cppjieba中文分词
- 去停用词
- 统计词频，生成词典和前缀索引
- 计算TF-IDF权重，构建倒排索引
- 归一化文档权重用于余弦相似度计算
- 将结果写入数据文件

**在线服务层职责：**
- Reactor模式处理网络IO
- 线程池并行处理用户请求
- 双缓存LRU一级内存缓存 + Redis二级缓存
- 处理 `/suggest` 搜索推荐（前缀匹配 + 编辑距离排序）
- 处理 `/search` 全文搜索（倒排索引 + TF-IDF + 余弦相似度排序）
- 返回JSON格式搜索结果

### 1.2 文件结构

```
searchs/v5/
├── bin/                    # 编译产物
│   ├── offline             # 离线预处理可执行文件
│   ├── online              # 在线服务可执行文件
│   ├── test_edit_distance  # 编辑距离单元测试
│   └── test_double_lru     # 双缓存LRU单元测试
├── include/                # 头文件
│   ├── cppjieba/           # cppjieba分词库
│   ├── dict/               # jieba词典文件
│   ├── nlohmann/           # JSON库
│   ├── simhash/            # simhash去重
│   ├── Acceptor.h          # 连接接收器
│   ├── Dictonary.h         # 词典单例
│   ├── DoubleLruCache.h    # 双缓存LRU缓存
│   ├── Echoserver.h        # 服务器主类
│   ├── EventLoop.h         # Reactor事件循环
│   ├── HTTPRequest.h       # HTTP请求解析
│   ├── HTTPRequestHandler.h # 请求处理器
│   ├── HTTPResponse.h      # HTTP响应
│   ├── InetAddress.h       # 网络地址封装
│   ├── NonCopyable.h       # 禁止拷贝基类
│   ├── ProcessFile.h       # 文件处理、编辑距离
│   ├── RedisSingleton.h    # Redis单例（thread_local）
│   ├── RSS.h               # RSS解析
│   ├── Socket.h            # socket封装
│   ├── SocketIO.h          # socket读写
│   ├── TaskQueue.h         # 任务队列（线程池）
│   ├── TcpConnection.h     # TCP连接
│   ├── TcpServer.h         # TCP服务器
│   ├── ThreadPool.h        # 线程池
│   ├── webPage.h           # 网页结构体
│   └── webPageHandler.h    # 网页处理、倒排索引
├── src/                    # 源文件
├── tests/                  # 测试脚本
├── data/                   # 预处理生成的数据文件
├── pageyuliao/             # 原始RSS网页语料
├── yuliao/                 # 停用词文件
└── simhash-1.2.1/          # simhash库
```

---

## 2. 离线预处理层

### 2.1 处理流程

```
1. 读取RSS XML文件
   ↓
2. 解析XML，提取网页标题、正文、链接
   ↓
3. 保存到page1.dat
   ↓
4. 生成偏移表，Simhash去重
   ↓
5. 去重后得到new_offset.dat和new_webpages.dat
   ↓
6. 遍历所有网页分词，去停用词
   ↓
7. 计算TF-IDF权重，构建倒排索引
   ↓
8. 归一化权重，保存到index_webpages.dat
```

### 2.2 代码类结构

**TextProcessor** (`ProcessFile.h/cc`)
- 功能：处理词典文件，生成词频和前缀索引
- 核心方法：
  - `preProcess()` - 预处理中英文词典
  - `editDistance(string s1, string s2)` - 计算最小编辑距离

**RSS** (`RSS.h/cc`)
- 功能：读取RSS目录，解析XML，清洗网页
- 核心方法：
  - `readFromDirectory()` - 遍历目录读取所有RSS文件
  - `parse()` - 使用tinyxml2解析XML
  - `store()` - 存储清洗后的网页数据

**webPageHandler** (`webPageHandler.h/cc`)
- 功能：生成偏移表，去重，构建倒排索引
- 核心静态成员：
  - `offsetlib` - 文档偏移表（全局）
  - `newnewindex` - 倒排索引（全局）
- 核心方法：
  - `getoffsetlib()` - 读取偏移表，Simhash去重
  - `generateIndex()` - 生成倒排索引，计算TF-IDF
  - `getwebPage()` - 根据docid读取网页内容
  - `loadOffsetlib()` - 加载偏移表
  - `loadIndex()` - 加载倒排索引

**Dictonary** (`Dictonary.h/cc`) - 单例模式
- 功能：加载词典前缀索引，供搜索推荐使用
- 核心成员：
  - `_dict` - 词 → 词频
  - `_index` - 字符 → 包含该字符的词id集合
- 核心方法：
  - `getInstance()` - 获取单例
  - `getCandidateWords()` - 根据前缀获取候选词
  - `getWord()` - 根据id获取词

---

## 3. 在线服务层

### 3.1 请求处理流程

```
用户发送HTTP请求
    ↓
epoll检测到可读事件
    ↓
accept新连接 → TcpConnection → 添加到epoll
    ↓
读取请求 → 解析HTTP → 封装任务
    ↓
任务放入线程池任务队列
    ↓
工作线程取出任务 → HTTPRequestHandler处理
    ↓
   ┌─────────────┐
   │ /suggest    │  前缀匹配 → 编辑距离排序 → JSON结果
   │ /search     │  倒排索引查找 → TF-IDF余弦排序 → JSON结果
   └─────────────┘
    ↓
回发HTTP响应
    ↓
关闭连接
```

### 3.2 网络类层次

**Echoserver** (`Echoserver.h/cc`) - 服务器主类
- 构造器：`Echoserver(int threadNum, int timeout, string ip, int port)`
- 启动方法：`start()` - 初始化并启动事件循环

**TcpServer** (`TcpServer.h/cc`)
- 持有Acceptor和EventLoop

**Acceptor** (`Acceptor.h/cc`)
- 功能：监听端口，accept新连接
- 核心方法：`accept()`

**EventLoop** (`EventLoop.h/cc`) - Reactor
- 功能：epoll主循环，分发事件
- 核心成员：
  - `_epollfd` - epoll文件描述符
  - `_eventList` - epoll_event数组
- 核心方法：
  - `loop()` - 主循环
  - `updateEpoll()` - 添加/修改epoll监听

**TcpConnection** (`TcpConnection.h/cc`)
- 功能：一个TCP连接的封装
- 核心方法：
  - `handleRead()` - 读取请求
  - `handleWrite()` - 发送响应

**ThreadPool** (`ThreadPool.h/cc`)
- 功能：生产者消费者任务队列，线程池
- 核心成员：
  - `_taskQueue` - 任务队列
  - `_pool` - 线程数组
- 核心方法：
  - `addTask()` - 添加任务
  - `workerFunc()` - 工作线程函数

**HTTPRequestHandler** (`HTTPRequestHandler.h/cc`)
- 功能：处理具体的搜索请求
- 核心方法：
  - `onGet()` - 处理GET（没用）
  - `onPost()` - 处理POST请求
  - `handleSuggest()` - 处理搜索推荐
  - `handleSearch()` - 处理全文搜索
  - `getResultFromCacheOrBackend()` - 查询缓存

---

## 4. 核心算法详解

### 4.1 Simhash 文档去重

**原理：局部敏感哈希，相似文档得到相似指纹**

**步骤：**
```
1. 分词，得到每个词和权重
2. 对每个词计算64bit哈希
3. 对每一位：
   - 如果该位是1，加上权重
   - 如果该位是0，减去权重
4. 遍历完所有词后，每位结果>0则该位为1，否则为0
5. 得到最终64bit指纹
6. 判断两篇文章是否重复：计算海明距离，≤3认为重复
```

**优化：大规模去重O(N²) → 近似线性**
```
性质：如果海明距离 ≤ 3，则最高8位一定相同
算法：按最高8位分为2^8=256个桶，只需要和同一个桶内的文章比较
时间复杂度从O(N²) → O(N × 平均桶大小)
```

**代码位置：** `webPageHandler::getoffsetlib()`

### 4.2 TF-IDF 权重计算

**TF - 词频**：词在文档中出现的次数

**IDF - 逆文档频率**：
```
IDF = log(N / (DF + 1)) + 1
N = 总文档数
DF = 包含该词的文档数
```

**权重 = TF × IDF**

**意义：**
- 常见停用词：DF大 → IDF小 → 权重小
- 稀有词：DF小 → IDF大 → 权重大

**代码位置：** `webPageHandler::generateIndex()`

### 4.3 倒排索引

**结构：** `词 → [(docid, weight), ...]`

**正排：文档→词，倒排：词→文档**
- 用户搜索是"根据词找文档"，倒排索引更适合

**存储格式：**
```
每个词占一行
词
  docid weight
  docid weight
  ...
下一个词...
```

**代码位置：** `webPageHandler::newnewindex` - `unordered_map<string, set<pair<int, double>>>`

### 4.4 余弦相似度排序

**原理：** 将查询和文档都表示为向量，余弦相似度越大越相似

**公式：**
```
相似度 = (Q · D) / (|Q| × |D|)
Q: 查询向量，每个维度是查询词的TF-IDF
D: 文档向量，每个维度是文档中该词的TF-IDF
```

**优化：** 文档权重已经预先归一化，`|D| = 1`，所以只需要计算 `Q · D / |Q|`

**代码位置：** `HTTPRequestHandler::handleSearch()`

### 4.5 最小编辑距离（搜索推荐）

**定义：** 将字符串s1转换成s2最少需要多少次增删改操作

**动态规划：**
`dp[i][j]` = s1[0..i-1] 和 s2[0..j-1] 的最小编辑距离

**递推公式：**
```
边界: dp[i][0] = i, dp[0][j] = j
如果 s1[i-1] == s2[j-1]:
  dp[i][j] = dp[i-1][j-1]
否则:
  dp[i][j] = min(
    dp[i-1][j],     // 删除s1[i-1]
    dp[i][j-1],     // 插入s2[j-1]
    dp[i-1][j-1]    // 替换
  ) + 1
```

**应用：** 用户输入前缀，对所有候选词计算和用户输入的编辑距离，越小越相似，排在前面

**代码位置：** `TextProcessor::editDistance()`

### 4.6 前缀搜索推荐

**步骤：**
```
1. 构建前缀索引：_index[char] = set<word_id>
   每个字符对应所有包含该字符的词id
2. 用户输入前缀，比如 "se":
   - 第一个字符's'，得到所有包含's'的词id集合A
   - 第二个字符'e'，得到所有包含'e'的词id集合B
   - 结果 = A ∩ B，得到同时包含's'和'e'的词
3. 对交集结果，按编辑距离从小到大排序
4. 返回前N个候选词给用户
```

**空间换时间：预处理建立索引，查询只需集合交集**

**代码位置：** `Dictonary::getCandidateWords()`

### 4.7 LRU 缓存淘汰

**数据结构：双向链表 + 哈希表**

```
双向链表：按访问顺序保存
  - 表头：最近访问
  - 表尾：最久未访问
哈希表：O(1)查找节点

操作：
- 访问命中：移动到表头
- 插入：放到表头，如果容量满了删除表尾
```

**代码位置：** `ThreadLocalCache` in `DoubleLruCache.h`

---

## 5. 并发网络架构

### 5.1 Reactor 模式

**设计思想：** 分离IO和计算
```
主线程（Reactor）：
- 只负责epoll监听IO事件
- 有事件到来就把任务分发到线程池

工作线程：
- 执行具体计算（搜索、缓存查询等）
- 完成后回发响应

优点：
- 主线程IO不阻塞，避免锁竞争
- 线程池利用多核并行计算
```

**代码位置：** `EventLoop::loop()`

### 5.2 epoll 边缘触发

**水平触发LT vs 边缘触发ET：**
- LT：只要socket缓冲区有数据，epoll每次循环都会通知
- ET：只有当缓冲区从不可读变为可读时（新数据到来）才通知一次

**为什么ET性能更好：**
- 不会重复通知已经可读但没读完的socket，减少系统调用次数
- 使用ET必须把socket设为非阻塞，循环read直到 `recv() 返回-1且errno == EAGAIN`

**代码：** `EventLoop::addToEpoll()` 使用 `EPOLLET | EPOLLONESHOT`

### 5.3 线程池设计

**生产者消费者模型：**
- 主线程是生产者，添加任务到队列
- 工作线程是消费者，从队列取任务执行
- 一个互斥锁 + 一个条件变量同步

**代码位置：** `ThreadPool` + `TaskQueue`

### 5.4 一步一步连接建立过程

```
1. 服务器启动
   - 创建listenfd → 绑定地址 → 监听
   - 添加listenfd到epoll

2. 客户端连接
   - epoll检测到listenfd可读
   - accept → 得到clientfd
   - 创建TcpConnection → 添加clientfd到epoll

3. 客户端发送请求
   - epoll检测到clientfd可读
   - TcpConnection::handleRead() 读取请求
   - 解析HTTP → 封装成任务 → 添加到线程池

4. 工作线程处理
   - 取出任务 → 调用HTTPRequestHandler处理
   - 查询缓存 → 如果不命中计算结果 → 写入缓存
   - 组装HTTP响应 → TcpConnection::send()

5. 发送响应
   - epoll检测到clientfd可写
   - TcpConnection::handleWrite() 发送数据
   - 发送完成 → 关闭连接
```

---

## 6. 缓存架构设计

### 6.1 二级缓存架构

```
用户查询
    ↓
一级缓存：DoubleLRU 内存缓存（每个线程私有）
    ↓ 命中？ → 是 → 直接返回
    ↓ 否
二级缓存：Redis 网络缓存
    ↓ 命中？ → 是 → 写入一级缓存 → 返回
    ↓ 否
计算倒排索引/编辑距离 → 写入一级和二级 → 返回
```

### 6.2 Double LRU 设计思想

**需求背景：**
- 一级内存缓存，减少Redis访问
- 读多写少，要尽量减少锁竞争，提高读性能

**双缓存设计：**
```
每个线程：
  working缓存 ← 所有读都在这里，完全无锁！
  sync缓存    ← 接收新数据同步

全局：
  shared_buffer ← 所有新写入先放到这里，需要锁

同步流程（每个线程自己做，访问时检查是否需要同步）：
1. 拷贝 working → sync
2. 把 shared_buffer 所有新数据插入到 sync
3. 原子交换 working 和 sync
4. 现在 working 就是最新版本

为什么这么设计读性能好：
- 读操作完全在working上，不需要加锁！
- 只有同步的时候需要加一次锁，同步是批量的，减少锁竞争
- 每个线程私有缓存，没有伪共享问题
```

**对比其他方案：**
| 方案 | 缺点 | 双缓存优点 |
|------|------|-----------|
| 分段锁 | 每次访问都要加锁，竞争多 | 读完全无锁 |
| RCU读写锁 | 读不加锁，但退占需要同步，实现复杂 | 实现简单 |
| thread-local每个线程一份 | 内存重复占用 | 每个线程一份，但总的缓存容量可控 |

**代码位置：** `DoubleLruCache.h/cc`

**关键代码解析：**

```cpp
// 每个线程的双缓存
struct ThreadDoubleCache {
    ThreadLocalCache working;  // 工作缓存：读都在这里
    ThreadLocalCache sync;     // 同步缓存：接收新数据
};

// thread_local每个线程独立
static thread_local std::unique_ptr<ThreadDoubleCache> thread_cache_;

// 查询：完全无锁
bool DoubleLruCache::get(const std::string& key, json& out_value) {
    ThreadDoubleCache& td = getThreadCache();
    checkSync(td);  // 检查是否需要同步（有新数据就同步）
    return td.working.get(key, out_value);  // 直接读working，无锁！
}

// 写入：需要锁，写入全局shared_buffer
void DoubleLruCache::put(const std::string& key, const json& value) {
    std::lock_guard<std::mutex> lock(global_mutex_);
    shared_buffer_[key] = value;
    cv_.notify_one();
}
```

**本次修复的bug：**
1. 原来不判断shared_buffer是否为空，不到时间不同步 → 修复：有数据就必须同步
2. 原来交换后清空sync → 错误，sync应该保留原来的working数据 → 修复：不清空，下次拷贝working到sync再添加新数据
3. 原来拷贝working顺序颠倒 → LRU顺序错了 → 修复：逆序遍历保持顺序
4. 缺少reset → 单元测试状态不干净 → 添加reset方法

### 6.3 Redis 并发安全解决

**问题：** hiredis 的 `redisContext` **不是线程安全**，多个线程不能共用同一个连接

**解决方案：thread_local 单例模式**

```cpp
static thread_local RedisSingleton instance;
```

- 每个线程创建自己独立的Redis连接
- 每个线程终身使用这个连接
- 完全不需要加锁，天生线程安全
- 空间换时间，并发性能好

**代码位置：** `RedisSingleton.h/cc`

---

## 7. C++ 语法与设计要点

### 7.1 单例模式的几种写法

**Dictonary 词典单例：**
```cpp
static Dictonary* getInstance() {
    static Dictonary instance;
    return &instance;
}
```
- 优点：C++11保证静态局部变量初始化线程安全
- 缺点：无法销毁，但是项目中词典全程需要，没问题

**DoubleLruCache 缓存单例：**
```cpp
static DoubleLruCache& getInstance() {
    static DoubleLruCache instance;
    return instance;
}
```
- 返回引用，和指针用法差不多，更安全

**RedisSingleton：**
```cpp
static thread_local RedisSingleton instance;
```
- 每个线程一个实例，thread_local存储时长

### 7.2 禁止拷贝继承

```cpp
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
```
- 使用=delete显示删除拷贝构造和赋值
- 继承这个类，子类也无法拷贝

**应用：** Acceptor、EventLoop、TcpServer等，这些类不该被拷贝

### 7.3 默认成员函数 =default

```cpp
DoubleLruCache() = default;
```
- 显式要求编译器生成默认构造函数
- 如果你声明了其他构造，编译器就不会生成默认构造，需要可以加=default

### 7.4 智能指针使用

| 智能指针 | 使用场景 | 代码位置 |
|---------|---------|---------|
| `std::unique_ptr` | 独占所有权 | `thread_cache_ = std::make_unique<ThreadDoubleCache>()` |
| `std::shared_ptr` | 共享所有权 | `std::shared_ptr<Dictonary> d = Dictonary::getInstance();` |

### 7.5 原子变量 `std::atomic`

```cpp
std::atomic<bool> running_{false};
std::atomic<long long> last_sync_time_;
```
- 原子操作，不需要加锁就能线程安全访问
- 用于标志位、计数器

### 7.6 `thread_local` 关键字

```cpp
static thread_local std::unique_ptr<ThreadDoubleCache> thread_cache_;
```
- 每个线程拥有独立的实例，存储在线程特定存储
- 每个线程访问自己的副本，不需要锁
- 项目中用于：每个线程独立的缓存、每个线程独立的Redis连接

### 7.7 范围for循环

```cpp
for (auto& pair : shared_buffer_) {
    td.sync.insert(pair.first, pair.second);
}
```
- 比for(;;)简洁，不容易错

### 7.8 移动语义

```cpp
std::swap(td.working, td.sync);
```
- swap使用移动语义，高效交换两个容器，不需要拷贝所有元素

### 7.9 右值引用和std::move
- 项目中多处使用标准库容器，自动使用移动

### 7.10 lambda表达式

```cpp
auto worker = [&cache, num_ops](int id) {
    for (int i = 0; i < num_ops; ++i) {
        // ...
    }
};
```
- 捕获列表可以捕获变量，简洁的匿名函数
- 单元测试中用于并发测试

### 7.11 nullptr 代替 NULL
- 类型安全，不会和整数混淆

### 7.12 override 关键字
```cpp
void handleRead() override;
```
- 显示标记重写基类虚函数，编译器检查

### 7.13 using namespace std; 争议
- 项目在cpp中使用，是可以接受的，面试项目代码简洁优先

### 7.14 前向声明 vs 包含头文件
- 头文件中尽量前向声明，减少编译依赖
- cpp中再包含头文件

---

## 8. 内存管理与安全

### 8.1 本次修复的栈溢出bug

**原代码：**
```cpp
char buffer[offlength + 1];  // offlength 可以很大，分配在栈上
```
- 变长数组是C99特性，不是标准C++
- 栈空间一般只有几MB，文档大的时候栈溢出崩溃

**修复：**
```cpp
std::vector<char> buffer(offlength + 1);  // 动态分配到堆上
```
- 标准C++，安全可靠
- 自动释放，不会内存泄漏

**代码位置：** `webPageHandler::getwebPage()`

### 8.2 内存泄漏防范
- 使用RAII（资源获取即初始化）
- 标准容器（vector、map、unordered_map等）自动管理
- 智能指针自动释放
- 项目中没有手动malloc/free，所以不会泄漏

### 8.3 多线程内存可见性
- 共享变量使用 `std::atomic`
- 锁提供内存栅栏，保证可见性

---

## 9. 并发编程要点

### 9.1 互斥锁 `std::mutex`
```cpp
std::lock_guard<std::mutex> lock(global_mutex_);
```
- RAII自动加锁解锁，不会忘了解锁
- `lock_guard` 是RAII封装，比手动lock/unlock安全

### 9.2 条件变量 `std::condition_variable`
```cpp
std::condition_variable cv_;
cv_.wait_for(lock, std::chrono::seconds(sync_interval_), [this] {
    return !running_ || !shared_buffer_.empty();
});
```
- 用于线程间等待通知
- 生产者消费者模型中，队列空消费者等待

### 9.3 线程安全总结

| 模块 | 线程安全措施 |
|------|-------------|
| DoubleLruCache读 | 每个线程working缓存私有，完全无锁 |
| DoubleLruCache写 | 全局锁保护shared_buffer |
| Redis连接 | thread_local每个线程一个，无锁 |
| 倒排索引 | 只读，启动后不修改，天然线程安全 |
| 词典 | 只读，启动后不修改，天然线程安全 |

**项目特点：** 读多写少，大部分数据启动加载后只读，所以并发性能很好

### 9.4 生产者消费者模型
```cpp
// 生产者（主线程）
void addTask(function<void()> f) {
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _taskQueue.push(f);
    }
    _cond.notify_one();
}

// 消费者（工作线程）
function<void()> take() {
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.wait(lock, [this] { return !_taskQueue.empty(); });
    auto task = _taskQueue.front();
    _taskQueue.pop();
    return task;
}
```
**代码位置：** `TaskQueue.h/cc`

---

## 10. 面试高频问题

### Q1: 为什么用倒排索引？
**A：**
- 倒排索引是"词 → 文档"映射，正排是"文档 → 词"
- 用户搜索是"根据词找文档"，倒排更适合，可以快速找出包含这个词的所有文档
- 同时存储TF-IDF权重，方便计算相似度排序

### Q2: TF-IDF 原理，公式？
**A：**
- **TF（词频）**：词在当前文档中出现的次数
- **IDF（逆文档频率）**：`log(总文档数 / (包含该词的文档数 + 1)) + 1`
  - 一个词出现在越少文档中，IDF越大，区分度越高
- **权重 = TF × IDF**

### Q3: 余弦相似度原理？
**A：**
- 查询表示为向量Q，文档表示为向量D，每个维度是词的TF-IDF权重
- 相似度 = (Q·D) / (|Q| × |D|)
- 结果越大越相似，按相似度降序排序返回

### Q4: Simhash 去重原理？为什么能处理大规模文档？
**A：**
- Simhash是局部敏感哈希，将文章映射为64bit指纹
- 步骤：分词 → 每个词hash → 加权累加 → 得到64bit指纹
- 海明距离 ≤ 3认为重复
- 优化：最高8位分桶，只需要同桶比较，时间复杂度O(N²)→O(N×平均桶大小)

### Q5: 搜索推荐（suggest）怎么做？前缀搜索怎么优化？
**A：**
- 预处理建立前缀索引：每个字符对应所有包含该字符的词id
- 用户输入前缀，每个字符找索引求交集，得到候选词
- 对候选词计算最小编辑距离，按编辑距离升序排序返回

### Q6: 最小编辑距离DP公式？
**A：**
`dp[i][j]` 表示s1[0..i-1]和s2[0..j-1]的最小编辑距离
```
边界: dp[i][0] = i, dp[0][j] = j
如果 s1[i-1] == s2[j-1]:
  dp[i][j] = dp[i-1][j-1]
否则:
  dp[i][j] = min(dp[i-1][j], dp[i][j-1], dp[i-1][j-1]) + 1
// dp[i-1][j]: 删除，dp[i][j-1]: 插入，dp[i-1][j-1]: 替换
```

### Q7: 为什么设计双缓存LRU？原理？
**A：**
- 需求：一级内存缓存减少Redis访问，读多写少，要减少锁竞争
- 设计：每个线程两个缓存 working（服务查询）+ sync（接收同步），全局shared_buffer收集新写入
- 定期同步：每个线程自己将shared_buffer拷贝到sync，交换working和sync
- 读操作都在working上，**完全无锁**，只有同步需要一次加锁
- 相比分段锁：每次访问不需要加锁，锁竞争大大减少

### Q8: LRU 原理？数据结构？
**A：**
- 双向链表 + 哈希表
- 双向链表按访问顺序：表头最近访问，表尾最久未访问
- 哈希表O(1)查找节点
- 访问命中：移动到表头
- 插入：放到表头，容量满了删除表尾

### Q9: Redis 并发问题怎么解决？
**A：**
- hiredis的redisContext不是线程安全，多线程不能共用
- 解决方案：**thread_local单例**，每个线程创建自己的连接
- 每个线程终身使用这个连接，完全不需要加锁，天生线程安全

### Q10: epoll 水平触发和边缘触发区别？为什么边缘触发性能更好？
**A：**
- 水平触发LT：只要缓冲区有数据，每次epoll_wait都会通知
- 边缘触发ET：只有缓冲区从不可读变可读（新数据到来）才通知一次
- ET性能更好：不会重复通知，减少系统调用次数
- ET必须用非阻塞socket，循环read直到EAGAIN

### Q11: Reactor 模式是什么？这里怎么用的？
**A：**
- Reactor是事件驱动的并发模式：主线程只负责监听IO事件，有事件就分发任务给线程池
- 分离IO和计算：主线程处理IO，工作线程处理计算
- 这里：主线程epoll监听，有请求交给线程池处理，计算完回发

### Q12: 项目中哪里用了分治？哪里用了空间换时间？
**A：**
- 分治：Simhash分桶去重，原问题分解为子问题，只在桶内比较
- 空间换时间：
  1. 前缀索引：预处理建索引，空间换查询时间
  2. 缓存：内存缓存换Redis网络IO时间
  3. 倒排索引：预建立词→文档映射，空间换查询时间

### Q13: 停用词作用是什么？
**A：**
- 停用词是"的、了、吗"这种高频无实际意义的词
- 作用：
  1. 减少索引大小，节省空间
  2. 减少计算量，提高搜索速度
  3. 避免无意义的词影响TF-IDF权重，提高搜索准确性

### Q14: 项目亮点是什么？
**A：**
1. **性能优化**：Simhash分桶去重，O(N²)→近线性时间
2. **缓存架构**：双缓存LRU设计，读完全无锁，热点查询微秒级返回
3. **并发安全**：thread_local每个线程一个Redis连接，解决hiredis非线程安全问题，完全无锁并发
4. **架构清晰**：离线和在线分离，启动快，代码结构清晰
5. **健壮性**：修复了栈溢出等严重bug，代码更安全

---

## 11. 编译和使用

```bash
# 编译
make clean
make offline
make online

# 运行离线预处理（如果需要重新生成数据）
./bin/offline

# 启动在线服务
./bin/online

# 测试
curl -X POST http://localhost:8888/search -H "Content-Type: application/json" -d '{"data":"中国"}'
curl -X POST http://localhost:8888/suggest -H "Content-Type: application/json" -d '{"data":"中"}'

# 运行单元测试
./bin/test_edit_distance
./bin/test_double_lru

# 运行性能测试
python3 tests/benchmark.py

# 运行并发测试
python3 tests/test_concurrent.py --concurrency 16 --requests 1000
```

---

## 11. 已修复bug列表

| ID | 问题 | 严重等级 | 状态 |
|----|------|----------|------|
| P1-001 | 硬编码绝对路径，项目移动位置找不到文件 | 🔴 严重 | ✅ 已修复 |
| P1-002 | getwebPage使用栈变长数组，大文档栈溢出 | 🔴 严重 | ✅ 已修复 |
| P1-003 | DoubleLruCache不到时间不即使有数据也不同步 | 🔴 严重 | ✅ 已修复 |
| P1-004 | DoubleLruCache同步后清空导致旧数据全部丢失 | 🔴 严重 | ✅ 已修复 |
| P1-005 | DoubleLruCache拷贝LRU顺序颠倒，淘汰错误 | 🔴 严重 | ✅ 已修复 |
| P1-006 | DoubleLruCache缺少reset，单元测试状态不干净 | 🟡 轻微 | ✅ 已修复 |
| P1-007 | 倒排索引加载格式不匹配只加载1个词 | 🔴 严重 | ✅ 已修复 |
| P2-001 | 停用词路径混用不一致 | 🟠 中等 | ✅ 已修复 |
| P2-002 | 变长数组不是标准C++，兼容性差 | 🟡 轻微 | ✅ 已修复 |

总计：发现9个问题，修复9个问题，其中严重问题6个全部修复。
