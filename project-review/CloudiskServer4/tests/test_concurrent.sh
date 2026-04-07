#!/bin/bash
# 并发连接性能测试
# 测试 epoll 边缘触发在高并发下的表现

echo "===== CloudiskServer 并发连接测试 ====="

SERVER_HOST="127.0.0.1"
SERVER_PORT="8080"
NUM_CLIENTS=$1

if [ -z "$NUM_CLIENTS" ]; then
    NUM_CLIENTS=50
fi

echo "并发连接数: $NUM_CLIENTS"
echo

# 检查服务器是否运行
if ! pgrep -x "CloudiskServer" > /dev/null; then
    echo "❌ 服务器未运行"
    exit 1
fi

echo "✅ 服务器运行中，开始并发连接测试..."
echo

START_TIME=$(date +%s%N)

# 创建多个并发连接
failed=0
for i in $(seq 1 $NUM_CLIENTS); do
    (echo -n "" | nc $SERVER_HOST $SERVER_PORT > /dev/null 2>&1) &
done

# 等待所有连接完成
wait

END_TIME=$(date +%s%N)
ELAPSED_MS=$(( ($END_TIME - $START_TIME) / 1000000 ))

echo "完成 $NUM_CLIENTS 个并发连接，总耗时: ${ELAPSED_MS}ms"
echo "平均每个连接: $(( $ELAPSED_MS / $NUM_CLIENTS ))ms"
echo

if [ $failed -eq 0 ]; then
    echo "✅ 所有并发连接测试通过"
else
    echo "⚠️  $failed 个连接失败"
fi

echo
echo "===== 并发连接测试完成 ====="
exit $failed
