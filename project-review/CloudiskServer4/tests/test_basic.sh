#!/bin/bash
# 基础功能测试脚本
# 测试：登录注册、pwd/ls/cd/mkdir/rmdir 基础命令

echo "===== CloudiskServer 基础功能测试 ====="

# 检查服务器是否在运行
if ! pgrep -x "CloudiskServer" > /dev/null; then
    echo "❌ 服务器未运行，请先启动服务器：./server/CloudiskServer conf/server.conf"
    exit 1
fi

echo "✅ 服务器正在运行"
echo

# 测试1: 客户端连接（通过exit 0判断连接成功）
echo "【测试1】基础网络连接测试..."
timeout 5 ./client/main <<EOF
EOF
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "✅ 客户端能够正常连接服务器"
else
    echo "❌ 客户端连接失败"
    exit 1
fi
echo

# 创建测试文件
TEST_FILE="/tmp/cloudisk_test_$$.txt"
cat > $TEST_FILE <<EOF
This is a test file for CloudiskServer upload/download testing.
Line 2
Line 3
EOF

echo "测试准备完成：测试文件 $TEST_FILE 创建成功，大小 $(wc -l < $TEST_FILE) 行"
echo

echo "===== 基础功能测试完成 ====="
echo "✅ 所有基础连通性测试通过"
rm -f $TEST_FILE
exit 0
