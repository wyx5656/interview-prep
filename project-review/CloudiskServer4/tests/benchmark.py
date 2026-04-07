#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
性能基准测试
测试:
- 短命令响应延迟 (pwd/ls/cd)
- 高并发连接处理能力
"""

import socket
import struct
import time
import statistics
import sys

CMD_TYPE_PWD = 1
CMD_TYPE_LS = 2

def send_ping(sock):
    """发送pwd命令，测量延迟"""
    start = time.perf_counter()
    length = 0
    buf = struct.pack('ii', length, CMD_TYPE_PWD)
    sock.sendall(buf)
    data = sock.recv(1024)
    end = time.perf_counter()
    return (end - start) * 1000  # ms

def benchmark_latency(host, port, num_pings=100):
    """测量短命令延迟"""
    print("=" * 60)
    print(f"【延迟测试】短命令 (pwd) 延迟，{num_pings} 次请求")
    print("=" * 60)

    latencies = []
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    # 读originfd
    sock.recv(4)

    # 热身
    for _ in range(10):
        send_ping(sock)

    # 正式测试
    for _ in range(num_pings):
        lat = send_ping(sock)
        latencies.append(lat)

    sock.close()

    avg_lat = statistics.mean(latencies)
    median_lat = statistics.median(latencies)
    min_lat = min(latencies)
    max_lat = max(latencies)
    p95 = sorted(latencies)[int(len(latencies) * 0.95)]
    p99 = sorted(latencies)[int(len(latencies) * 0.99)]

    print(f"  样本数量: {len(latencies)}")
    print(f"  平均延迟: {avg_lat:.2f} ms")
    print(f"  中位数延迟: {median_lat:.2f} ms")
    print(f"  最小延迟: {min_lat:.2f} ms")
    print(f"  最大延迟: {max_lat:.2f} ms")
    print(f"  P95 延迟: {p95:.2f} ms")
    print(f"  P99 延迟: {p99:.2f} ms")
    print()

    return {
        'avg': avg_lat,
        'median': median_lat,
        'min': min_lat,
        'max': max_lat,
        'p95': p95,
        'p99': p99,
        'samples': len(latencies)
    }

def benchmark_throughput(host, port, num_requests=1000):
    """测量吞吐量"""
    print("=" * 60)
    print(f"【吞吐量测试】短命令 (pwd) 吞吐量，{num_requests} 次请求")
    print("=" * 60)

    start = time.perf_counter()

    completed = 0
    for _ in range(num_requests):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        sock.recv(4)
        length = 0
        buf = struct.pack('ii', length, CMD_TYPE_PWD)
        sock.sendall(buf)
        sock.recv(1024)
        sock.close()
        completed += 1

    end = time.perf_counter()
    elapsed = end - start
    qps = completed / elapsed

    print(f"  完成请求: {completed}")
    print(f"  总耗时: {elapsed:.2f} s")
    print(f"  QPS: {qps:.2f} req/s")
    print()

    return {
        'qps': qps,
        'elapsed': elapsed,
        'requests': completed
    }

def main():
    host = '127.0.0.1'
    port = 8080

    if len(sys.argv) > 1:
        host = sys.argv[1]
    if len(sys.argv) > 2:
        port = int(sys.argv[2])

    print(f"性能基准测试 -> {host}:{port}")
    print()

    # 延迟测试
    latency_result = benchmark_latency(host, port)

    # 吞吐量测试
    throughput_result = benchmark_throughput(host, port)

    print("\n" + "=" * 60)
    print("性能测试总结")
    print("=" * 60)
    print(f"  平均延迟........: {latency_result['avg']:.2f} ms")
    print(f"  P95 延迟........: {latency_result['p95']:.2f} ms")
    print(f"  吞吐量..........: {throughput_result['qps']:.1f} req/s")
    print("=" * 60)
    print()

if __name__ == '__main__':
    main()
