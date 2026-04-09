/**
 * @file C++基础知识面试必背.cc
 * @brief C++ 面试高频基础知识整理，每个知识点带代码示例
 */

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <algorithm>

// ============================================================================
// 1. const 可以修饰哪些内容
// ============================================================================

/*
const 可以修饰：
- 普通变量
- 指针 → 常量指针 vs 指针常量
- 引用
- 函数参数
- 成员函数
- 返回值
- 类对象
*/

namespace const_examples {

// 1. const 修饰普通变量
const int MAX_VALUE = 100;  // 全局常量，不可修改

void example() {
    const int a = 10;  // 局部常量，不可修改
    // a = 20;  // 编译错误！不能修改const变量
}

// 2. 常量指针 vs 指针常量
// ----------------------------------------------------------------------------

// ✅ 常量指针：指针指向的内容不能改，指针本身可以改指向别的地方
//    const 在 * 左边 → 修饰 *p → 内容不可变
const int* p1 = &MAX_VALUE;
// *p1 = 200;  // 错误！*p1是const
p1 = &a;          // 没问题，指针本身可以改

// ✅ 指针常量：指针本身不能改，指向的内容可以改
//    const 在 * 右边 → 修饰指针变量p本身 → 指针不可变
int* const p2 = const_cast_cast<int*>(&MAX_VALUE);
*p2 = 200;  // 没问题，内容可以改
// p2 = &a;  // 错误！p2本身是const，不能改指向

// 口诀：左数右指 → const在*左边是*（数=内容）const，const在*右边是指针本身const

// 3. const 修饰指针的引用
//int* const& ref = p;  // 引用本身已经不能改，const就是修饰指针本身

// 4. const 修饰函数参数
void func(const std::string& s) {
    // s 不能修改，避免拷贝 → 最常用写法
    // 既保证不修改，又避免拷贝开销
}

// 5. const 修饰成员函数
class Example {
    int x = 0;
public:
    // const 成员函数：承诺不会修改类的任何成员变量（除了mutable）
    int get_x() const {  // 成员函数后面加const
        //x = 10;  // 错误！const成员函数不能修改成员变量
        return x;
    }
    void set_x(int val) {
        x = val;
    }
};

// 6. const 修饰返回值
const std::string get_name() {
    return "test";
}  // 返回值是const，不能修改

// 7. const 修饰类对象
const Example obj;
//obj.set_x(10);  // 错误！const对象只能调用const成员函数，不能调用非const成员函数

} // namespace const_examples

// ============================================================================
// 2. 数组指针 vs 指针数组
// ============================================================================
namespace array_pointer_examples {

// ✅ 数组指针：p是一个指针，指向一个数组
// 语法：(*p)，因为[]优先级比*高，所以必须加括号
int (*p_arr)[10];  // p_arr 是指针，指向 int[10] 这个数组

// 使用：
int arr[10] = {1, 2, 3, 4, 5};
p_arr = &arr;  // p_arr指向arr数组
(*p_arr)[0] = 100;  // 访问第一个元素

// ✅ 指针数组：arr是一个数组，数组每个元素都是指针
int* arr_p[10];  // arr_p 是数组，每个元素是 int*
// 因为[]优先级高，所以先组合成数组，每个元素是指针

// 使用：
int a = 1, b = 2, c = 3;
arr_p[0] = &a;
arr_p[1] = &b;
arr_p[2] = &c;
// arr_p[0] 就是指针，指向a

// 记忆："数组指针" = 指针 指向 数组；"指针数组" = 数组 存 指针

} // namespace array_pointer_examples

// ============================================================================
// 3. 函数指针 vs 指针数组（函数指针数组）
// ============================================================================
namespace function_pointer_examples {

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }

// ✅ 函数指针：f is a pointer to function
int (*fp)(int, int);  // fp 是函数指针，指向 返回int，参数两个int 的函数

fp = add;  // 函数名就是地址，可以直接赋值
int result = fp(3, 4);  // result = 7

// ✅ 函数指针数组：数组，每个元素是函数指针
// 常见：跳转表、命令处理
int (*fparr[2])(int, int) = {add, sub};
result = fparr[0](3, 4);  // 7
result = fparr[1](3, 4);  // -1

// 可以用 typedef 简化：
typedef int (*CalcFunc)(int, int);
CalcFunc funcs[] = {add, sub};

} // namespace function_pointer_examples

// ============================================================================
// 4. 四种 C++ 强制转换
// ============================================================================
namespace cast_examples {

/*
1. static_cast
   - 用途：普通类型转换（int→double，void*→具体类型指针，父类指针→子类指针（向上转换，编译期检查））
   - 编译期检查，没有运行时开销
   - 不能去掉const，不能做不同无关类型指针转换
*/

void static_cast_example() {
    double d = 3.14;
    int i = static_cast<int>(d);  // 3 → 正常转换

    void* vp = malloc(100);
    int* ip = static_cast<int*>(vp);  // void* 转具体类型指针
}

/*
2. dynamic_cast
   - 用途：多态类型向下转换（父类指针/引用 → 子类指针/引用），运行时检查类型
   - 只有类有虚函数才能用，失败返回 nullptr（指针）或抛bad_cast异常（引用）
   - 有运行时开销（需要查RTTI信息）
*/

class Base {
public:
    virtual ~Base() = default;  // 必须有虚函数才能用dynamic_cast
    virtual void hello() {}
};
class Derived : public Base {
public:
    void derived_func() {}
};

void dynamic_cast_example(Base* b) {
    Derived* d = dynamic_cast<Derived*>(b);
    if (d != nullptr) {
        // 确实是Derived，安全调用
        d->derived_func();
    } else {
        // 不是Derived，处理错误
    }
}

/*
3. const_cast
   - 唯一用途：去掉const属性，修改变量
   - 不做其他转换，专门用来去const
*/

void const_cast_example(int x) {
    const int a = x;
    int* p = const_cast<int*>(&a);
    *p = 100;  // 去掉const后可以修改
}

/*
4. reinterpret_cast
   - 最暴力的转换：任何指针→整数，不同类型指针互相转，函数指针互相转
   - 不做任何检查，完全相信程序员
   - 非常危险，除非底层编程，一般不用
*/

void reinterpret_cast_example() {
    int x = 100;
    int* p = &x;
    // 指针转整数
    uintptr_t addr = reinterpret_cast<uintptr_t>(p);
    std::cout << "x 地址是: " << std::hex << addr << std::endl;

    // 不同类型指针转
    char* cp = reinterpret_cast<char*>(p);
}

} // namespace cast_examples

// ============================================================================
// 5. 内联函数 vs 宏定义
// ============================================================================

/*
区别总结：
|                | 内联函数         | 宏定义             |
|----------------|------------------|-------------------|
| 什么时候处理   | 编译阶段展开     | 预处理阶段文本替换 |
| 类型检查       | 有，编译检查参数 | 没有，纯文本替换   |
| 副作用         | 不会有额外副作用 | 参数多次计算可能有副作用 |
| 作用域         | 遵循正常作用域   | 文本替换，到处生效 |

inline 只是建议编译器展开，编译器可以不听（函数太大还是不展开）
*/

// 宏定义例子，有什么问题？
#define MAX(a, b) ((a) > (b) ? (a) : (b))
// 问题：如果参数是表达式，会多次计算
// MAX(x++, y) → x++ 会被计算两次，结果不对

// 内联函数版本
inline int inline_max(int a, int b) {
    return a > b ? a : b;
}
// 没问题，参数只计算一次，有类型检查，安全

// 结论：内联函数比宏安全，能用内联就不用宏

// ============================================================================
// 6. 引用 vs 指针
// ============================================================================

/*
核心区别：
- 引用：变量的别名，必须初始化，不能为空，绑定后不能换绑定
- 指针：存储地址的变量，可以为空，可以改变指向

底层：引用一般也是用指针实现（编译器帮你解引用）
*/

namespace ref_vs_pointer {

// 引用作为函数参数：避免拷贝，比指针干净
void swap(int& a, int& b) {  // 直接用，不需要解引用
    int tmp = a;
    a = b;
    b = tmp;
}

// 指针版本
void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int main() {
    int x = 1, y = 2;
    swap(x, y);  // 直接传，不用&，干净

    // 引用不能为空，必须初始化
    // int& r;  // 编译错误！引用必须初始化
    int a = 10;
    int& r = a;  // ✓ 正确，r是a的别名

    // 指针可以空，可以改指向
    int* p = nullptr;  // ✓ 没问题
    p = &a;  // ✓ 可以改指向
    p = &x;  // ✓ 再改

    // 结论：
    // - 需要指向"一定存在"的变量，用引用，语法更干净安全
    // - 需要"可以为空"或者"需要改变指向"，用指针
}

} // namespace ref_vs_pointer

// ============================================================================
// 7. 内存分配方式
// ============================================================================

/*
C++ 内存分为五个区域：

1. **栈区 (stack)**
   - 存放：局部变量、函数参数、返回地址、帧指针
   - 分配释放：编译器自动管理，函数调用分配，返回自动释放
   - 大小：固定大小，Linux一般几MB到十几MB，不能太大
   - 生长方向：向下（高地址→低地址）
   - 速度：快，连续内存，只需要移动栈指针

2. **堆区 (heap)**
   - 存放：动态分配的对象（new/malloc出来的）
   - 分配释放：程序员手动管理（C++还是靠智能指针自动）
   - 大小：几乎不限（虚拟地址空间很大），可以分配大块
   - 生长方向：向上（低地址→高地址）
   - 速度：比栈慢，需要管理空闲块，容易产生内存碎片

3. **全局/静态存储区**
   - 存放：全局变量、static变量（不管是全局static还是函数内static）
   - 生命周期：程序启动分配，程序结束才释放，整个运行周期都在

4. **文字常量区**
   - 存放：字符串常量、const修饰的全局常量
   - 只读，程序结束释放

5. **程序代码区**
   - 存放：二进制机器指令
   - 只读，程序结束释放

对比堆和栈：
|          | 栈                | 堆                      |
|----------|-------------------|--------------------------|
| 管理方式 | 编译器自动分配释放 | 程序员手动管理（或智能指针） |
| 大小     | 有限，不能太大     | 几乎不限                |
| 生长方向 | 向下（高→低）      | 向上（低→高）            |
| 碎片     | 几乎没有碎片       | 容易产生内存碎片          |
| 速度     | 快                 | 慢                       |
*/

// ============================================================================
// 8. 面向对象四大特性
// ============================================================================

/*
四大特性：抽象、封装、继承、多态

### 1. 抽象 (Abstraction)
- 把现实事物抽取出核心属性和行为，去掉不重要的细节
- 比如 "动物" 抽象出 "吃、跑"，不管皮肤颜色这些无关细节
- 代码层面就是 "类" 把核心接口暴露出来，隐藏内部实现

### 2. 封装 (Encapsulation)
- 把数据和操作打包成一个类，隐藏内部实现细节，只暴露必要接口给外部
- 通过访问权限控制：`private` 成员只能类内部访问，`public` 接口给外部
- 好处：模块化，内部改了不影响外部，防止内部被错误修改

示例：
*/

class EncapsulationExample {
private:
    // private：只有这个类自己能访问，外部看不见
    int internal_data;
    void internal_helper() {}

public:
    // public：暴露给外部的接口
    int get_data() const {
        return internal_data;
    }
    void set_data(int x) {
        internal_data = x;
    }
};

/*
### 3. 继承 (Inheritance)
- 子类继承父类的属性和方法，可以复用已有代码，在此基础上扩展
- 实现 "is-a" 关系：Dog is a Animal

### 4. 多态 (Polymorphism)
- 同一个调用，不同对象表现出不同行为
- C++ 多态指：通过基类指针/引用调用虚函数，实际运行时调用到派生类的重写版本
- 条件：
  1. 基类要有虚函数
  2. 派生类重写基类的虚函数
  3. 通过基类指针/引用调用

原理：虚函数表 + 虚指针，详细见后面虚函数章节
*/

// ============================================================================
// 9. class vs struct 区别
// ============================================================================

/*
C++ 中唯一区别：

- struct 默认访问权限是 public
- class 默认访问权限是 private

继承的时候也一样：
- struct 默认继承是 public
- class 默认继承是 private

除此之外没有区别，struct也能有成员函数，也能有构造析构，也能继承。

惯例用法：
- struct 用来放纯数据，不需要封装，比如配置、结构体数据
- class 用来做面向对象，需要封装，有成员函数有private数据
*/

struct StructExample {
    int x;  // 默认public
    void func() {}  // 默认public
};

class ClassExample {
    int x;  // 默认private
    void func() {}  // 默认private
};

// 继承权限默认：
struct A : B {  // 默认public继承
};
class C : D {  // 默认private继承
};

// 总结：语法只有默认权限区别，其他都一样，看你习惯用哪个

// ============================================================================
// 10. 静态数据成员 & 静态成员函数
// ============================================================================

namespace static_member {

class Example {
public:
    // 1. 静态数据成员：属于类，不属于任何对象，整个类只有一份
    static int count;  // ✓ 类内声明

    // 静态数据成员不占用对象内存！sizeof(Example)不包含count

    // 2. 静态成员函数：属于类，不属于任何对象，没有 this 指针
    static int get_count() {
        // 不能访问非静态成员变量和非静态成员函数！因为没有this
        return count;
    }

    // 静态成员函数可以调用静态成员变量/其他静态成员函数，没问题
};

// ✅ 静态数据成员必须 类内声明，类外初始化
int Example::count = 0;  // 这里定义，分配内存，在全局静态区

// 使用方式：
void use() {
    Example::get_count();        // ✓ 类名::直接调用
    Example obj;
    obj.get_count();            // ✓ 也可以通过对象调用，一样的
}

/*
总结：
- 静态数据：类只有一份，存在全局静态区，不占用对象大小
- 静态函数：没有this指针，不能访问非静态成员
- 用途：计数、单例、工厂方法、工具函数（不需要访问对象状态）

常见坑：
- 忘记类外初始化 → 链接错误：undefined reference
- 头文件里定义静态变量 → 每个编译单元一份，重复定义 → 应该头文件声明，cpp文件初始化
*/

} // namespace static_member

// ============================================================================
// 11. 虚函数 & 虚表 & 动态多态实现
// ============================================================================

/*
### 什么是虚函数？
- 成员函数前加 virtual 关键字，就是虚函数，支持动态多态

### 什么是虚表（vtable）？
- 每个有虚函数的类，编译器会生成一张 **虚函数表**，存这个类所有虚函数的地址
- 每个对象的首地址，会有一个 **虚指针（vptr）** 指向这个类的虚表
- 虚表整个类只有一份，所有对象共享同一个虚表

### 动态多态怎么激活？（派生类重写后怎么找到派生类版本？）

1. 派生类继承基类，会继承基类的虚函数
2. 如果派生类重写了基类的虚函数，派生类的虚表里，对应位置会替换成派生类自己的函数地址
3. 对象构造的时候，虚指针会被设置成指向派生类自己的虚表
4. 当用基类指针指向派生对象，调用虚函数时：
   - 通过对象的虚指针 → 找到虚表
   - 从虚表里查到函数地址 → 调用的就是派生类重写的版本
5. 这就是运行时动态绑定，根据实际对象类型调用对应版本，就是多态

代码示例：
*/

namespace virtual_function {

class Base {
public:
    virtual void func() {
        std::cout << "Base::func()" << std::endl;
    }
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    void func() override {  // override 关键字标记重写，C++11推荐
        std::cout << "Derived::func()" << std::endl;
    }
};

void example() {
    Derived d;
    Base* b = &d;       // 基类指针指向派生对象
    b->func();         // ✅ 调用 Derived::func() → 多态生效
}

/*
内存布局（64位系统）：
- Derived对象开头 8字节 → vptr 指向 Derive::vtable
- Derive::vtable 里面：第一个条目就是 &Derived::func()
- 调用 b->func() 时： *vptr → 找func地址 → 调用 Derived::func

总结：
- 每个有虚函数的类 → 一张虚表
- 每个对象 → 一个虚指针 → 指向类的虚表
- 重写 → 虚表里对应位置替换地址
- 调用 → 虚指针找虚表 → 找地址 → 调用
*/

} // namespace virtual_function

// ============================================================================
// 12. 重载、重写、重定义（隐藏）区别
// ============================================================================

/*
| 名称   | 发生位置 | 特点 |
|--------|----------|------|
| **重载 (overload)** | 同一个类中 | 同名函数，参数列表（个数/类型）不同，编译期根据参数选哪个，和多态无关 |
| **重写 (override)** | 基类 + 派生类 | 派生类重写基类的**虚函数**，函数签名（名字+参数）必须一样，运行时根据实际类型调用，多态 |
| **重定义/隐藏 (redefine/hide)** | 基类 + 派生类 | 派生类定义了和基类**同名**的函数，不管参数是否一样，基类的同名函数在派生类中都被隐藏，看不到了 |

代码示例：
*/

namespace overload_override_hide {

class Base {
public:
    void func(int x) {
        std::cout << "Base::func(int)" << std::endl;
    }
    virtual void vfunc(float f) {
        std::cout << "Base::vfunc(float)" << std::endl;
    }
};

class Derived : public Base {
public:
    // 1. ✅ 重写：完全匹配虚函数，override
    void vfunc(float f) override {
        std::cout << "Derived::vfunc(float)" << std::endl;
    }

    // 2. ✅ 重定义（隐藏）：同名，参数不同，隐藏了Base::func(int)
    void func(float f) {
        std::cout << "Derived::func(float)" << std::endl;
    }
    // 现在：Derived d; d.func(10); → 编译错误！因为Base的func(int)被隐藏了
};

/*
总结：
- 重载：同范围，同名不同参 → 编译期选
- 重写：不同范围（基类+派生类），虚函数，签名相同 → 运行时多态
- 隐藏：不同范围，同名 → 派生类隐藏基类同名函数
*/

} // namespace overload_override_hide

// ============================================================================
// 13. 哪些函数不能是虚函数？为什么？
// ============================================================================

/*
### 1. 构造函数不能是虚函数
原因：
- 构造函数执行的时候，对象还没构造完成，虚指针还没正确初始化好（这时候虚指针还是指向基类的虚表）
- 如果构造函数允许虚函数，基类构造函数调用虚函数，如果是派生类重写的，这时候派生类还没构造完，调用会访问没构造的成员，不安全
- 编译器直接禁止构造函数是虚函数

### 2. 静态成员函数不能是虚函数
原因：
- 静态成员函数没有 this 指针，不属于对象，属于类
- 虚函数需要对象的虚指针找虚表，静态成员不需要对象，所以没办法
- 编译器禁止

### 3. 内联函数不能是虚函数？
- 其实语法上允许，但内联要编译时展开，虚函数是运行时动态绑定，所以一般内联函数不会是虚函数，实际项目也很少这么用

### 4. 友元函数不能是虚函数
- 友元不是类的成员函数，没有 this 指针，虚函数依赖类成员的虚指针，所以不能是虚函数

总结：构造、静态、内联、友元 → 一般都不能做虚函数，主要原因就是虚函数依赖对象的虚指针，这些函数不满足条件
*/

// ============================================================================
// 14. 为什么析构函数要设为virtual？解决什么问题？
// ============================================================================

/*
问题场景：
当你用 **基类指针 delete 派生类对象** 时，如果析构不是virtual，只会调用基类析构，派生类析构不会调用 → 派生类的资源没释放 → 内存泄漏！

示例：
*/

namespace virtual_destructor {

class BadBase {
public:
    ~BadBase() {
        // 不是virtual
    }
};

class BadDerived : public BadBase {
private:
    int* data = new int[100];
};

void bad_example() {
    BadBase* b = new BadDerived();
    delete b;  // ❌ 只调用 ~BadBase()，~BadDerived() 没调用 → data 泄漏！
}

// ✅ 正确做法：基类析构设为virtual
class GoodBase {
public:
    virtual ~GoodBase() = default;  // virtual 析构
};

class GoodDerived : public GoodBase {
private:
    int* data = new int[100];
public:
    ~GoodDerived() override {
        delete[] data;  // ✓ 会被正确调用
    }
};

void good_example() {
    GoodBase* b = new GoodDerived();
    delete b;  // ✓ 通过虚表找到 ~GoodDerived()，调用，再调用 ~GoodBase() → 正确，没有泄漏
}

/*
结论：
- 只要这个类会被当做基类用来多态，就把析构设为virtual
- 只要类有虚函数，析构就设为virtual，不会错
- 纯虚析构也要有实现（哪怕是空实现），因为派生类析构调用基类析构，需要定义

最佳实践：如果要多态，基类析构必须virtual，否则一定会出现内存泄漏。
*/

} // namespace virtual_destructor

// ============================================================================
// 15. 左值 vs 右值
// ============================================================================

/*
简单判断标准：
- **左值**：有名字，可以取地址，表达式结束后还存在
- **右值**：没名字，不能取地址，是临时值，表达式结束就销毁

示例：
*/

namespace lvalue_rvalue {

int x = 10;  // x 是左值 → 有名字，&x 合法
int y = x + 1;  // x 是左值，x+1 是右值

int& get_ref() { static int a; return a; }
int&& get_rval() { return std::move(x); }

// 区分：
// - x 是左值
// - x++ 是右值（先用后加，返回原来值的拷贝，临时）
// - ++x 是左值（返回x引用，修改x，返回x引用）

bool is_lvalue = std::is_lvalue_reference<decltype(x)>::value;

/*
总结：
能取地址的一定是左值，不能取地址的是右值
*/

} // namespace lvalue_rvalue

// ============================================================================
// 16. 右值引用 && 移动语义
// ============================================================================

/*
### 什么是右值引用？
- 用 `&&` 声明，专门绑定右值（临时对象、将要销毁的对象）
- 目的：允许我们"窃取"临时对象的资源，避免不必要的深拷贝，提升性能

### 为什么需要移动语义？
- C++98/03 中，函数返回对象、vector插入对象，都会拷贝临时对象，明明临时对象马上就要销毁了，还要深拷贝一次，浪费性能
- 移动语义：把临时对象手里的资源直接抢过来给新对象，不用拷贝，O(1)搞定

代码示例：
*/

namespace rvalue_ref_move {

// 简单的动态数组实现，展示移动语义
class MyVector {
private:
    int* data_;
    size_t size_;
public:
    MyVector(size_t size) : data_(new int[size]), size_(size) {}

    // 拷贝构造
    MyVector(const MyVector& other) {
        size_ = other.size_;
        data_ = new int[size_];
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
        std::cout << "深拷贝" << std::endl;
    }

    // ✅ 移动构造：参数是右值引用，绑定要被销毁的右值
    MyVector(MyVector&& other) noexcept
        : data_(other.data_), size_(other.size_) {  // 直接把指针拿过来！不用拷贝
        other.data_ = nullptr;  // 把原对象指针置空，析构不释放
        std::cout << "移动：O(1)，不用拷贝" << std::endl;
    }

    ~MyVector() {
        delete[] data_;
    }

    // 同样，移动赋值也一样
    MyVector& operator=(MyVector&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
        }
        return *this;
    }
};

// 使用：
MyVector create_vector(size_t n) {
    MyVector v(n);
    return v;  // 返回局部变量，是右值
}

void example() {
    // 没有移动语义的时候：
    // create_vector 返回v → 拷贝到外面临时对象 → v销毁
    // 临时对象再拷贝给myvec → 两次深拷贝
    // 有了移动语义：直接移动，一次深拷贝都不用！

    MyVector myvec = create_vector(1000);  // ✓ 调用移动构造，没有拷贝，超快
}

/*
### std::move 是干什么的？
std::move 不移动任何东西，它只做一件事：**把左值强制转换成右值引用**，让编译器选择移动构造/移动赋值。

比如：
```cpp
MyVector a = ...;
MyVector b = a;  // 拷贝构造
MyVector c = std::move(a);  // a 是左值，转成右值 → 调用移动构造，a 现在空了
```

结论：std::move = 转为右值引用，给你移动的机会，不移动本身不做任何操作。

### 引用折叠 & 完美转发

引用折叠规则：模板参数推导时，引用的引用会折叠：
- X& &  → X&  (左值引用叠左值引用 → 左值引用)
- X& && → X&  (左值引用叠右值引用 → 左值引用)
- X&& & → X&  (右值引用叠左值引用 → 左值引用)
- X&& && → X&& (右值引用叠右值引用 → 右值引用)

完美转发用途：在模板函数中，保留参数原来的值类别（左值还是右值），不丢失，让编译器能正确重载。

写法：
```
template<typename T>
void func(T&& t) {  // 万能引用，能接受左值也能接受右值
    other_func(std::forward<T>(t));  // std::forward 按原来的值类别转发
}
```

作用：比如工厂函数，创建对象的时候，参数完美转发，左值传过去还是左值，右值传过去还是右值，正确调用移动构造等。
*/

} // namespace rvalue_ref_move

// ============================================================================
// 17. 四种智能指针 区别 & 使用注意
// ============================================================================

/*
### 核心思想：RAII → 资源获取即初始化
- 把指针包装成栈对象，析构自动释放，避免忘记delete，异常安全
- 不同智能指针区别主要在 **所有权** 管理

### 1. std::auto_ptr (C++98，已废弃)
- 设计：独占所有权，但拷贝的时候会把所有权从原指针转移到新指针，原指针置空
- 问题：语义太诡异，拷贝后原指针悄悄变空，一不小心用了就崩溃
- C++11 后废除，用 unique_ptr 代替

示例问题：
```cpp
std::auto_ptr<int> p1(new int(10));
std::auto_ptr<int> p2 = p1;  // p1 现在变成 nullptr 了！你不注意就用p1 → 崩溃
*/

/*
### 2. std::unique_ptr (独占所有权)
- 核心：**独占**对象所有权，同一个时间只能有一个unique_ptr指向对象
- 禁用拷贝构造/拷贝赋值（=delete），只能移动（std::move转移所有权）
- 没有引用计数，开销和裸指针一样，没有额外开销
- 推荐优先用，比shared_ptr轻，除非你真需要共享所有权

代码：
*/

namespace unique_ptr_example {

std::unique_ptr<int> p1 = std::make_unique<int>(10);  // ✓ C++14 make_unique，推荐
std::unique_ptr<int> p2 = p1;  // ❌ 编译错误！不能拷贝，独占
std::unique_ptr<int> p3 = std::move(p1);  // ✓ 转移所有权，p1 现在空了
}

/*
### 3. std::shared_ptr (共享所有权)
- 核心：**共享**对象所有权，多个shared_ptr可以指向同一个对象
- 内部有**原子引用计数**，每拷贝一次计数+1，每析构一次计数-1，计数到0自动释放对象
- 线程安全：引用计数更新是原子的，线程安全，但对象本身访问不是线程安全
- 有一点额外开销（原子计数）

代码：
*/

namespace shared_ptr_example {

std::shared_ptr<int> p1 = std::make_shared<int>(10);
std::shared_ptr<int> p2 = p1;  // ✓ 拷贝，计数+1 → 现在计数=2
std::cout << p1.use_count();  // 2
p2.reset();  // 计数-1 → 现在计数=1
// 最后一个shared_ptr析构，计数到0 → 对象释放
}

/*
### 4. std::weak_ptr (弱引用)
- 核心：不增加引用计数，只是观察对象，不影响对象生命周期
- 用来解决 **shared_ptr 循环引用** 问题

循环引用问题：
- A shared_ptr 持有 B，B shared_ptr 持有 A → 两个人互相持有，计数永远到不了0 → 永远不会释放 → 内存泄漏
- 解决：把其中一个换成 weak_ptr，不增加计数，这样当外部没有引用时就能正常释放

代码：
*/

namespace weak_ptr_example {

class Node {
public:
    // ✅ 正确：next 用 weak_ptr 打破循环
    std::weak_ptr<Node> next;
    // ❌ 错误：如果是 std::shared_ptr<Node> next; 循环引用泄漏
    int data;
};

// 使用：
std::weak_ptr<int> wp;
void use_weak() {
    if (auto sp = wp.lock()) {  // lock → 成功说明对象还在，转成shared_ptr用
        // 使用 *sp
    } else {
        // 对象已经被销毁了
    }
}

/*
总结四种智能指针：
| 智能指针 | 所有权 | 引用计数 | 开销 | 使用场景 |
|----------|--------|----------|------|----------|
| unique_ptr | 独占 | 无 | 无，和裸指针一样 | 大多数场景，独占对象，优先用 |
| shared_ptr | 共享 | 有，原子 | 一点点（原子计数） | 需要多个所有者共享对象 |
| weak_ptr | 无 | 无 | 无 | 打破shared_ptr循环引用，观察对象 |
| auto_ptr | 独占 | 无 | 无 | 废弃，别用 |
*/

} // namespace weak_ptr_example

// ============================================================================
// 18. 多线程内存可见性问题：一个线程改变量，另一个线程能立刻看到吗？
// ============================================================================

/*
**答案：不一定！**

原因：
1. **CPU 缓存**：每个CPU核心有自己的L1/L2缓存，修改了可能只在缓存里，没写回内存，其他核心看不到
2. **编译器优化**：编译器可能把变量优化到寄存器，其他线程看不到修改
3. **指令重排**：编译器和CPU可能重排指令执行顺序，导致看起来顺序不对

怎么保证可见？
- `std::atomic<>`：原子变量，每次读写都同步缓存，保证可见性，适合简单标志位
- `std::mutex`：加锁解锁包含内存屏障，保证解锁前的修改对其他加锁线程可见
- `std::condition_variable`：等待/通知也会保证可见性

实际项目：简单计数器、标志位用atomic，复杂共享数据用mutex。
*/

// ============================================================================
// STL 篇
// ============================================================================

// ============================================================================
// 1. vector push_back 扩容原理
// ============================================================================

/*
vector 底层是**连续动态数组**，通常用三个指针维护：
- `_start` → 数组起始
- `_finish` → 最后一个元素的下一个位置 → size = _finish - _start
- `_end_of_storage` → 已分配内存结束 → capacity = _end_of_storage - _start

push_back 执行流程：
1. 如果 `size < capacity` → 直接放进去，finish++，搞定
2. 如果 `size == capacity` → 需要扩容：
   - 通常新容量是原来的 **1.5倍 ~ 2倍**（不同实现不一样，GCC是2倍）
   - 分配一块新的更大的连续内存
   - C++11 之后，如果元素有移动构造，**逐个移动过去**（O(n)但不用拷贝，快），没有移动构造才拷贝
   - 释放旧内存
   - 更新三个指针指向新内存
   - 插入新元素

均摊复杂度：push_back 是 O(1) 均摊，因为不是每次都扩容，扩容n次总移动次数是O(n)，均摊每次O(1)

优化：如果你提前知道要放多少元素，提前 `reserve(n)` 分配好空间，避免多次扩容拷贝，性能更好。

代码示意：
*/

namespace vector_expansion {

template<typename T>
void my_push_back(std::vector<T>& v, T val) {
    if (v.size() == v.capacity()) {
        size_t new_cap = v.capacity() == 0 ? 1 : v.capacity() * 2;
        std::vector<T> new_v;
        new_v.reserve(new_cap);
        for (auto& elem : v) {
            new_v.push_back(std::move(elem));  // 移动，不用拷贝
        }
        // swap 交换，原来的旧vector销毁
        v.swap(new_v);
    }
    v.push_back(val);
}

}

// ============================================================================
// 2. deque 底层实现原理
// ============================================================================

/*
deque 底层是**分段连续数组**（也叫分块存储）：
- 维护一个中心map（其实就是一个数组），每个元素是指针，指向一个固定大小的缓冲区（chunk）
- 每个缓冲区大小固定，比如 512 bytes 或者 1KB
- 这样设计好处：
  1. 头尾插入删除都是 O(1) 均摊：头空了就新分配一个缓冲区放头部，尾满了就新分配一个放尾部，不需要移动已有元素
  2. 不会像vector那样整块扩容移动所有元素，push_back/push_front都快
  3. 支持随机访问，通过map找到缓冲区，再偏移，O(1)

对比：
| 操作 | vector | deque |
|------|--------|-------|
| push_back | 均摊O(1)，扩容要移动 | O(1)，不需要移动 |
| push_front | O(n)，要移动所有元素 | O(1) |
| 随机访问 | O(1)，连续内存快 | O(1)，但多一次map查找，比vector慢点 |
| 中间插入删除 | O(n) | O(n)，比vector更慢 |

适用场景：需要频繁在两端插入删除 → deque 比vector好，比如滑动窗口、生产者消费者队列。
*/

// ============================================================================
// 3. 迭代器失效问题
// ============================================================================

/*
不同容器不同情况：

### vector:
- 扩容（push_back 触发扩容）→ **所有迭代器都失效** → 因为整个内存块换地方了
- insert / erase 在中间 → **插入删除位置及之后**的迭代器都失效 → 因为元素移动了

正确写法：erase 后重新赋值迭代器
```cpp
for (auto it = v.begin(); it != v.end(); ) {
    if (need_delete(*it)) {
        it = v.erase(it);  // ✅ erase 返回下一个迭代器
    } else {
        ++it;
    }
}
```

### deque:
- 头尾插入删除 → 只有被操作的那个迭代器失效，其他迭代器还能用
- 中间插入删除 → 所有迭代器失效

### list / map / unordered_map:
- 删除元素 → 只有指向被删除元素的迭代器失效，其他迭代器有效
- 插入不会导致任何迭代器失效

总结：**只要迭代器失效了，就不能继续用，必须重新获取，否则访问非法内存**
*/

// ============================================================================
// 4. emplace_back vs push_back 区别
// ============================================================================

/*
push_back: 参数是 T 类型对象，你传进去，vector 会：
- 如果是左值 → 拷贝构造到vector内存里
- 如果是右值 → 移动构造到vector内存里

emplace_back: 参数是构造T需要的构造函数参数，vector**直接在vector内存里调用构造函数构造**，不需要临时对象，不需要拷贝/移动

区别：
- push_back: 需要你先构造一个对象，再拷贝/移动进去
- emplace_back: 原地构造，省了临时对象和一次拷贝/移动，更高效

示例：
*/

struct Person {
    std::string name;
    int age;
    Person(std::string n, int a) : name(std::move(n)), age(a) {}
};

std::vector<Person> v;

// push_back 写法：需要构造临时Person，再移动进去
v.push_back(Person("tom", 20));  // 临时对象 → 移动构造

// emplace_back 写法：直接传构造参数，原地构造，没有临时对象
v.emplace_back("tom", 20);  // ✓ 直接在vector内存里构造Person，更高效

// 结论：能用工emplace_back就用emplace_back，尤其是对象较大或者构造参数多，能省不少开销

// ============================================================================
// 5. (无序)关联容器底层实现
// ============================================================================

/*
### std::unordered_map / std::unordered_set
- 底层：**哈希表**（数组 + 链表/开放寻址解决冲突），GCC用拉链法（数组每个桶是链表）
- 平均查找/插入/删除都是 O(1)，最坏所有元素哈希冲突到一个桶，退化成O(n)
- 无序：元素顺序不按key排序，遍历顺序和插入顺序无关
- 优点：平均速度快，O(1)查找；缺点：无序，缓存不如红黑树友好

### std::map / std::set
- 底层：**红黑树**（平衡二叉搜索树）
- 查找/插入/删除都是 O(log n)，稳定
- 有序：元素按key从小到大排序，可以顺序遍历，找上下边界
- 优点：有序，时间复杂度稳定O(log n)；缺点：比unordered_map慢点

### std::unordered_multimap / std::unordered_multiset
- 底层和unordered_map一样，还是哈希表
- 允许同一个key对应多个value（multimap），或者允许重复key（multiset）

总结选择：
- 要 O(1) 查找，不需要有序 → unordered_map
- 需要有序，或者稳定 O(log n) → map
*/

// ============================================================================
// 6. 什么是函数对象？STL 中作用？
// ============================================================================

/*
函数对象（functor）就是**重载了 operator() 的类**，实例可以像函数一样调用。

优点：比普通函数灵活，可以携带状态（成员变量存状态），比普通函数方便。

STL算法大量用函数对象作为比较器、操作器。

示例：
*/

// 自定义比较器，按绝对值排序
struct AbsCmp {
    bool ascending;  // 成员变量存状态：升序还是降序
    explicit AbsCmp(bool asc) : ascending(asc) {}

    bool operator()(int a, int b) const {
        if (ascending) {
            return std::abs(a) < std::abs(b);
        } else {
            return std::abs(a) > std::abs(b);
        }
    }
};

// 使用：
void example() {
    std::vector<int> v = {1, -2, 3, -4, 5};
    std::sort(v.begin(), v.end(), AbsCmp(true));  // 按绝对值升序
}

/*
lambda 表达式本质就是编译器生成的函数对象，所以lambda也是函数对象。
*/

// ============================================================================
// 设计模式 高频
// ============================================================================

// ============================================================================
// 1. 单例模式 (C++11 正确写法)
// ============================================================================

// 懒汉式，C++11 静态局部变量，天生线程安全，最简洁
class Singleton {
public:
    // 禁止拷贝
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // 全局访问点
    static Singleton* getInstance() {
        // C++11 标准保证：静态局部变量初始化是线程安全的，只有一个线程会初始化
        static Singleton instance;
        return &instance;
    }

private:
    // 构造私有，禁止外面创建
    Singleton() = default;
};

// 饿汉式：程序启动就创建
// class SingletonHungry {
// private:
//     static SingletonHungry instance;
//     SingletonHungry() = default;
// public:
//     static SingletonHungry* getInstance() { return &instance; }
// };
// SingletonHungry SingletonHungry::instance;

// 优点：饿汉天生线程安全，但是程序启动就创建，不管用不用都占空间

// ============================================================================
// 2. 观察者模式
// ============================================================================

/*
定义：定义对象间一对多依赖关系，当一个对象（被观察者）状态改变时，所有依赖它的对象（观察者）都会自动收到通知更新。

角色：
- Subject（被观察者）：维护观察者列表，提供 attach / detach / notify 接口
- Observer（观察者）：接口，update 方法，被通知时更新
- ConcreteSubject：具体被观察者，状态变化时通知
- ConcreteObserver：具体观察者，实现update

优点：松耦合，被观察者和观察者互相依赖少，容易扩展，加新观察者不用改被观察者

示例：文件监控，文件改了，通知所有监听模块更新
*/

class Observer;

class Subject {
public:
    virtual void attach(Observer* obs) = 0;
    virtual void detach(Observer* obs) = 0;
    virtual void notify() = 0;
    virtual ~Subject() = default;
};

class Observer {
public:
    virtual void update() = 0;
    virtual ~Observer() = default;
};

class ConcreteSubject : public Subject {
private:
    std::vector<Observer*> observers_;
    int state_;
public:
    void set_state(int s) {
        state_ = s;
        notify();
    }
    int get_state() const { return state_; }
    void attach(Observer* obs) override {
        observers_.push_back(obs);
    }
    void detach(Observer* obs) override {
        // 移除obs
        observers_.erase(std::remove(observers_.begin(), observers_.end(), obs), observers_.end());
    }
    void notify() override {
        for (auto obs : observers_) {
            obs->update();
        }
    }
};

class ConcreteObserver : public Observer {
private:
    int observer_state_;
    Subject* subject_;
public:
    ConcreteObserver(Subject* subj) : subject_(subj) {}
    void update() override {
        observer_state_ = subject_->get_state();
        // 更新自己...
    }
};

// ============================================================================
// C++ 高级基础知识
// ============================================================================

// ============================================================================
// 1. new/delete vs malloc/free 区别
// ============================================================================

/*
| 区别 | new/delete | malloc/free |
|--------|-----------|-------------|
| 是什么 | C++ 运算符 | C 库函数 |
| 构造析构 | new 分配内存后调用构造函数，delete 先调用析构再释放 | 只分配释放内存，不调用构造析构 |
| 返回类型 | new 返回对应类型指针，不需要强转 | malloc 返回 void*，需要强转 |
| 大小 | new 自动算对象大小 | malloc 要你手动算大小（malloc(n * sizeof(T))） |

总结：new/delete 是malloc/free 基础上加上了对象的构造析构，面向对象必须用new/delete，不要混合用（new完free，malloc完delete错）
*/

// ============================================================================
// 2. RAII 是什么？为什么重要？
// ============================================================================

/*
RAII = Resource Acquisition Is Initialization （资源获取即初始化）

核心思想：**把资源的获取和释放绑定到对象的生命周期**
- 构造函数：获取资源（打开文件、加锁、分配内存）
- 析构函数：自动释放资源

好处：
- 异常安全：不管函数怎么退出，只要对象离开作用域，析构一定调用，资源一定释放，不会泄漏
- 不用手动管理释放，不容易忘

例子：
*/

// 1. lock_guard 封装mutex
void critical_section() {
    std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);  // 构造加锁
    // 操作共享数据...
}  // 离开作用域，lock_guard 析构 → 自动解锁 → 不会忘解锁

// 2. unique_ptr 封装动态内存
void func() {
    std::unique_ptr<int> p = std::make_unique<int>(10);  // 构造分配内存
    // 使用...
}  // 离开作用域，unique_ptr 析构 → 自动delete → 不会泄漏

// 3. 文件描述符封装
struct FileGuard {
    int fd;
    FileGuard(int f) : fd(f) {}
    ~FileGuard() { if (fd >= 0) close(fd); }
    // 禁止拷贝
    FileGuard(const FileGuard&) = delete;
    FileGuard& operator=(const FileGuard&) = delete;
};

// RAII 是C++资源管理的基石，没有RAII就没办法写出安全的C++代码。

// ============================================================================
// 3. 右值引用、移动语义、完美转发 总结
// ============================================================================

/*
- **右值引用 (`T&&`)**：C++11 引入的新引用类型，专门绑定右值（临时对象、将要销毁的对象），为移动语义提供基础
- **移动语义**：通过右值引用绑定临时对象，把临时对象的资源直接转移给新对象，不需要深拷贝，避免不必要的拷贝开销，提升性能。比如vector扩容，返回对象，都能受益。`std::move` 把左值强制转成右值引用，触发移动。
- **完美转发 (`std::forward`)**：在模板函数中，保持参数原来的值类别（左值还是右值），解决万能引用转发时值类别丢失的问题，让编译器能正确重载。比如工厂函数创建对象，参数能正确转发给构造函数，调用正确的重载（拷贝/移动）。

实际项目用：我在项目中用std::move优化vector扩容，减少了很多不必要的深拷贝，性能提升明显。
*/

// ============================================================================
// 4. lambda 表达式 语法 & 捕获
// ============================================================================

/*
语法：
```
[捕获列表] (参数列表) mutable -> 返回类型 { 函数体 }
```

- 捕获列表：决定怎么捕获外部变量，最重要：
  - `[=]`：所有外部变量**值捕获**（拷贝一份，lambda内部用拷贝，修改不影响外面）
  - `[&]`：所有外部变量**引用捕获**（lambda内部用引用，修改影响外面）
  - `[x, &y]`：x 值捕获，y 引用捕获，混合用
  - `[this]`：捕获当前对象指针，成员函数里lambda用
  - `[*this]`：C++17，拷贝整个当前对象

常见坑：
- 引用捕获生命周期：如果lambda生命周期比变量长，变量销毁了lambda还引用，就是悬空引用，崩溃
- 值捕获没问题，但有拷贝开销，看场景选

lambda 本质就是编译器生成的匿名函数对象，重载了operator()，所以本质还是函数对象。

示例：
```cpp
vector<int> v = {1, 3, 5, 2, 4};
int threshold = 3;
// 统计比threshold大的个数
int cnt = count_if(v.begin(), v.end(), [threshold](int x) {
    return x > threshold;
});
```
*/

// ============================================================================
// 5. C++11 标准线程库
// ============================================================================

/*
C++11 之后标准库有了统一线程库，主要组件：

1. `std::thread`：创建管理线程
```cpp
std::thread t(func, arg1, arg2);  // 创建线程，执行func
t.join();   // 等待线程结束，回收资源
t.detach(); // 分离，线程后台运行
```

2. `std::mutex`：互斥锁，保护共享资源
```cpp
std::mutex mtx;
std::lock_guard<std::mutex> lock(mtx);  // RAII 自动加锁解锁，推荐
// 操作共享数据...
```
- `std::unique_lock<std::mutex>`：更灵活，可以提前unlock，配合condition_variable用

3. `std::condition_variable`：条件变量，生产者消费者模型，等待通知
```cpp
// 消费者：
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock, []{ return !queue.empty(); }); // 条件满足就往下走，不满足就等
// 处理数据
// 生产者：放完数据 notify_one()
```

4. `std::future` / `std::promise`：线程间传递异步结果
```cpp
std::promise<int> p;
std::future<int> f = p.get_future();
// 子线程：
p.set_value(42);
// 主线程：
int result = f.get(); // 阻塞等待结果
```

实际项目：我在搜索引擎项目中用 `std::thread` + `std::mutex` + `std::condition_variable` 实现了线程池，处理搜索任务，避免主线程阻塞，提升并发性能。
*/

// ============================================================================
// 6. 五法则 (Rule of Five) C++11 后
// ============================================================================

/*
核心思想：如果你的类需要自定义以下任何一个特殊成员函数，那么五个都要自定义，保证资源管理正确。

五个特殊成员函数：
1. 析构函数 ~T() 释放资源
2. 拷贝构造 T(const T&) 深拷贝
3. 拷贝赋值 T& operator=(const T&) 深拷贝
4. 移动构造 T(T&&) noexcept 转移资源 (C++11+)
5. 移动赋值 T& operator=(T&&) noexcept 转移资源 (C++11+)

为什么？C++11 如果你自定义了析构/拷贝，编译器不会自动生成移动构造和移动赋值，就享受不到移动语义的优化，所以要自己写全。

如果你不需要资源管理（就是纯值类型，没有动态资源），那五个都用编译器默认生成就行，不需要自定义。
*/

// 例子：管理动态数组的类，遵守五法则
class MyArray {
private:
    int* data_;
    size_t size_;
public:
    MyArray(size_t n) : data_(new int[n]), size_(n) {}

    // 五法则：五个都要有
    ~MyArray() { delete[] data_; }

    // 拷贝
    MyArray(const MyArray& other) : size_(other.size_) {
        data_ = new int[size_];
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }
    MyArray& operator=(const MyArray& other) {
        if (this == &other) return *this;
        delete[] data_;
        size_ = other.size_;
        data_ = new int[size_];
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
        return *this;
    }

    // 移动
    MyArray(MyArray&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }
    MyArray& operator=(MyArray&& other) noexcept {
        if (this == &other) return *this;
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
        return *this;
    }
    // 现在 C++11 后就是要这么写，五法则
};

// ============================================================================
// 补充高频基础知识点
// ============================================================================

// ============================================================================
// 1. explicit 关键字
// ============================================================================

/*
explicit 用来修饰**单参数构造函数**，禁止**隐式类型转换**。

为什么需要？防止编译器偷偷帮你做类型转换，得到你不想要的结果。
*/

namespace explicit_example {

// ❌ 没有explicit，编译器可以隐式转换
class MyString {
public:
    MyString(int len) {  // 可以接受int，隐式转换int → MyString
        // 分配长度len的字符串
    }
};

void func(MyString s) {
    // 接受MyString
}

void example() {
    // 你以为会编译错？其实编译器偷偷把 10 隐式转换成 MyString(10) → 编译通过，但这不是你想要的
    func(10);  // ❌ 隐式转换，能过编译，但不对
}

// ✅ 加上explicit，禁止隐式转换
class MyStringExplicit {
public:
    explicit MyStringExplicit(int len) {}
};

void func_explicit(MyStringExplicit s) {}

void example_explicit() {
    // func_explicit(10);  // ✅ 编译错误！提前阻止错误，你必须显式写 MyStringExplicit(10)
    func_explicit(MyStringExplicit(10));  // ✓ 正确，显式构造
}

// 总结：单参数构造函数都建议加explicit，除非你就是想要隐式转换（比如转换运算符）
} // namespace explicit_example

// ============================================================================
// 2. mutable 关键字
// ============================================================================

/*
mutable 关键字：**即使在const成员函数中，也能修改mutable修饰的成员变量**。

用途：
- 缓存：const成员函数要修改缓存，没问题
- 互斥锁：const成员函数要加锁修改锁状态，没问题
- 调试计数：const函数里计数，mutable允许修改
*/

namespace mutable_example {

class Cache {
private:
    mutable int cache_hits_ = 0;  // mutable，const成员也能改
    std::string data_;
public:
    std::string get_data() const {  // const成员函数，承诺不修改成员
        cache_hits_++;  // ✓ mutable，就算const成员函数也能改
        return data_;
    }
    int get_hits() const {
        return cache_hits_;
    }
};

// 结论：mutable 打破const的只读限制，给需要在const函数修改的特殊成员用
} // namespace mutable_example

// ============================================================================
// 3. volatile 关键字
// ============================================================================

/*
volatile 告诉编译器：**这个变量可能被其他线程/硬件改变，不要做优化**，每次读写都必须真的去内存读，不能缓存到寄存器。

用途：
1. 共享变量，会被其他线程修改（但现在一般用atomic，volatile不够）
2. 硬件寄存器，映射到内存，每次读写硬件，必须真读写，不能优化

例子：
*/

volatile bool flag = false;

// 线程1：
void set_flag() {
    flag = true;  // 写入
}

// 线程2：
void wait_flag() {
    while (!flag) {  // 没有volatile，编译器会优化成 if(!flag) while(1); 永远循环，因为它认为flag不会变
        // 等待
    }
}

/*
总结：现在并发编程一般用 std::atomic，比volatile更安全正确。volatile主要用在底层硬件编程，保证不被编译器优化。
*/

// ============================================================================
// 4. sizeof 常见面试题
// ============================================================================

/*
常见问题：
*/

namespace sizeof_example {

// Q: 空类 sizeof 是多少？ → 1字节！不是0！
class EmptyClass {};
// 为什么？每个对象必须有独一无二的地址，所以至少分配1字节，所以sizeof是1

// Q: 有虚函数的类 sizeof 是多少？ → 必须多8字节（64位）存虚指针vptr
class VirtualClass {
    virtual void func() {}
};
// sizeof(VirtualClass) = 8 (64位)，只有虚指针，没其他成员

// Q: 继承空类，派生类sizeof多少？ → 还是1，派生类加自己成员，空基类不占空间（编译器优化空基类优化）
class Derived : public EmptyClass {
    int x;
};
// sizeof(Derived) = 4（64位下还是4，因为空基类优化，基类那1字节优化掉了）

// Q: 64位系统下，指针sizeof多少？ → 所有指针都是8字节！不管指向int还是函数还是什么，都是8字节，引用也是8字节（底层就是指针）
// Q: 32位就是4字节

// 总结：
// - 空类 → 1字节
// - 有虚函数 → 多加sizeof(vptr) = 8字节（64位）
// - 所有指针，不管什么类型，都是8字节（64位）
} // namespace sizeof_example

// ============================================================================
// 5. 多继承 & 菱形继承 & 虚继承
// ============================================================================

/*
### 多继承：一个派生类可以有多个直接基类

### 菱形继承问题：
```
    A
   / \
  B   C
   \ /
    D
```
- B和C都继承A，D继承B和C → D里面有两份A的成员 → **二义性**，访问A成员不知道哪一份
- 地址也浪费，两份相同成员没必要

### 解决：虚继承 → 让最派生类只保存一份共同基类
*/

namespace virtual_inheritance {

class A {
public:
    int a;
};

// B虚继承A，C虚继承A → 只保存一份A
class B : virtual public A {};
class C : virtual public public A {};

class D : public B, public C {
    // 现在D里只有一份A，不会二义性了
};

/*
原理：虚继承会添加一个虚指针，指向共享基类的偏移，这样就能找到共享的那一份，不会有两份。
结论：菱形继承必须用虚继承解决二义性和空间浪费。
*/

} // namespace virtual_inheritance

// ============================================================================
// 6. 拷贝构造函数 什么时候会被调用？
// ============================================================================

/*
四种情况会调用拷贝构造：
1. 用一个对象初始化另一个对象：`T a = b;` where b is T
2. 函数参数按值传递：`void func(T a);` 调用func(b)，b拷贝给a
3. 函数返回值按值返回：`T func() { T obj; return obj; }` 返回局部对象，会拷贝（RVO优化可能优化掉）
4. 插入容器元素：`vector.push_back(obj);` obj拷贝进去

记住：只要需要产生一个**新副本**，就会调用拷贝构造。
*/

// ============================================================================
// 7. friend 友元
// ============================================================================

/*
friend 就是"好朋友"，友元函数/友元类可以访问这个类的private成员。

用途：
- 运算符重载，比如 `ostream& operator<<(ostream& os, const MyClass& obj)` 需要访问私有成员，所以声明为friend
- 测试：测试类可以访问被测类私有成员，方便测试

优缺点：
- 优点：方便，需要访问私有又不想给所有人开放，给特定朋友开放
- 缺点：破坏了封装，friend能访问私有，所以要谨慎用

例子：
*/

namespace friend_example {

class Box {
private:
    int width_;
public:
    Box(int w) : width_(w) {}
    // 声明友元函数，可以访问私有width_
    friend int compare(const Box& a, const Box& b);
};

int compare(const Box& a, const Box& b) {
    return a.width_ - b.width_;  // ✓ 友元可以访问私有成员
}

} // namespace friend_example

// ============================================================================
// 8. nullptr  vs NULL
// ============================================================================

/*
- NULL 就是宏，本质是 `#define NULL ((void*)0)`，是整数0，不是真正的指针
- nullptr 是C++11引入的，类型是 `std::nullptr_t`，真正代表空指针

优点：
- 类型安全：不会和整数混淆，函数重载能正确区分
- 例子：
*/

namespace nullptr_example {

void func(int) {
    // 处理int
}
void func(void*) {
    // 处理指针
}

void example() {
    func(NULL);  // ❌ NULL是0 整数，会调用func(int)，不是你想要的func(void*)
    func(nullptr);  // ✓ nullptr是指针类型，正确调用func(void*)
}

// 结论：C++11以后，请用nullptr代替NULL，类型安全
} // namespace nullptr_example

// ============================================================================
// 9. =default =delete 用法
// ============================================================================

/*
C++11 引入，非常有用：

- `=default`：要求编译器生成默认版本。
  - 你写了自定义构造函数，编译器就不生成默认构造了，如果你还要默认构造，加 `A() = default;`
  - 保持语法干净，不用空实现，编译器生成更优化

- `=delete`：禁止你不想要的函数。
  - 禁止拷贝：`A(const A&) = delete;` 禁止拷贝构造
  - 禁止参数匹配到错误类型：比如禁止double转int，你可以声明 `void func(double) = delete;`

例子：
*/

namespace default_delete_example {

// 单例，禁止拷贝赋值
class Singleton {
public:
    Singleton() = default;  // 默认构造，=default
    Singleton(const Singleton&) = delete;  // 禁止拷贝
    Singleton& operator=(const Singleton&) = delete;  // 禁止赋值
};

} // namespace default_delete_example

// ============================================================================
// 10. override final 关键字
// ============================================================================

/*
- `override`: 标记派生类重写基类的虚函数
  - 好处：编译器检查，如果签名不对（比如参数不对），编译报错，提前发现错误
  - 你写错函数签名，没override编译器不会报错，就是没重写成功，找bug很难
  - C++11以后，重写虚函数都建议加override，好习惯

- `final`:
  - 修饰类：final class A { ... }; → 这个类不能被继承了
  - 修饰虚函数：virtual void func() final; → 派生类不能重写这个函数了

例子：
*/

namespace override_final_example {

class Base {
public:
    virtual void func(int x);
    virtual void func2() final;  // 派生类不能重写func2
};

class Derived final : public Base {  // Derived 不能再被继承
public:
    void func(int x) override {  // ✅ 明确标记重写，编译器检查
        // ...
    }
};

} // namespace override_final_example

// ============================================================================
// 11. constexpr 关键字
// ============================================================================

/*
constexpr 告诉编译器：**这个函数/变量可以在**编译期**算出结果**，所以可以在编译期求值，运行不用算。

用途：
- constexpr 变量 → 编译期常量，比const更强大
- constexpr 函数 → 可以在编译期计算结果，提升运行时性能
- constexpr 构造函数 → 可以在编译期构造对象，constexpr 变量就能用这个类
*/

namespace constexpr_example {

constexpr int factorial(int n) {  // 编译期可以计算阶乘
    return n <= 1 ? 1 : n * factorial(n - 1);
}

// 编译期算出结果，运行直接用
constexpr int fact5 = factorial(5);  // 编译期算出 120，运行直接是常量

// 对比：constexpr 是编译期确定，const 只是说不能改，可以运行期确定
// constexpr 一定是const，const 不一定是constexpr

// C++11以后，能constexpr就constexpr，提升性能
} // namespace constexpr_example

// ============================================================================
// 12. shared_ptr 引用计数线程安全吗？
// ============================================================================

/*
结论：
- ✓ **引用计数的修改是线程安全的**：shared_ptr 引用计数用原子操作增减，所以计数肯定对，不会计数错了导致double free或者漏释放
- ✗ **对象本身的访问不是线程安全**：你改对象里面的数据，还是要自己加锁
- ✗ **拷贝shared_ptr**：拷贝要修改引用计数，这个修改是线程安全的

例子：
- 多个线程同时拷贝/销毁同一个shared_ptr → 计数修改原子，安全
- 多个线程同时改shared_ptr指向的对象 → 对象本身没保护，不安全，要自己加锁

总结：计数安全，对象不安全，记住这个就行。
*/

// ============================================================================
// 13. enable_shared_from_this 是什么？解决什么问题？
// ============================================================================

/*
问题场景：你对象被shared_ptr管理，你现在需要**在成员函数里拿到this的shared_ptr**，怎么办？

你直接 `return shared_ptr<T>(this)` → 会新建一个控制块，引用计数变成两份，最后double free，错了。

解决：让你的类继承 `std::enable_shared_from_this<T>`，然后调用 `shared_from_this()` 方法，它会从现有控制块拿指针，增加计数，不会新建控制块，正确。

例子：
*/

#include <memory>

class Node : public std::enable_shared_from_this<Node> {
public:
    std::shared_ptr<Node> get_shared() {
        return shared_from_this();  // ✓ 正确拿到this的shared_ptr
    }
};

/*
总结：当对象已经被shared_ptr管理，又需要在内部获取this的shared_ptr，就继承enable_shared_from_this，用shared_from_this()。
*/

// ============================================================================
// 14. auto 推导规则
// ============================================================================

/*
auto 自动推导类型，C++11引入，简化代码，避免写很长类型。

基本规则：
- 表达式推表达式类型，`auto x = expr;` x类型就是expr类型
- 如果要推导引用，要写 `auto& ref = expr;`
- 如果要不可变，`const auto x = expr;`
- 万能引用：`template<typename T> void func(T&& t);` → T&& 不是右值引用，是万能引用，能接受左值也能接受右值，推导后会保留值类别，给完美转发用

好处：避免你写错类型，简化代码，lambda必须用auto。
*/

// ============================================================================
// 15. 拷贝交换 idiom (copy-and-swap)
// ============================================================================

/*
怎么实现拷贝赋值运算符，能处理自赋值，异常安全？

最佳实践就是拷贝交换：
1. 按值传参，自动拷贝
2. 交换指针，swap
3. 函数结束，旧对象析构释放

好处：
- 自动处理自赋值（拷贝已经做了，swap交换，没问题）
- 异常安全：如果拷贝抛出异常，*this还没改，回滚容易

代码示例：
*/

namespace copy_swap {

template<typename T>
class MyPtr {
    T* data_;
public:
    MyPtr(T* data) : data_(data) {}
    ~MyPtr() { delete data_; }

    // 拷贝赋值 拷贝交换
    MyPtr& operator=(MyPtr other) {  // 🔥 按值传参，自动拷贝
        swap(data_, other.data_);  // 交换指针
        return *this;
    }  // 函数结束，other析构，释放原来的data_，完美
};

} // namespace copy_swap

// ============================================================================
// 16. namespace 名字空间
// ============================================================================

/*
名字空间用来解决**名字冲突**，不同库的相同名字不会冲突。

- `namespace ns { ... }` 定义名字空间
- `using namespace ns;` 引入整个名字空间到当前作用域
- `using ns::func;` 只引入特定名字
- 匿名命名空间 `namespace { ... }` → 里面的东西只在当前cpp文件可见，相当于static全局变量，不会链接冲突

现在项目一般都放在自己名字空间，避免和其他库冲突。
*/

// ============================================================================
// END
// ============================================================================
