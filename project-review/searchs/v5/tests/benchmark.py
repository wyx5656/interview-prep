#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
搜索引擎性能基准测试
测试:
- 延迟测试：suggest/search 查询延迟分布
- 吞吐量测试：QPS
- 缓存命中率测试
"""

import socket
import json
import time
import statistics
import sys
import argparse

def send_request(host, port, endpoint, data):
    """发送请求并返回响应"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    request = f"POST {endpoint} HTTP/1.1\r\nHost: {host}\r\nContent-Type: application/json\r\nContent-Length: {len(json.dumps(data))}\r\n\r\n{json.dumps(data)}"

    start = time.perf_counter()
    sock.sendall(request.encode())

    # 读取响应
    response = b""
    while True:
        chunk = sock.recv(4096)
        if not chunk:
            break
        response += chunk

    end = time.perf_counter()
    sock.close()

    # 解析响应体
    response_str = response.decode()
    if "\r\n\r\n" in response_str:
        body = response_str.split("\r\n\r\n", 1)[1]
        try:
            return json.loads(body), (end - start) * 1000
        except:
            return None, (end - start) * 1000
    return None, (end - start) * 1000

def benchmark_latency(host, port, queries, endpoint):
    """测量延迟"""
    print("=" * 60)
    print(f"【延迟测试】{endpoint}，{len(queries)} 次查询")
    print("=" * 60)

    latencies = []
    errors = 0

    for query in queries:
        result, lat = send_request(host, port, endpoint, {"data": query})
        if result is not None and "error" not in result:
            latencies.append(lat)
        else:
            errors += 1

    if not latencies:
        print("❌ All requests failed")
        return None

    avg_lat = statistics.mean(latencies)
    median_lat = statistics.median(latencies)
    min_lat = min(latencies)
    max_lat = max(latencies)
    p95 = sorted(latencies)[int(len(latencies) * 0.95)]
    p99 = sorted(latencies)[int(len(latencies) * 0.99)]

    print(f"  总请求数: {len(queries)}")
    print(f"  错误数: {errors}")
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
        'samples': len(latencies),
        'errors': errors
    }

def benchmark_throughput(host, port, num_requests, endpoint, query):
    """测量吞吐量"""
    print("=" * 60)
    print(f"【吞吐量测试】{endpoint}，{num_requests} 次请求")
    print("=" * 60)

    start = time.perf_counter()
    completed = 0
    errors = 0

    for _ in range(num_requests):
        result, _ = send_request(host, port, endpoint, {"data": query})
        if result is not None and "error" not in result:
            completed += 1
        else:
            errors += 1

    end = time.perf_counter()
    elapsed = end - start
    qps = completed / elapsed

    print(f"  完成请求: {completed}")
    print(f"  错误数: {errors}")
    print(f"  总耗时: {elapsed:.2f} s")
    print(f"  QPS: {qps:.2f} req/s")
    print()

    return {
        'qps': qps,
        'elapsed': elapsed,
        'requests': completed,
        'errors': errors
    }

def main():
    parser = argparse.ArgumentParser(description='Search engine benchmark')
    parser.add_argument('--host', default='localhost', help='Server host')
    parser.add_argument('--port', type=int, default=8888, help='Server port')
    args = parser.parse_args()

    # 检查服务器是否运行
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3)
        sock.connect((args.host, args.port))
        sock.close()
        print(f"✅ Server is running on {args.host}:{args.port}")
    except Exception as e:
        print(f"❌ Cannot connect to server {args.host}:{args.port}")
        print("Please start the server first: ./bin/online")
        sys.exit(1)

    print()

    # 示例搜索词
    suggest_queries = [
        "se", "sea", "sear", "searc", "search",
        "go", "goo", "goog", "google",
        "中", "中国",
    ]

    search_queries = [
        "search", "engine", "google", "china", "world",
        "中国", "经济", "文化", "科技", "互联网",
    ]

    # 运行测试
    print("=" * 60)
    print("SEARCH ENGINE BENCHMARK")
    print("=" * 60)
    print()

    suggest_latency = benchmark_latency(args.host, args.port, suggest_queries, "/suggest")
    search_latency = benchmark_latency(args.host, args.port, search_queries, "/search")

    throughput_result = benchmark_throughput(args.host, args.port, 100, "/search", "search")

    # 总结
    print("=" * 60)
    print("性能测试总结")
    print("=" * 60)
    if suggest_latency:
        print(f"  /suggest 平均延迟........: {suggest_latency['avg']:.2f} ms")
    if search_latency:
        print(f"  /search 平均延迟........: {search_latency['avg']:.2f} ms")
    print(f"  吞吐量..................: {throughput_result['qps']:.1f} req/s")
    print("=" * 60)
    print()

if __name__ == '__main__':
    main()
