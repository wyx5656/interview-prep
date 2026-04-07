#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
并发压力测试
测试服务器在高并发下是否稳定
"""

import asyncio
import aiohttp
import json
import argparse
import time
from collections import Counter

async def fetch(session, url, query):
    """异步发送请求"""
    data = {"data": query}
    try:
        async with session.post(url, json=data) as response:
            result = await response.json()
            return (response.status, result)
    except Exception as e:
        return (None, str(e))

async def run_concurrent_test(url, queries, concurrency, total_requests):
    """运行并发测试"""
    connector = aiohttp.TCPConnector(limit=concurrency)
    async with aiohttp.ClientSession(connector=connector) as session:
        tasks = []
        for i in range(total_requests):
            query = queries[i % len(queries)]
            task = asyncio.ensure_future(fetch(session, url, query))
            tasks.append(task)

        print(f"Starting {total_requests} requests with {concurrency} concurrent connections...")
        start_time = time.time()
        results = await asyncio.gather(*tasks)
        end_time = time.time()

        return results, end_time - start_time

def analyze_results(results, elapsed):
    """分析测试结果"""
    status_counter = Counter()
    errors = []
    success = 0

    for status, result in results:
        if status is None:
            errors.append(result)
            continue
        status_counter[status] += 1
        if status == 200:
            success += 1

    total = len(results)
    error_rate = len(errors) / total * 100
    qps = success / elapsed

    print("\n" + "=" * 60)
    print("并发测试结果")
    print("=" * 60)
    print(f"  总请求数.......: {total}")
    print(f"  成功请求.......: {success}")
    print(f"  失败请求.......: {len(errors)}")
    print(f"  错误率.........: {error_rate:.2f}%")
    print(f"  总耗时.........: {elapsed:.2f} s")
    print(f"  QPS............: {qps:.2f} req/s")
    print()

    if status_counter:
        print("  状态码分布:")
        for status, count in sorted(status_counter.items()):
            print(f"    {status}: {count}")

    print("=" * 60)

    return {
        'total': total,
        'success': success,
        'errors': len(errors),
        'error_rate': error_rate,
        'qps': qps,
        'elapsed': elapsed
    }

def main():
    parser = argparse.ArgumentParser(description='Concurrent stress test for search engine')
    parser.add_argument('--host', default='http://localhost', help='Server host')
    parser.add_argument('--port', type=int, default=8888, help='Server port')
    parser.add_argument('--endpoint', default='/search', help='Endpoint to test')
    parser.add_argument('--concurrency', type=int, default=10, help='Number of concurrent connections')
    parser.add_argument('--requests', type=int, default=1000, help='Total number of requests')
    args = parser.parse_args()

    url = f"{args.host}:{args.port}{args.endpoint}"

    # 测试查询
    test_queries = [
        "search", "engine", "google", "china", "world", "news",
        "中国", "经济", "文化", "科技", "互联网", "社会", "政治",
        "company", "business", "market", "data", "science"
    ]

    print(f"Starting concurrent test on {url}")
    print(f"Concurrency: {args.concurrency}, Total requests: {args.requests}")
    print()

    loop = asyncio.get_event_loop()
    results, elapsed = loop.run_until_complete(
        run_concurrent_test(url, test_queries, args.concurrency, args.requests)
    )

    analyze_results(results, elapsed)

if __name__ == '__main__':
    main()
