#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
一个全面的TCP客户端压力测试工具，用于测试Reactor网络库的性能和稳定性。
"""

import socket
import time
import threading
import sys
import random
import statistics
from collections import defaultdict
from datetime import datetime
import os

class TestLogger:
    """测试日志记录器"""
    def __init__(self, log_file="reactor_test_results.log"):
        self.log_file = log_file
        self.lock = threading.Lock()
        
        # 确保日志目录存在
        os.makedirs(os.path.dirname(os.path.abspath(log_file)), exist_ok=True)
        
        # 写入测试开始标记
        self.log(f"\n{'='*60}")
        self.log(f"测试开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        self.log(f"{'='*60}")
    
    def log(self, message):
        """同时输出到控制台和日志文件"""
        print(message)
        with self.lock:
            with open(self.log_file, 'a', encoding='utf-8') as f:
                f.write(message + '\n')
                f.flush()  # 确保立即写入磁盘

# 全局日志器
logger = TestLogger("tests/reactor_test_results.log")

class TestClient:
    """一个封装了TCP连接、发送、接收和关闭操作的客户端类。"""
    def __init__(self, host='127.0.0.1', port=8888):
        self.host = host
        self.port = port
        self.socket = None
        
    def connect(self, timeout=5):
        """连接到服务器"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(timeout)
            self.socket.connect((self.host, self.port))
            return True
        except Exception as e:
            return False
    
    def send(self, message):
        """发送消息到服务器"""
        if self.socket is None:
            return False
        try:
            self.socket.sendall(message.encode('utf-8'))
            return True
        except Exception:
            return False
    
    def receive(self):
        """接收服务器响应"""
        if self.socket is None:
            return None
        try:
            data = self.socket.recv(4096)
            if data:
                return data.decode('utf-8')
            return None
        except Exception:
            return None
    
    def close(self):
        """关闭连接"""
        if self.socket:
            try:
                self.socket.close()
            except:
                pass

# 全局统计
stats = {
    'connections_success': 0,
    'connections_failed': 0,
    'messages_success': 0,
    'messages_failed': 0,
    'response_times': [],
    'lock': threading.Lock()
}

def update_stats(conn_success=0, conn_failed=0, msg_success=0, msg_failed=0, response_time=None):
    with stats['lock']:
        stats['connections_success'] += conn_success
        stats['connections_failed'] += conn_failed
        stats['messages_success'] += msg_success
        stats['messages_failed'] += msg_failed
        if response_time is not None:
            stats['response_times'].append(response_time)

def basic_functionality_test():
    """基础功能测试"""
    logger.log("\n=== [测试 1/4] 基础功能测试 ===")
    client = TestClient()
    
    if not client.connect():
        logger.log("✗ 基础测试失败：无法连接到服务器")
        return False
    
    test_messages = [
        "Hello, Reactor!",
        "你好，世界！",
        "A" * 100,  # 100字节
        "B" * 1000, # 1KB
        "C" * 4000, # 4KB（接近缓冲区大小）
    ]
    
    for i, msg in enumerate(test_messages):
        start_time = time.time()
        if not client.send(msg):
            logger.log(f"✗ 发送消息 {i+1} 失败")
            client.close()
            return False
        
        response = client.receive()
        response_time = time.time() - start_time
        
        if response != msg:
            logger.log(f"✗ 消息 {i+1} 回显不匹配")
            client.close()
            return False
        
        logger.log(f"✓ 消息 {i+1} ({len(msg)} 字节) - {response_time*1000:.2f}ms")
        update_stats(msg_success=1, response_time=response_time)
    
    client.close()
    logger.log("✓ 基础功能测试通过")
    return True

def concurrent_connections_test(num_clients=50):
    """渐进式并发连接测试"""
    logger.log(f"\n=== [测试 2/4] 并发连接测试 ({num_clients} 客户端) ===")
    
    def client_worker(client_id):
        client = TestClient()
        if not client.connect():
            update_stats(conn_failed=1)
            return
        
        update_stats(conn_success=1)
        
        # 发送一条消息
        msg = f"Client-{client_id}-Hello"
        start_time = time.time()
        
        if client.send(msg):
            response = client.receive()
            response_time = time.time() - start_time
            
            if response == msg:
                update_stats(msg_success=1, response_time=response_time)
            else:
                update_stats(msg_failed=1)
        else:
            update_stats(msg_failed=1)
        
        client.close()
    
    threads = []
    start_time = time.time()
    
    # 分批启动连接，避免瞬间压力过大
    batch_size = 10
    for batch_start in range(0, num_clients, batch_size):
        batch_end = min(batch_start + batch_size, num_clients)
        
        # 启动一批线程
        batch_threads = []
        for i in range(batch_start, batch_end):
            thread = threading.Thread(target=client_worker, args=(i + 1,))
            batch_threads.append(thread)
            thread.start()
        
        threads.extend(batch_threads)
        time.sleep(0.1)  # 批次间隔100ms
    
    # 等待所有线程完成
    for thread in threads:
        thread.join()
    
    total_time = time.time() - start_time
    logger.log(f"✓ 并发测试完成 - 耗时 {total_time:.2f}s")
    return True

def sustained_load_test(duration=30, clients_per_second=5):
    """持续负载测试"""
    logger.log(f"\n=== [测试 3/4] 持续负载测试 ({duration}秒, {clients_per_second} 连接/秒) ===")
    
    def client_worker(client_id):
        client = TestClient()
        if not client.connect():
            update_stats(conn_failed=1)
            return
        
        update_stats(conn_success=1)
        
        # 发送多条消息
        for i in range(3):  # 每个客户端发送3条消息
            msg_size = random.choice([100, 500, 1000])  # 随机消息大小
            msg = f"Client-{client_id}-Msg-{i}-" + "X" * (msg_size - 20)
            
            start_time = time.time()
            if client.send(msg):
                response = client.receive()
                response_time = time.time() - start_time
                
                if response == msg:
                    update_stats(msg_success=1, response_time=response_time)
                else:
                    update_stats(msg_failed=1)
            else:
                update_stats(msg_failed=1)
            
            time.sleep(0.1)  # 消息间隔
        
        client.close()
    
    start_time = time.time()
    client_id = 0
    
    while time.time() - start_time < duration:
        # 每秒启动指定数量的客户端
        for _ in range(clients_per_second):
            client_id += 1
            thread = threading.Thread(target=client_worker, args=(client_id,))
            thread.daemon = True  # 设置为守护线程
            thread.start()
        
        time.sleep(1)  # 等待1秒
        
        # 实时显示统计
        with stats['lock']:
            status_msg = (f"运行时间: {time.time() - start_time:.1f}s | "
                         f"连接: {stats['connections_success']}/{stats['connections_success'] + stats['connections_failed']} | "
                         f"消息: {stats['messages_success']}/{stats['messages_success'] + stats['messages_failed']}")
            logger.log(status_msg)
    
    # 等待最后一批完成
    time.sleep(3)
    logger.log("✓ 持续负载测试完成")
    return True

def print_final_statistics():
    """打印最终统计结果"""
    logger.log("\n=== [测试 4/4] 性能统计报告 ===")
    
    with stats['lock']:
        total_connections = stats['connections_success'] + stats['connections_failed']
        total_messages = stats['messages_success'] + stats['messages_failed']
        
        logger.log(f"连接统计:")
        logger.log(f"  成功: {stats['connections_success']}")
        logger.log(f"  失败: {stats['connections_failed']}")
        logger.log(f"  成功率: {stats['connections_success']/total_connections*100:.1f}%" if total_connections > 0 else "  成功率: 0%")
        
        logger.log(f"\n消息统计:")
        logger.log(f"  成功: {stats['messages_success']}")
        logger.log(f"  失败: {stats['messages_failed']}")
        logger.log(f"  成功率: {stats['messages_success']/total_messages*100:.1f}%" if total_messages > 0 else "  成功率: 0%")
        
        if stats['response_times']:
            response_times = stats['response_times']
            logger.log(f"\n响应时间统计:")
            logger.log(f"  平均: {statistics.mean(response_times)*1000:.2f}ms")
            logger.log(f"  中位数: {statistics.median(response_times)*1000:.2f}ms")
            logger.log(f"  最小: {min(response_times)*1000:.2f}ms")
            logger.log(f"  最大: {max(response_times)*1000:.2f}ms")
            if len(response_times) > 1:
                logger.log(f"  标准差: {statistics.stdev(response_times)*1000:.2f}ms")
    
    # 写入测试结束标记
    logger.log(f"\n测试结束时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    logger.log(f"{'='*60}\n")

def main():
    logger.log("Reactor网络库压力测试工具")
    logger.log("请确保服务器已在127.0.0.1:8888上运行")
    logger.log("=" * 50)

    try:
        # 基础功能测试
        if not basic_functionality_test():
            logger.log("✗ 基础功能测试失败，终止测试")
            sys.exit(1)
        
        # 并发连接测试（适中的数量）
        concurrent_connections_test(num_clients=50)
        
        # 持续负载测试
        sustained_load_test(duration=30, clients_per_second=3)
        
        # 打印统计
        print_final_statistics()
        
        logger.log("\n" + "=" * 50)
        logger.log("✓✓✓ 所有压力测试完成！")
        
    except KeyboardInterrupt:
        logger.log("\n用户中断测试")
        print_final_statistics()
    except Exception as e:
        logger.log(f"\n测试过程中出现错误: {e}")
        print_final_statistics()

if __name__ == "__main__":
    main() 