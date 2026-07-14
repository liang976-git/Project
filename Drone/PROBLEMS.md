# 项目问题与难点记录

> 记录开发过程中遇到的问题、解决方案和重要知识点  
> **面试高频** 标记 = 面试常问，需重点掌握

---

## Day 1 — 环境搭建

### 问题 1：QSqlError 报错 "invalid use of incomplete type"
- **原因**：缺少 `#include <QSqlError>` 头文件
- **解决**：添加头文件即可
- **知识点**：Qt 模块化设计，每个子模块需单独包含头文件
- **面试相关**：⚠️ Qt 头文件组织规则

### 问题 2：ZeroMQ recv 在 send 前导致卡死
- **原因**：PUB-SUB 模式中，subscriber 连接需要时间，且 recv 是阻塞调用
- **解决**：先 send 后 recv，中间加 sleep 等待连接建立
- **知识点**：阻塞/非阻塞 IO、PUB-SUB 时序

**⚠️ 面试题：阻塞 IO vs 非阻塞 IO vs IO 多路复用**

| 类型 | 描述 | 特点 |
|------|------|------|
| **阻塞 IO** | read/recv 调用后，线程挂起等待数据就绪 | 简单，但并发时一个连接占一个线程 |
| **非阻塞 IO** | read/recv 立即返回，数据没好就返回错误码(EAGAIN) | 需要轮询，CPU 空转浪费 |
| **IO 多路复用** | select/poll/epoll 同时监听多个 fd，哪个就绪处理哪个 | 高并发核心，一个线程管多个连接 |
| **异步 IO** | 内核完成数据拷贝后通知用户态 | 最高效，但实现复杂 |

**面试追问**：
- **select vs poll vs epoll**？→ select 有 fd 数量限制(1024)且每次拷贝全量 fd_set；poll 无数量限制但仍拷贝；epoll 用红黑树+就绪链表，只返回就绪的 fd，O(1) 效率
- **epoll 的 ET 和 LT 模式**？→ LT(默认) 数据没读完下次还会通知；ET 只通知一次，必须一次读完，性能更好但编程更难
- **ZeroMQ 本质是什么**？→ 用户态的高性能消息队库，封装了 socket + IO 多路复用，提供 PUB-SUB/REQ-REP/PUSH-PULL 等模式

**⚠️ 面试题：发布订阅模式 vs 观察者模式**

| 对比 | 观察者模式 | 发布订阅模式 |
|------|-----------|-------------|
| **耦合度** | 观察者直接引用被观察者（紧耦合） | 通过消息中间件解耦（松耦合） |
| **通信方式** | 直接调用 | 通过 Broker 中转 |
| **典型实现** | Qt 信号槽、C# event | ZeroMQ PUB-SUB、Redis Pub-Sub |
| **适用场景** | 同进程内对象通信 | 跨进程/跨机器分布式通信 |

---

## Day 2 — 核心数据结构与模型层

### 知识点 1：explicit 关键字
- **用法**：`explicit DroneManager(QObject *parent = nullptr);`
- **作用**：禁止编译器使用该构造函数进行隐式类型转换

**⚠️ 面试题：explicit 的作用？什么时候必须加？**

```cpp
class Foo {
public:
    Foo(int x);           // 不加 explicit
    explicit Foo(int x);  // 加 explicit
};

// 不加 explicit 时，以下代码合法（危险！）
Foo f = 42;       // 编译器隐式把 42 转为 Foo(42)
Foo f2 = Foo(42); // 显式构造，合法

// 加了 explicit 后
Foo f = 42;       // 编译错误！禁止隐式转换
Foo f2 = Foo(42); // 合法
Foo f3(42);       // 合法
```

**什么情况必须加**：
1. 单参数构造函数（最容易出意外转换）
2. 有默认参数的构造函数（如 `Foo(int x = 0)` 等效单参数）
3. C++11 的**列表初始化**也受 explicit 控制

**标准库中的例子**：`std::unique_ptr`、`std::optional` 都加了 explicit，防止 `unique_ptr<A> p = new A()` 这种意外。

### 知识点 2：Qt 信号槽机制（面试必问）
- **基本概念**：对象间通信机制，替代回调函数

**⚠️ 面试题：Qt 信号槽的底层原理？效率如何？**

**底层实现**：
1. **MOC（Meta-Object Compiler）**：Qt 的元对象编译器，预处理时扫描 `Q_OBJECT` 宏，生成 `moc_xxx.cpp` 文件
2. **connect 调用时**：将信号和槽的函数指针、线程信息存入 `QMetaObject::Connection` 数据结构
3. **emit 信号时**：调用 `QMetaObject::activate()`，遍历该信号连接的所有槽函数指针并调用

**三种连接方式**：
```
DirectConnection（直连）：
  sender → 直接调用 receiver->slot()
  同线程，同步执行，相当于普通函数调用

QueuedConnection（队列连接）：
  sender → 事件队列 → receiver 所在线程的事件循环取出并调用 slot()
  跨线程，异步执行，通过事件循环线程安全

AutoConnection（自动连接，默认）：
  Qt 在 emit 时检查 sender 和 receiver 是否同线程
  同线程 → DirectConnection
  不同线程 → QueuedConnection
```

**性能**：
- 信号槽比直接函数调用慢约 10 倍（额外开销：查找连接、线程安全检查）
- 但在 UI 交互场景下完全可以忽略
- 高频数据场景（如本项目 100ms 遥测刷新）可以用函数指针直调代替

**面试追问**：
- **信号可以连接信号吗？** → 可以，信号转发
- **信号槽可以断开吗？** → `disconnect()`
- **一个信号可以连接同一个槽多次吗？** → 可以，会调用多次
- **槽函数可以有返回值吗？** → 可以，但多连接时只有最后一个返回值有效

### 知识点 3：QMap 容器
- **底层实现**：红黑树（平衡二叉搜索树）

**⚠️ 面试题：map vs unordered_map vs multimap？红黑树 vs 哈希表？**

| 容器 | 底层 | 查找 | 插入 | 删除 | 有序？ |
|------|------|------|------|------|--------|
| `std::map` / `QMap` | 红黑树 | O(log n) | O(log n) | O(log n) | ✅ 有序 |
| `std::unordered_map` / `QHash` | 哈希表 | O(1) 均摊 | O(1) 均摊 | O(1) 均摊 | ❌ 无序 |
| `std::multimap` | 红黑树 | O(log n) | O(log n) | O(log n) | ✅ 有序，key 可重复 |

**红黑树特点**：
- 自平衡二叉搜索树，最长路径不超过最短路径 2 倍
- 查找/插入/删除都是 O(log n)
- 遍历有序（中序遍历递增）
- 适合：需要有序遍历、范围查询、key 数量不大

**哈希表特点**：
- 平均 O(1) 查找，但最坏 O(n)（哈希冲突）
- 无序，不支持范围查询
- 适合：纯查找、key 数量大、不需要有序

**面试追问**：
- **为什么选 QMap？** → 本项目无人机数量不大（<100），且需要按 ID 快速查找 + 有序遍历，红黑树完全够用
- **什么时候用 QHash？** → 纯查找场景，key 数量大（>1000），不需要顺序
- **C++17 的 `std::map::contains()`** → `if (map.contains(key))` 比 `find() != end()` 更简洁

### 知识点 4：设计模式对比 — 观察者 vs 信号槽 vs 发布订阅

**⚠️ 面试题：观察者模式、发布订阅、信号槽有什么区别？**

**观察者模式（Observer）**：
```
Subject（被观察者）
  ├── ObserverA（直接引用）
  ├── ObserverB（直接引用）
  └── ObserverC（直接引用）

特点：
  - 被观察者持有观察者的引用/指针
  - 通过 attach/detach 管理订阅
  - 观察者直接调用被观察者的方法
  - 耦合度：中（直接引用）
  - 典型实现：C# event、Java Listener、Go channel
```

**发布订阅模式（Pub-Sub）**：
```
Publisher → Message Broker ← Subscriber A
                      ↖ Subscriber B

特点：
  - 发布者和订阅者完全不感知对方
  - 通过消息中间件（Broker）转发
  - 支持消息过滤（topic/selector）
  - 耦合度：最低（完全解耦）
  - 典型实现：ZeroMQ、Redis Pub-Sub、Kafka、RabbitMQ
```

**Qt 信号槽（Signal-Slot）**：
```
connect(sender, &Sender::signal, receiver, &Receiver::slot);

特点：
  - 本质是观察者模式的增强版
  - 不需要手动 attach/detach，connect 自动管理
  - 支持跨线程（QueuedConnection 自动线程切换）
  - 编译期类型检查（新语法 &Sender::signal）
  - 耦合度：低（通过信号名连接，不需要直接引用）
```

**对比总结**：

| 维度 | 观察者模式 | 信号槽 | 发布订阅 |
|------|-----------|--------|---------|
| 耦合度 | 中 | 低 | 最低 |
| 跨线程 | 不支持 | ✅ 自动 | ✅ Broker 负责 |
| 跨进程 | 不支持 | 不支持 | ✅ 支持 |
| 类型安全 | 取决于实现 | ✅ 编译期检查 | ❌ 通常弱类型 |
| 本项目使用 | — | DroneManager 状态通知 |遥测数据分发 |

---

## 待记录

_后续开发中遇到的问题在此追加_

