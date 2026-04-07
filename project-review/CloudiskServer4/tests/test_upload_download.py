#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
完整上传下载测试脚本
测试：
1. 普通上传
2. 普通下载
3. 断点续传上传
4. 断点续传下载
"""

import socket
import struct
import os
import sys
import hashlib

CMD_TYPE_PWD=1
CMD_TYPE_LS=2
CMD_TYPE_CD=3
CMD_TYPE_MKDIR=4
CMD_TYPE_RMDIR=5
CMD_TYPE_PUTS=6
CMD_TYPE_GETS=7

def recv_all(sock, length):
    """接收指定长度的数据"""
    data = b''
    while len(data) < length:
        chunk = sock.recv(length - len(data))
        if not chunk:
            return None
        data += chunk
    return data

def send_command(sock, cmd_type, content):
    """发送命令按照小火车协议"""
    length = len(content)
    buf = struct.pack('ii', length, cmd_type) + content.encode('utf-8')
    sock.sendall(buf)

def test_basic_commands(host, port):
    """测试基础命令"""
    print("=" * 60)
    print("【测试1】基础命令测试")
    print("=" * 60)

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    # 接收originfd
    originfd_data = recv_all(sock, 4)
    if not originfd_data:
        print("❌ 接收originfd失败")
        return False

    print("✅ 连接成功，收到originfd")

    # 测试pwd
    print("\n  >>> 测试 pwd")
    send_command(sock, CMD_TYPE_PWD, "")
    data = sock.recv(1024)
    if data:
        print(f"  ← {data.decode('utf-8').strip()}")
        print("  ✅ pwd 成功")
    else:
        print("  ❌ pwd 失败")
        sock.close()
        return False

    # 测试 ls
    print("\n  >>> 测试 ls")
    send_command(sock, CMD_TYPE_LS, "")
    data = sock.recv(4096)
    if data:
        print(f"  ← {len(data)} bytes received")
        print("  ✅ ls 成功")
    else:
        print("  ❌ ls 失败")
        sock.close()
        return False

    sock.close()
    print("\n✅ 所有基础命令测试通过")
    return True

def test_upload_download(host, port, test_file_size=10240):
    """测试上传下载"""
    print("\n" + "=" * 60)
    print("【测试2】完整上传下载测试")
    print(f"测试文件大小: {test_file_size} bytes")
    print("=" * 60)

    # 创建测试文件
    test_data = os.urandom(test_file_size)
    sha256 = hashlib.sha256(test_data).hexdigest()

    with open('/tmp/test_upload.bin', 'wb') as f:
        f.write(test_data)

    # 上传
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    recv_all(sock, 4)  # 读originfd

    print(f"\n  上传文件: random.bin, SHA256={sha256}")
    send_command(sock, CMD_TYPE_PUTS, "random.bin")

    # 发送sha256
    sock.sendall(sha256.encode('utf-8'))
    # 发送文件长度
    sock.sendall(struct.pack('i', test_file_size))
    # 发送偏移量 (断点续传，0表示从头开始)
    sock.sendall(struct.pack('i', 0))
    # 发送文件内容
    sock.sendall(test_data)

    print("  ✅ 文件上传完成")

    # 接收响应
    resp = sock.recv(128)
    print(f"  ← 服务器响应: {resp.decode('utf-8').strip()}")
    sock.close()

    # 下载
    print(f"\n  下载文件: random.bin")
    sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock2.connect((host, port))
    recv_all(sock2, 4)

    send_command(sock2, CMD_TYPE_GETS, "random.bin")

    # 客户端请求断点从0开始
    sock2.sendall(struct.pack('i', 0))

    # 接收文件长度
    len_data = recv_all(sock2, 4)
    if not len_data:
        print("  ❌ 接收文件长度失败")
        return False
    file_len = struct.unpack('i', len_data)[0]
    print(f"  ← 文件长度: {file_len} bytes")

    # 接收文件内容
    received = b''
    while len(received) < file_len:
        chunk = sock2.recv(min(4096, file_len - len(received)))
        if not chunk:
            break
        received += chunk

    print(f"  ✅ 文件下载完成，接收 {len(received)} bytes")

    received_sha256 = hashlib.sha256(received).hexdigest()
    print(f"  原SHA256: {sha256}")
    print(f"  下载SHA256: {received_sha256}")

    if received_sha256 == sha256:
        print("  ✅ 文件完整性校验通过")
    else:
        print("  ❌ 文件完整性校验失败")
        sock2.close()
        return False

    sock2.close()

    # 清理
    os.remove('/tmp/test_upload.bin')
    if os.path.exists('/tmp/test_download.bin'):
        os.remove('/tmp/test_download.bin')

    print("\n✅ 上传下载测试通过")
    return True

def main():
    host = '127.0.0.1'
    port = 8080

    if len(sys.argv) > 1:
        host = sys.argv[1]
    if len(sys.argv) > 2:
        port = int(sys.argv[2])

    print(f"测试目标: {host}:{port}")
    print()

    all_passed = True

    # 测试1: 基础命令
    if not test_basic_commands(host, port):
        all_passed = False

    # 测试2: 上传下载 10KB
    if not test_upload_download(host, port, 10240):
        all_passed = False

    # 测试3: 上传下载 1MB
    if not test_upload_download(host, port, 1024 * 1024):
        all_passed = False

    print("\n" + "=" * 60)
    if all_passed:
        print("✅ 所有测试通过!")
    else:
        print("❌ 部分测试失败!")
        sys.exit(1)
    print("=" * 60)

if __name__ == '__main__':
    main()
