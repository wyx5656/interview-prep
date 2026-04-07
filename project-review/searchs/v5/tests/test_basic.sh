#!/bin/bash
# 基础功能测试脚本
# 测试服务器启动、API响应基本功能

echo "===== 搜索引擎 v5 基础功能测试 ====="
echo

# 检查编译产物
if [ ! -f "./bin/online" ]; then
    echo "❌ 可执行文件 ./bin/online 不存在，请先编译"
    exit 1
fi

echo "✅ 编译产物检查通过"
echo

# 检查数据文件
DATA_FILES=(
    "./data/Chinese_fre.txt"
    "./data/ChineseIndex.txt"
    "./data/English_fre.txt"
    "./data/EnglishIndex.txt"
    "./data/index_webpages.dat"
    "./data/new_offset.dat"
    "./data/new_webpages.dat"
)

for f in "${DATA_FILES[@]}"; do
    if [ ! -f "$f" ]; then
        echo "❌ 数据文件缺失: $f"
        exit 1
    fi
done

echo "✅ 所有数据文件检查通过"
echo "   $(wc -l ./data/index_webpages.dat | awk '{print $1}') terms in inverted index"
echo "   $(wc -l ./data/new_offset.dat | awk '{print $1}') documents total"
echo

# 检查Redis是否运行
if ! redis-cli ping > /dev/null 2>&1; then
    echo "⚠️  Redis 未运行，需要启动Redis才能测试完整功能"
    echo "   启动Redis: redis-server"
    exit 1
fi

echo "✅ Redis 运行正常"
echo

echo "===== 基础检查完成 ====="
echo "现在可以启动服务器: ./bin/online"
echo "然后运行性能测试: python3 tests/benchmark.py"
echo

exit 0
