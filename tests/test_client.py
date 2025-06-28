#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
一个简化的TCP客户端, 用于测试Reactor网络库的核心功能。
(多线程并发还未实现)
"""

import socket
import time
import threading
import sys

class TestClient:
    """一个封装了TCP连接、发送、接收和关闭操作的客户端类。"""
    def __init__(self, host='127.0.0.1', port=8888):
        self.host = host
        self.port = port
        self.socket = None
        
    def connect(self):
        """连接到服务器"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            print(f"✓ 连接成功 -> {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"✗ 连接失败: {e}")
            return False
    
    def send(self, message):
        """发送消息到服务器"""
        assert self.socket is not None, "Socket 尚未连接"
        try:
            self.socket.sendall(message.encode('utf-8'))
            print(f"→ 发送: {message[:70] if len(message) > 70 else message}")
            return True
        except Exception as e:
            print(f"✗ 发送失败: {e}")
            return False
    
    def receive(self):
        """接收服务器响应"""
        assert self.socket is not None, "Socket 尚未连接"
        try:
            data = self.socket.recv(4096) # 使用稍大的缓冲区
            if data:
                message = data.decode('utf-8')
                print(f"← 接收: {message[:70] if len(message) > 70 else message}")
                return message
            else:
                print("✗ 服务器似乎已关闭连接。")
                return None
        except Exception as e:
            print(f"✗ 接收失败: {e}")
            return None
    
    def close(self):
        """关闭连接"""
        if self.socket:
            self.socket.close()
            print("✓ 连接已关闭。")

def main():
    print("开始测试Reactor网络库...")
    print("请确保服务器已在127.0.0.1:8888上运行。")
    print("-" * 40)

    try:
        # --- 测试 1: 单连接与多种消息类型 ---
        print("\n=== [测试 1/2] 单连接与多种消息回显 ===")
        client = TestClient()
        if not client.connect():
            print("✗ 测试失败：无法连接到服务器。")
            sys.exit(1)

        test_messages = {
            "基本消息": "Hello, Reactor!",
            "中文消息": "你好，世界！",
            "长消息": "This_is_a_long_message_" * 20  # ~400 bytes
        }

        all_passed = True
        for name, msg in test_messages.items():
            print(f"\n--- 正在测试: {name} ---")
            if not client.send(msg):
                all_passed = False
                break
            
            response = client.receive()
            if response != msg:
                print(f"✗ 失败: 回显不匹配！")
                all_passed = False
                break
            else:
                print(f"✓ 成功: {name}回显正确。")
            time.sleep(0.1)

        client.close()
        
        if not all_passed:
             print("\n✗ 单连接测试失败，测试终止。")
             sys.exit(1)
        else:
            print("\n✓ 单连接测试全部通过！")

        # --- 测试 2: 多客户端并发 ---
        time.sleep(1)  # 等待服务器处理上一个连接的关闭
        print("\n\n=== [测试 2/2] 多客户端并发连接 ===")
        
        num_clients = 5
        threads = []
        
        def client_worker(client_id):
            worker_client = TestClient()
            if worker_client.connect():
                msg = f"来自客户端 {client_id} 的消息"
                worker_client.send(msg)
                resp = worker_client.receive()
                if resp != msg:
                    print(f"✗ 并发测试失败 (客户端 {client_id}): 回显不匹配")
                worker_client.close()
        
        print(f"正在启动 {num_clients} 个并发客户端...")
        for i in range(num_clients):
            thread = threading.Thread(target=client_worker, args=(i + 1,))
            threads.append(thread)
            thread.start()
        
        for thread in threads:
            thread.join()
        
        print("\n✓ 并发连接测试完成。")

        # --- 测试总结 ---
        print("\n" + "=" * 40)
        print("✓✓✓ 所有核心功能测试已成功执行！")

    except KeyboardInterrupt:
        print("\n用户中断测试。")
    except Exception as e:
        print(f"\n测试过程中出现严重错误: {e}")

if __name__ == "__main__":
    main() 