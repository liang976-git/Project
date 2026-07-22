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

## Day 3 — 模拟数据层 + MAVLink协议

### 问题 1：MavlinkParser.h 头文件保护写错
- **原因**：第一行写成了 `#include MAVLINKPARSER_H`，应为 `#ifndef MAVLINKPARSER_H`
- **解决**：改为正确的头文件保护语法
- **知识点**：C++ 头文件保护的两种写法（`#ifndef` vs `#pragma once`）
- **面试相关**：⚠️ `#ifndef` 和 `#pragma once` 的区别

**⚠️ 面试题：`#ifndef` vs `#pragma once`？**

| 对比 | `#ifndef` | `#pragma once` |
|------|-----------|----------------|
| 标准 | C/C++ 标准 | 编译器扩展（非标准） |
| 可移植性 | ✅ 所有编译器支持 | 大部分支持，极少数不支持 |
| 写法 | 需定义宏名，较冗长 | 一行搞定，简洁 |
| 精确度 | 按文件名匹配 | 按物理文件路径匹配 |
| 实际使用 | 老项目、跨平台项目 | 现代C++项目主流 |

### 问题 2：SimulatedLink.h 方法名拼写错误
- **原因**：头文件声明 `stopSimulaton`（少了一个 `i`），但 .cpp 实现是 `stopSimulation`
- **解决**：统一为 `stopSimulation`
- **知识点**：声明和定义的函数签名必须完全一致，包括拼写
- **面试相关**：⚠️ C++ 链接（Linking）— 编译器如何匹配声明和定义

### 问题 3：reinterpret_cast 语法错误
- **原因**：写成 `reinterpret_cast<const char*(&yaw)`，`*` 和 `>` 位置错误
- **正确写法**：`reinterpret_cast<const char*>(&yaw)`
- **知识点**：`reinterpret_cast` 用于指针/引用的底层二进制类型转换
- **面试相关**：⚠️ C++ 四种 cast 的区别

**⚠️ 面试题：C++ 四种类型转换？**

| 类型 | 用途 | 安全性 |
|------|------|--------|
| `static_cast` | 编译期类型转换（int↔float、继承关系） | ✅ 安全，编译器检查 |
| `dynamic_cast` | 运行时多态类型转换（基类↔派生类） | ✅ 安全，失败返回nullptr |
| `const_cast` | 去除/添加 const 修饰 | ⚠️ 危险，改了就改了 |
| `reinterpret_cast` | 底层二进制重新解释（指针↔整数、指针↔指针） | ❌ 最危险，依赖平台 |

**本项目为什么用 `reinterpret_cast`？**
→ 需要把 `int32_t`/`uint16_t` 等数值的二进制直接写入 `QByteArray`，不做任何数值转换，这是 `reinterpret_cast` 的典型用途。

---

### 知识点 1：QTimer 工作原理（面试常问）

**⚠️ 面试题：QTimer 是怎么实现定时的？底层用的什么？**

**原理**：
```
QTimer::start(100)
    ↓
Qt 事件循环注册一个定时器（底层用 epoll/kqueue/timerfd）
    ↓
每隔 100ms，内核向事件队列插入一条 QTimerEvent
    ↓
QCoreApplication::notify() 分发给对应的 QTimer 对象
    ↓
触发 timeout() 信号
```

**底层实现（Linux）**：
- Qt 5 用 `timerfd_create()` 创建文件描述符
- 用 `epoll` 监听该 fd
- 定时到期时 epoll 返回，Qt 事件循环处理

**面试追问**：
- **QTimer 精确吗？** → 不精确。受事件循环阻塞影响，如果有耗时操作卡住事件循环，定时器会延迟
- **QTimer 可以跨线程吗？** → 可以，但必须在目标线程的事件循环中创建，或用 `moveToThread()`
- **QTimer::singleShot 用过吗？** → 一次性定时器，延迟执行某个槽函数，等效于 `std::this_thread::sleep_for` 但不阻塞线程

---

### 知识点 2：二进制协议 vs JSON 序列化（面试必问）

**⚠️ 面试题：项目中为什么用二进制序列化而不是 JSON？**

**本项目的两种序列化方式**：

| 方式 | 用途 | 格式 |
|------|------|------|
| MavlinkParser | 编解码 MAVLink 消息 | 二进制字节流 |
| DataDispatcher | 发布遥测数据 | 二进制字节流 |

**二进制协议 vs JSON 对比**：

| 维度 | 二进制协议 | JSON |
|------|-----------|------|
| 体积 | 小（一个int占4字节） | 大（一个int占~10字节） |
| 速度 | 快（直接内存拷贝） | 慢（需要解析字符串） |
| 可读性 | ❌ 不可读 | ✅ 人类可读 |
| 跨语言 | 需约定字节序/对齐 | ✅ 所有语言都有JSON库 |
| 调试 | 困难，需要抓包工具 | 容易，直接看文本 |
| 适用场景 | 高频实时数据（遥测、游戏） | 配置文件、Web API |

**本项目为什么选二进制？**
→ 遥测数据每 100ms 发一次，5 架无人机 = 每秒 50 次。JSON 解析开销大，二进制直接 `memcpy` 最快。

**面试追问**：
- **字节序问题怎么处理？** → MAVLink 标准用小端序（Little-Endian），x86/ARM 都是小端，本项目直接用 `reinterpret_cast` 写入，不需要转换
- **怎么设计一个二进制协议？** → 固定帧头(0xFD) + 长度 + 序列号 + 消息体 + 校验和，类似本项目的 `buildFrame()`

---

### 知识点 3：ZeroMQ PUB-SUB 深入（面试高频）

**⚠️ 面试题：ZeroMQ 的 PUB-SUB 模式有什么特点？有什么缺点？**

**工作原理**：
```
Publisher                    Subscriber
    │                            │
    ├── bind("tcp://*:5555")     ├── connect("tcp://ip:5555")
    │                            │
    ├── zmq_send(data) ──────────┤── poll/recv 接收
    │                            │
    └── 一对多：多个sub可连接 ────┘
```

**特点**：
- **松耦合**：发布者不知道订阅者是谁
- **一对多**：一个 PUB 可以被多个 SUB 订阅
- **消息过滤**：SUB 可以通过 topic 过滤消息
- **非阻塞**：PUB 发送不等待 SUB 接收

**缺点（重要！）**：
- **消息丢失**：如果 SUB 断开，PUB 发送的消息直接丢弃（无持久化）
- **慢订阅者问题**：如果 SUB 处理太慢，消息会在 ZMQ 内部队列堆积，最终丢弃
- **无确认机制**：PUB 不知道 SUB 是否收到

**面试追问**：
- **怎么解决消息丢失？** → 用 REQ-REP 模式加确认，或用 ZeroMQ 的 `ZMQ_ROUTER`/`ZMQ_DEALER`
- **PUB-SUB 和消息队列（Kafka/RabbitMQ）的区别？** → ZeroMQ 是库（进程内），Kafka 是服务（独立部署），ZeroMQ 无持久化，Kafka 有

---

### 知识点 4：zmq_poll 非阻塞轮询

**⚠️ 面试题：如何在不阻塞 UI 的情况下接收 ZeroMQ 消息？**

**方案对比**：

| 方案 | 做法 | 问题 |
|------|------|------|
| 直接 zmq_recv | 阻塞等待 | ❌ 卡死 UI 线程 |
| 新建线程 + zmq_recv | 子线程阻塞接收 | ⚠️ 需要线程同步 |
| QTimer + zmq_poll | 定时轮询 | ✅ 本项目采用 |

**本项目代码**：
```cpp
zmq_pollitem_t items[] = { { m_subscriber, 0, ZMQ_POLLIN, 0 } };
int rc = zmq_poll(items, 1, 0);  // timeout=0 即非阻塞
if (rc > 0 && (items[0].revents & ZMQ_POLLIN)) {
    zmq_recv(m_subscriber, buffer, sizeof(buffer)-1, ZMQ_DONTWAIT);
}
```

**`zmq_poll` 参数说明**：
- `items[]`：要监听的 socket 数组
- `1`：监听数量
- `0`：超时时间（毫秒），0 = 不等待
- `ZMQ_POLLIN`：可读事件
- `revents`：实际触发的事件

---

### 知识点 5：C++ 内存对齐与字节序

**⚠️ 面试题：网络通信中字节序怎么处理？**

**字节序（Byte Order）**：
```
大端序（Big-Endian）：高字节在低地址 → 网络传输标准（TCP/IP）
小端序（Little-Endian）：低字节在低地址 → x86/ARM 默认

int32_t value = 0x12345678;
内存：0x78 0x56 0x34 0x12  ← 小端序（本项目直接这样写入）
```

**本项目为什么没处理字节序？**
→ MAVLink 协议规定用小端序，而 Qt 开发环境（x86/ARM）本身就是小端，`reinterpret_cast` 直接写入内存就是正确的。

**如果跨平台（大小端混合）怎么办？**
```cpp
// 网络字节序转换函数
uint32_t htonl(uint32_t hostlong);  // host to network long
uint16_t htons(uint16_t hostshort); // host to network short
uint32_t ntohl(uint32_t netlong);   // network to host long
```

**面试追问**：
- **什么是内存对齐？** → 编译器为了让 CPU 高效访问内存，会在结构体成员间插入填充字节
- **`#pragma pack(1)` 的作用？** → 取消对齐，结构体紧凑排列，网络协议常用

---

## Day 3 — 联调与编译问题

### 问题 1：头文件与实现文件变量名不一致
- **现象**：`error: 'm_drones' was not declared in this scope; did you mean 'm_drone'?`
- **原因**：`DroneManager.h` 中成员变量名为 `m_drone`，但 `.cpp` 中用的是 `m_drones`
- **解决**：统一变量名

**⚠️ 面试题：C++ 编译链接过程？头文件在哪个阶段被处理？**

```
源码(.cpp) → 预处理(#include展开) → 编译(生成.o) → 链接(合并所有.o → 可执行文件)
```

| 阶段 | 工具 | 做了什么 |
|------|------|----------|
| 预处理 | `g++ -E` | 展开 `#include`、宏替换、删除注释 |
| 编译 | `g++ -S` | 生成汇编代码 |
| 汇编 | `g++ -c` | 生成目标文件 `.o`（机器码） |
| 链接 | `ld` | 合并所有 `.o` + 库文件 → 可执行文件 |

**头文件的作用**：
- `#include` 是**文本复制粘贴**，预处理阶段直接展开
- 头文件只包含**声明**（函数声明、类定义、extern变量）
- `.cpp` 包含**定义**（函数体、变量内存分配）
- 链接器负责把 `.cpp` 中的定义和 `.h` 中的声明匹配

**面试追问**：
- **为什么头文件只放声明不放定义？** → 如果多个 `.cpp` 都 `#include` 同一个头文件，定义会被复制多份，链接时报重复定义错误
- **`inline` 函数为什么可以放头文件？** → `inline` 告诉编译器允许重复定义，链接时选一份即可

---

### 问题 2：connect 语法错误
- **现象**：`error: invalid use of non-static member function 'void SimulatedLink::generateTelemetry()'`
- **原因**：`connect` 第4个参数漏了 `&` 和类名前缀
- **错误**：`connect(m_timer, &QTimer::timeout, this, generateTelemetry);`
- **正确**：`connect(m_timer, &QTimer::timeout, this, &SimulatedLink::generateTelemetry);`

**⚠️ 面试题：Qt connect 旧语法 vs 新语法？**

| 特性 | 旧语法（字符串） | 新语法（函数指针） |
|------|----------------|-------------------|
| 写法 | `connect(sender, SIGNAL(sig()), receiver, SLOT(slot()))` | `connect(sender, &Sender::sig, receiver, &Receiver::slot)` |
| 检查时机 | 运行时 | **编译期** |
| 错误发现 | 运行时报 warning | 编译直接报错 |
| IDE 支持 | 无法跳转 | ✅ 可跳转、可重构 |
| 性能 | 略慢（字符串查找） | 略快（直接函数指针） |

**推荐使用新语法**，编译期就能发现错误。

**面试追问**：
- **`SLOT` 和 `SIGNAL` 宏做了什么？** → 把函数名转成字符串（如 `"clicked()"`），运行时通过 MOC 生成的元信息查找匹配
- **Lambda 能做槽函数吗？** → 可以，`connect(sender, &Sender::sig, [=](){ ... });`，但要注意生命周期（sender 和 receiver 都销毁后不能再调用）

---

### 问题 3：const 方法中调用 emit 报错
- **现象**：`error: passing 'const DroneManager' as 'this' argument discards qualifiers`
- **原因**：`unregisterDrone` 声明为 `const`，但 `emit droneRemoved()` 需要修改内部状态
- **解决**：去掉 `const` 修饰符

**⚠️ 面试题：const 成员函数的限制？**

```cpp
class Foo {
    int value;
public:
    void set(int v) { value = v; }        // 非const，可以修改成员
    int get() const { return value; }      // const，不能修改成员
};

// const 对象只能调用 const 方法
const Foo f;
f.set(10);    // ❌ 编译错误
f.get();      // ✅
```

**const 方法的限制**：
- 不能修改任何非 `mutable` 成员变量
- 不能调用非 `const` 方法
- 不能 `emit` 信号（信号本质是函数调用，可能触发非const槽）

**`mutable` 关键字**：允许在 `const` 方法中修改特定成员
```cpp
mutable int cacheCount;  // const 方法也能改
```

---

### 问题 4：缺少头文件导致编译错误
- **现象**：`error: incomplete type 'QThread' used in nested name specifier`
- **原因**：`DataDispatcher.cpp` 用了 `QThread::msleep()` 但没 `#include <QThread>`
- **解决**：添加 `#include <QThread>`

**⚠️ 面试题：为什么有些函数不 include 也能编译通过？**

- **隐式包含**：某些 Qt 头文件会间接 include 其他头文件（如 `<QWidget>` 会包含 `<QObject>`）
- **前向声明**：如果只用了指针/引用，可以不 include 完整头文件（`class QThread;`）
- **但不推荐**：依赖隐式包含会导致修改一个头文件后大面积编译失败

**最佳实践**：
> 每个 `.cpp` 文件应该 include 它**直接使用**的所有头文件，不依赖隐式包含。

---

### 知识点 5：多文件项目编译组织

**⚠️ 面试题：一个有 50 个 .cpp 文件的项目，怎么组织编译？**

**Makefile/qmake 的作用**：
1. 只编译**修改过的** `.cpp` 文件（增量编译）
2. 头文件变化时，重新编译所有 `#include` 它的 `.cpp`
3. 最后链接所有 `.o` 成可执行文件

**编译时间优化**：
| 技术 | 原理 |
|------|------|
| 增量编译 | 只编译改动的文件，不重新编译全部 |
| 预编译头（PCH） | 把不常改的头文件（Qt、STL）预编译成 `.gch` |
| 并行编译 | `make -j8`，8个文件同时编译 |
| 前向声明 | 减少头文件依赖，减少重编译范围 |

**本项目的编译结构**：
```
Drone.pro → qmake → Makefile
Makefile 管理 24 个 .cpp 的编译顺序和依赖关系
最终链接：所有 .o + libQt5Widgets + libQt5Sql + libzmq → Drone 可执行文件
```

---

---

## Day 4 — SQLite 数据库层

### 知识点 1：单例模式（Singleton）— 面试必问

**⚠️ 面试题：单例模式有几种实现？各有什么优缺点？**

**本项目用法**：
```cpp
class DatabaseManager {
public:
    static DatabaseManager &instance();
private:
    DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;
};
```

**三种单例实现对比**：

| 实现方式 | 代码 | 线程安全？ | 延迟初始化？ |
|---------|------|-----------|-------------|
| **饿汉式** | 程序启动时创建静态对象 | ✅ 安全 | ❌ 一开始就创建 |
| **懒汉式 + mutex** | 首次调用时加锁创建 | ✅ 安全 | ✅ 按需创建 |
| **C++11 局部静态** | `static DatabaseManager db;` | ✅ 安全（C++11标准保证） | ✅ 按需创建 |

**本项目采用的是 C++11 局部静态**，最简洁且安全：
```cpp
DatabaseManager &DatabaseManager::instance() {
    static DatabaseManager db;  // C++11 保证线程安全
    return db;
}
```

**面试追问**：
- **为什么禁止拷贝？** → 如果允许拷贝，就会创建多个实例，违背单例语义
- **`delete` 拷贝构造 vs 声明为 private 哪个好？** → `delete` 更好，C++11 语法，编译期报错更清晰
- **单例有什么缺点？** → ①全局状态，难以测试（mock）；②生命周期不可控；③多线程需额外考虑；④依赖注入是更现代的替代方案
- **什么时候不用单例？** → 需要多个实例、需要依赖注入测试、需要跨线程共享状态时

---

### 知识点 2：SQL 预编译语句（Prepared Statement）— 面试高频

**⚠️ 面试题：为什么不能直接拼 SQL 字符串？**

**错误写法（SQL 注入风险）**：
```cpp
QString sql = "SELECT * FROM drones WHERE name='" + droneName + "'";
query.exec(sql);
// 如果 droneName = "'; DROP TABLE drones; --"
// 整张表就没了
```

**正确写法（参数化查询）**：
```cpp
query.prepare("SELECT * FROM drones WHERE name=?");
query.addBindValue(droneName);
query.exec();
// droneName 是什么内容都安全，SQL 引擎会自动转义
```

**预编译原理**：
```
1. prepare("INSERT INTO ... VALUES(?, ?, ?)")  → SQL 引擎解析语法，生成执行计划
2. addBindValue(1)                              → 绑定参数值
3. exec()                                      → 执行，不重新解析 SQL
```

**三大好处**：
| 好处 | 说明 |
|------|------|
| **防注入** | 参数和 SQL 语法分离，用户输入永远是数据不是代码 |
| **性能** | 多次执行同一条 SQL 时，只解析一次，后续只换参数 |
| **可读性** | 比字符串拼接清晰得多 |

**面试追问**：
- **`?` 占位符和 `:name` 命名占位符的区别？** → `?` 按位置绑定，`:name` 按名字绑定，功能一样，命名的更清晰
- **SQLite 和 MySQL 的预编译有区别吗？** → MySQL 的预编译是真正在服务端缓存执行计划；SQLite 是单文件库，预编译主要防注入和减少解析次数

---

### 知识点 3：事务（Transaction）与外键约束

**⚠️ 面试题：数据库事务的 ACID 是什么？**

| 特性 | 含义 | 本项目体现 |
|------|------|-----------|
| **A** 原子性 | 一组操作要么全成功要么全回滚 | `removeLog` 先删轨迹点再删日志，如果中间失败需要回滚 |
| **C** 一致性 | 事务前后数据完整性不变 | 外键约束保证不会出现"孤儿"轨迹点 |
| **I** 隔离性 | 并发事务互不干扰 | SQLite 默认是序列化执行 |
| **D** 持久性 | 提交后数据永久保存 | SQLite WAL 模式下写入即持久 |

**本项目的外键约束**：
```sql
PRAGMA foreign_keys = ON;  -- 必须开启，默认是 OFF！

CREATE TABLE waypoints (
    ...
    FOREIGN KEY(path_id) REFERENCES flight_paths(id)
);
```

**⚠️ 面试题：SQLite 的外键约束为什么默认关闭？怎么开启？**

- SQLite 为了兼容老版本，默认不检查外键
- 必须在每次连接后执行 `PRAGMA foreign_keys = ON`
- 本项目在 `DatabaseManager::initialize()` 里开启

**面试追问**：
- **DELETE CASCADE 知道吗？** → 级联删除，父表删除时自动删子表关联行。本项目 `removeLog` 手动先删轨迹点，没用 CASCADE
- **什么时候用事务？** → 批量插入时用事务包裹，性能提升巨大（SQLite 单条插入~10ms，事务包裹批量插入~0.01ms/条）

---

### 知识点 4：RAII 与数据库连接管理

**⚠️ 面试题：RAII 是什么？在数据库编程中怎么用？**

**RAII（Resource Acquisition Is Initialization）**：
- 资源获取即初始化，C++ 核心编程范式
- 构造函数获取资源，析构函数释放资源
- 利用栈对象自动析构的特性，保证资源不泄漏

**本项目的 RAII 体现**：
```cpp
// DatabaseManager 析构自动关闭数据库
DatabaseManager::~DatabaseManager() { close(); }

// QSqlQuery 的 RAII 用法
{
    QSqlQuery query(db);   // 构造：绑定数据库连接
    query.exec("...");
}   // 析构：query 自动释放
```

**数据库连接池 vs 单连接**：

| 方案 | 优点 | 缺点 |
|------|------|------|
| 单连接（本项目） | 简单，SQLite 本身支持多线程读 | 写操作串行，高并发瓶颈 |
| 连接池 | 高并发，多连接并行读写 | 复杂，需要管理连接生命周期 |

**面试追问**：
- **`QSqlDatabase::addDatabase()` 多次调用会怎样？** → 同一个 connectionName 只会创建一个连接，第二次调用会覆盖之前的
- **智能指针能管理数据库连接吗？** → 可以，`std::shared_ptr<QSqlDatabase>` 但 Qt 自己有对象树管理，一般不需要

---

### 知识点 5：DAO 模式（Data Access Object）— 面试常问

**⚠️ 面试题：什么是 DAO 模式？有什么好处？**

**DAO 模式**：
```
业务逻辑层（DroneManager）
        ↓ 调用
数据访问层（DroneDAO）    ← 封装所有 SQL 操作
        ↓ 操作
数据库（SQLite）
```

**好处**：
| 好处 | 说明 |
|------|------|
| **解耦** | 业务层不写 SQL，只调 DAO 接口 |
| **可替换** | 换数据库只改 DAO，业务层不动 |
| **可测试** | 可以 mock DAO 来测试业务逻辑 |
| **集中管理** | SQL 集中在 DAO 里，方便优化和维护 |

**本项目的 DAO 结构**：
```
DroneDAO      → drones 表 CRUD
FlightLogDAO  → flight_logs + flight_log_points 表 CRUD
GeoFenceDAO   → geofence_zones 表 CRUD
```

**面试追问**：
- **DAO 和 ORM 的区别？** → DAO 是自己写 SQL 封装；ORM 是自动映射对象到表（如 Hibernate、Qt ORM）
- **Repository 模式和 DAO 的区别？** → Repository 是 DDD 的概念，更偏向聚合根的操作；DAO 更偏向数据库表级别的 CRUD
- **什么时候用 ORM 什么时候手写 SQL？** → 简单 CRUD 用 ORM 效率高；复杂查询、性能敏感场景手写 SQL 更灵活

---

### 知识点 6：SQLite 特性与适用场景

**⚠️ 面试题：SQLite 和 MySQL/PostgreSQL 的区别？什么时候用 SQLite？**

| 维度 | SQLite | MySQL/PostgreSQL |
|------|--------|-----------------|
| 架构 | 嵌入式，库文件 | C/S 架构，独立服务 |
| 并发 | 写操作串行 | 多连接并行 |
| 存储 | 单个 .db 文件 | 服务端管理 |
| 部署 | 零配置，拷贝即用 | 需要安装配置 |
| 事务 | 支持，但性能一般 | 完整 ACID，高性能 |

**适用场景**：
- ✅ 嵌入式应用（本项目、手机 App）
- ✅ 单机桌面应用
- ✅ 原型开发、单元测试
- ✅ 数据量小（< 1TB）
- ❌ 高并发 Web 服务
- ❌ 多服务器分布式系统

**面试追问**：
- **SQLite 的 WAL 模式是什么？** → Write-Ahead Logging，允许读写并发，提升性能
- **SQLite 支持外键吗？** → 支持，但默认关闭，需要 `PRAGMA foreign_keys = ON`
- **本项目为什么选 SQLite？** → 单机桌面应用，无需部署数据库服务器，零配置，数据量小

---

### 知识点 7：JSON 在数据库中的存储

**⚠️ 面试题：复杂数据结构怎么存进关系型数据库？**

**本项目的例子**：`GeoFenceZone` 的 `points` 字段是 `QList<QPair<double,double>>`，数据库没有数组类型

**方案对比**：

| 方案 | 做法 | 优点 | 缺点 |
|------|------|------|------|
| **JSON 字符串** | 存 `[{lat:39,lng:116},...]` | 灵活，不需要改表结构 | 查询慢，不能用索引 |
| **关联表** | 单独建 points 表 | 可查询，可索引 | 表多，JOIN 复杂 |
| **序列化二进制** | protobuf/capnp 序列化 | 体积小，速度快 | 不可读，调试困难 |

**本项目选择 JSON**：
```cpp
// 写入：对象 → JSON 字符串
QJsonArray arr;
for (const auto &p : zone.points) {
    QJsonObject obj;
    obj["lat"] = p.first;
    obj["lng"] = p.second;
    arr.append(obj);
}
query.addBindValue(QJsonDocument(arr).toJson());

// 读取：JSON 字符串 → 对象
QJsonDocument doc = QJsonDocument::fromJson(pointsStr.toUtf8());
QJsonArray arr = doc.array();
```

**面试追问**：
- **MySQL 有 JSON 类型吗？** → MySQL 5.7+ 有 JSON 类型，支持 JSON 函数查询，但性能不如原生列
- **什么时候用 JSON 存储？** → 配置信息、元数据、不需要频繁查询的嵌套数据
- **SQLite 有 JSON 函数吗？** → SQLite 3.9+ 支持 `json_extract()` 等函数

---

## Day 5 — 地理围栏与坐标变换算法

### 知识点 1：Haversine 球面距离公式（面试高频）

**⚠️ 面试题：如何在球面上计算两点距离？为什么不用勾股定理？**

**Haversine 公式**：
```
a = sin²(Δlat/2) + cos(lat1)·cos(lat2)·sin²(Δlng/2)
c = 2 · atan2(√a, √(1-a))
d = R · c    （R = 6371km）
```

**为什么不用欧氏距离**：
| 维度 | 欧氏距离（平面） | Haversine（球面） |
|------|-----------------|-------------------|
| 假设 | 地球是平面 | 地球是球体 |
| 短距 | 误差可接受（< 1km 时 < 0.1%） | 精度高 |
| 长距 | ❌ 误差巨大（1000km 时偏差 > 20%） | ✅ 误差 < 0.5% |
| 计算量 | 低 | 略高（三角函数） |

**本项目用法**：GPS 坐标是经纬度（球面坐标），禁飞区检测需要精确到几十米，必须用球面公式。

**面试追问**：
- **`atan2` vs `atan` 的区别？** → `atan2` 自动处理象限，返回 `(-π, π]`，比 `atan` 更安全
- **Vincenty 公式知道吗？** → 更精确的椭球面公式，精度 0.5mm，但计算量比 Haversine 大很多，本项目不需要这么高精度

---

### 知识点 2：PNPOLY 射线法 — 点在多边形内判定（面试常考）

**⚠️ 面试题：如何判断一个点是否在多边形内部？**

**思路**：从被测点向右发一条水平射线，统计与多边形边的交点个数：
- **奇数次** → 点在内部
- **偶数次** → 点在外部

```
       多边形
    ┌───────┐
    │   P●──┼────→ 射线（2个交点 → 偶数 → 外部）
    │   Q●  │
    └───┼───┘
        └────→ 射线（1个交点 → 奇数 → 内部）
```

**关键实现**：
```cpp
if (((yi > lng) != (yj > lng)) &&              // 射线穿过边
    (lat < (xj - xi) * (lng - yi) / (yj - yi) + xi)) {  // 交点在射线右侧
    inside = !inside;
}
```

**边界情况处理**：
| 情况 | 处理方式 |
|------|----------|
| 点在顶点上 | 算一个交点（或返回 true） |
| 点在边上 | 算一个交点 |
| 射线穿过顶点 | 通过 `>=` vs `>` 避免重复计数 |
| 多边形有洞 | 标准 PNPOLY 不支持，需要用更复杂的算法 |

**时间复杂度**：O(n)，n 为多边形顶点数（本项目无人机 < 100 架，够用）

**面试追问**：
- **如果多边形有几万个顶点，O(n) 不够快怎么办？** → 用空间索引（R-tree、四叉树）预先过滤
- **还有其他算法吗？** → 转角法（Winding Number）、网格化预处理
- **PNPOLY 和转角法的区别？** → PNPOLY 只适合简单多边形，转角法能处理自相交多边形

---

### 知识点 3：中国坐标系体系（WGS84 / GCJ02 / BD09）— 面试常考

**⚠️ 面试题：为什么在中国用 GPS 坐标定位会偏？GPS 坐标和高德/百度地图的坐标为什么不一样？**

**三种坐标系**：
```
WGS84（GPS 原始坐标）
    │  ↓ 国测局加密（非线性偏移 300-500m）
GCJ02（火星坐标系，高德/腾讯）
    │  ↓ 百度二次加密
BD09（百度坐标系）
```

| 坐标系 | 使用者 | 特点 |
|--------|--------|------|
| **WGS84** | GPS 设备、Google Earth、OpenStreetMap | 全球通用，真实经纬度 |
| **GCJ02** | 高德地图、腾讯地图、苹果中国 | 国测局强制加密，非线性偏移 300-500m |
| **BD09** | 百度地图 | GCJ02 基础上进一步加密 |

**为什么存在 GCJ02？**
- 中国法律规定：所有公开地图必须使用 GCJ02 加密坐标
- 不允许直接显示 WGS84 坐标
- 加密算法是**国家机密**，但已被逆向工程破解（本项目用的是公开的反推公式）

**面试追问**：
- **中国境外坐标需要加密吗？** → 不需要，`outOfChina()` 判断范围外直接透传
- **加密算法是国家安全机密，你的代码哪里来的？** → 开源社区逆向工程的结果，所有地图 JS SDK 内部都实现了相同的算法，高德/百度官方文档里也有直接提供转换 API
- **经纬度 1° 大约多少米？** → 纬度 1° ≈ 111km（固定），经度 1° ≈ 111km·cos(lat)（随纬度变化）

---

### 知识点 4：迭代法反算 — 正向易、逆向难

**⚠️ 面试题：已知正向函数 y = f(x)，怎么求反函数 x = f⁻¹(y)？**

**本项目的场景**：
- 正向：`wgs84ToGcj02(wgs, → gcj)` — 已知 WGS84，算 GCJ02
- 逆向：`gcj02ToWgs84(gcj, → wgs)` — 已知 GCJ02，还原 WGS84
- GCJ02 的加密公式是三角级数，**无法解析求逆**

**迭代法原理**：
```cpp
wgs84ToGcj02(wgs0, → gcj0);       // 用当前猜测算出一个 gcj
误差 = gcj0 - 目标_gcj;             // 误差太大
wgs0 -= 误差;                       // 朝反方向修正
// 重复 5 次 → 收敛到 1e-4 度（~10m 精度）
```

**面试追问**：
- **为什么 5 次就够了？** → 加密函数是光滑的（sin/cos 组成），局部近似线性，收敛快
- **如果迭代不收敛怎么办？** → 可以加阻尼因子或二分法做 fallback
- **还有其他方案吗？** → 二分法、牛顿法（需要导数，这里没有解析导数）

---

### 知识点 5：边界情况处理（面试加分项）

**⚠️ 面试题：写算法时怎么考虑边界情况？**

本项目中处理的边界：

| 边界情况 | 处理方式 | 文件 |
|----------|----------|------|
| 少于 3 个顶点 | 直接返回 false（无法构成多边形） | `GeoFence.cpp:22` |
| 退化多边形 | 射线法自然处理，0 交点 = 外部 | `GeoFence.cpp` |
| 零半径圆 | 只有圆心在点上才算内部 | `GeoFence.cpp:40` |
| 禁飞区 disabled | 不检测，返回 false | `GeoFence.cpp:58` |
| 中国境外坐标 | 透传不偏移 | `CoordTransform.cpp:28` |
| 迭代不收敛 | 硬限制 5 次迭代 | `CoordTransform.cpp:69` |

**面试官想听什么**：
> "我先考虑正常路径，然后列一遍所有边缘情况：空的、负值的、越界的、禁用状态的……每一步都加防御。"

---

## Day 6 — 路径规划算法

### 知识点 1：A* 搜索算法（面试必问）

**⚠️ 面试题：A* 算法的原理？和 Dijkstra 有什么区别？**

**A* 核心公式**：`f(n) = g(n) + h(n)`
- `g(n)`：起点到当前节点的实际代价
- `h(n)`：当前节点到终点的**启发式估计**（必须是可接受的，即不高估）

**Dijkstra vs A***：

| 对比 | Dijkstra | A* |
|------|----------|-----|
| 评估函数 | 只有 `g(n)` | `g(n) + h(n)` |
| 搜索方向 | 均匀扩展（像水波纹） | 有方向地朝目标搜索 |
| 时间复杂度 | O(V²) 或 O((V+E)logV) | 取决于 h(n) 质量，通常远小于 Dijkstra |
| 最优性 | ✅ 保证最短路径 | ✅ h(n) 可接受时保证最优 |
| 适用场景 | 无目标的全图搜索 | 有明确起点终点的寻路 |

**本项目的 h(n)**：Haversine 球面距离，是真实距离的下界（直线距离 ≤ 实际路径距离），满足可接受性条件。

**面试追问**：
- **h(n) 什么时候不可接受？** → 如果 h(n) 高估了真实距离，A* 可能找到次优解
- **h(n) = 0 时 A* 退化成什么？** → Dijkstra
- **A* 的时间复杂度？** → 最坏 O(b^d)（b=分支因子，d=最优解深度），实际远优于最坏情况
- **IDA* 知道吗？** → 迭代加深 A*，内存使用 O(d)，适合大规模状态空间

**⚠️ 面试题：本项目中 A* 的网格化设计？**

| 设计决策 | 选择 | 原因 |
|----------|------|------|
| 网格分辨率 | 100m | 无人机飞行尺度（km 级），100m 足够精细 |
| 移动方向 | 8 方向 | 允许斜向移动，路径更自然 |
| 禁飞区判定 | 格子中心点 | 简化计算，如果格子一半在禁飞区内就标记为障碍 |
| 启发函数 | Haversine | GPS 坐标是球面坐标，不能用欧氏距离 |

---

### 知识点 2：Douglas-Peucker 路径简化算法

**⚠️ 面试题：如何精简路径中的冗余航点？**

**算法步骤**：
1. 连接首尾两点，画一条线段
2. 找到离线段最远的点，记录距离 d
3. 如果 d > ε（阈值），以该点为分割点，递归处理左右两段
4. 如果 d ≤ ε，删除中间所有点

```
原始路径：  A---B---C---D---E---F---G
                    ↑ 最远点 D，距离 > ε
                递归左段 A-D：保留 A, D
                递归右段 D-G：保留 D, G
结果：      A-----------D-----------G
```

**时间复杂度**：最坏 O(n²)，平均 O(n log n)

**本项目参数**：`epsilonMeters = 5.0`（5 米精度阈值，航点偏差超过 5 米才保留）

**⚠️ 面试题：Douglas-Peucker 点到线段距离怎么算？**

**不能用欧氏距离**（经纬度是球面坐标），本项目用 Haversine 计算投影点到线段端点的距离：
1. 计算点在直线上的投影参数 `param = dot / lenSq`
2. `param` 限制在 [0,1] 范围内（防止投影到线段延长线上）
3. 用 Haversine 计算原点到投影点的球面距离

---

### 知识点 3：航线冲突检测与自动规避

**⚠️ 面试题：如何检测航线是否穿越禁飞区？**

**本项目方案：分段插值检测**

| 检测方式 | 优点 | 缺点 |
|----------|------|------|
| 只检查航点 | 简单快速 | ❌ 两点之间穿过禁飞区会被漏掉 |
| 分段插值（本项目） | 不会漏检 | 计算量略大 |
| 射线与多边形相交 | 精确 | 实现复杂 |

**本项目实现**：相邻航点之间插 5 个采样点，加上航点本身共 7 个检测点，覆盖 100m 网格分辨率下不会漏检。

**面试追问**：
- **5 个采样点够不够？** → 100m 分辨率下，航点间距通常 200-500m，5 个采样点间距约 40-100m，与网格分辨率匹配，够用
- **如果要更精确怎么办？** → 用线段与圆形/多边形的几何相交判定，数学更复杂但无漏检

**⚠️ 面试题：自动规避的策略？**

本项目策略：
```
直连检测 → 无冲突 → 返回直连（最短路径）
       ↓ 有冲突
    A* 绕行 → 返回绕行路径
```

更高级的策略（面试加分）：
| 策略 | 描述 |
|------|------|
| **绕行**（本项目） | A* 网格搜索绕开禁飞区 |
| **穿行** | 如果禁飞区有高度限制，从上方飞越 |
| **分段绕行** | 将航线分段，每段独立规避 |
| **势场法** | 禁飞区产生斥力场，无人机沿合力方向飞行 |

---

### 知识点 4：优先队列（堆）在 A* 中的应用

**⚠️ 面试题：A* 的 open set 用什么数据结构？为什么？**

**本项目用 `std::priority_queue`（最小堆）**：

| 操作 | 数据结构 | 时间复杂度 |
|------|----------|-----------|
| 插入节点 | 最小堆 push | O(log n) |
| 取最小 f 值节点 | 最小堆 top + pop | O(log n) |
| 检查是否在 open set 中 | QMap 辅助 | O(log n) |

**为什么不用数组/链表**：每次取 f 值最小的节点，如果用数组需要 O(n) 遍历找最小值。

**面试追问**：
- **最小堆 vs 最大堆？** → A* 需要取最小 f 值，用最小堆；Dijkstra 同理
- **Fibonacci 堆知道吗？** → 理论上更优（decrease-key O(1)），但实现复杂，实际很少用
- **`std::priority_queue` 和手写堆的区别？** → STL 封装，接口简洁，但不支持 decrease-key；本项目通过重复插入+closedSet 去重规避此问题

---

### 知识点 5：航点数据模型设计

**⚠️ 面试题：WayPoint 结构体设计考虑了什么？**

```cpp
struct WayPoint {
    double latitude;   // 纬度
    double longitude;  // 经度
    double altitude;   // 高度(m)
    double speed;      // 建议飞行速度(m/s)
    double hoverTime;  // 悬停时间(s)
    int action;        // 动作：0=无，1=拍照，2=录像，3=抛投
};
```

**设计考虑**：

| 字段 | 为什么需要 | 面试考点 |
|------|-----------|----------|
| 经纬高 | 三维空间定位 | GPS 坐标是 WGS84 |
| 速度 | 每段可不同速度 | 节能模式/快速到达 |
| 悬停时间 | 巡检场景需要定点停留 | 任务多样性 |
| action | 任务执行 | 可扩展为枚举/位掩码 |

**面试追问**：
- **为什么不用 `int action` 而不用 `enum`？** → 序列化简单，数据库存整数方便；也可以用 enum，功能一样
- **航点之间怎么连线？** → 直线飞行（大圆航线），本项目路径规划模块计算中间点

---

### 知识点 6：QMap 在 A* 中的 used/open set 管理

**⚠️ 面试题：本项目 A* 为什么用 QMap 而不用 std::set 或 unordered_set？**

| 容器 | 用途 | 优点 | 缺点 |
|------|------|------|------|
| `QMap<qint64, double>` | gScore 存储 | 查找 O(log n)，有序 | 比哈希慢 |
| `QMap<qint64, bool>` | closedSet | 与 gScore 统一风格 | 同上 |
| `std::unordered_set` | closedSet | O(1) 查找 | 需要自定义 hash |
| `std::set` | open set | 自动排序 | 不如 priority_queue 高效 |

**本项目用 `qint64` 作为 key**：`(row << 32) | col` 将二维坐标压缩为 64 位整数，避免 `std::pair<int,int>` 的 hash 问题。

**面试追问**：
- **为什么不用 `std::pair<int,int>` 做 key？** → QMap 需要 key 可比较，QPair 支持 `<` 比较，也可以；但 `qint64` 更简洁高效
- **哈希表和红黑树在 A* 中哪个更好？** → 哈希表 O(1) vs 红黑树 O(log n)，节点数 < 10000 时差异不大

---

## Day 7 — 模块联调 + 阶段二验收

### Bug 1：FlightLogDAO::removeLog 缺少事务保护

- **问题**：先 `DELETE flight_log_points` 再 `DELETE flight_logs`，如果第二步失败，航点已丢失而日志还在，数据不一致
- **修复**：用 `db.transaction()` / `db.commit()` / `db.rollback()` 包裹，两步要么全成功要么全回滚
- **知识点**：⚠️ 数据库事务 — ACID 的原子性

**⚠️ 面试题：什么时候应该用事务？**

| 场景 | 是否需事务 | 原因 |
|------|-----------|------|
| 单条 INSERT | ❌ 不需要 | 单条操作本身就是原子的 |
| 批量 INSERT（1000条） | ✅ 强烈建议 | 事务包裹后速度提升 100 倍（一次磁盘写入 vs 1000次） |
| 关联表级联删除（本项目） | ✅ 必须 | 两步操作，防止部分成功导致数据不一致 |
| 转账（A扣款 → B加款） | ✅ 必须 | 业务要求原子性 |

**面试追问**：
- **SQLite 事务和 MySQL 事务有区别吗？** → SQLite 默认自动提交，每句 SQL 一个事务；MySQL InnoDB 也是自动提交。手动 `BEGIN TRANSACTION` 可以把多个语句合并到一个事务
- **事务隔离级别有哪些？** → READ UNCOMMITTED / READ COMMITTED / REPEATABLE READ / SERIALIZABLE。SQLite 默认 SERIALIZABLE，最严格

---

### Bug 2：SimulatedLink 电量初始化异常

- **问题**：`bounded(20) * 0.1` 导致电量始终是 80 或 81，本意是 80-99 随机
- **原因**：`* 0.1` 把整数随机范围 0-19 压缩到 0.0-1.9，加 80 后永远是 80-81
- **修复**：去掉 `* 0.1`，改为 `bounded(20)` 直接得到 0-19 整数
- **知识点**：⚠️ 隐式类型转换 — int → double → int 的精度丢失

**⚠️ 面试题：C++ 隐式类型转换有哪些风险？**

```cpp
int x = 3;
double y = 2.7;
int z = x + y;  // z = 5，小数部分被丢弃！
```

**本项目 Bug 分析**：
```cpp
// 原代码
drone.batteryPercent = 80 + QRandomGenerator::global()->bounded(20) * 0.1;
// 执行顺序：
//   1. bounded(20) → int (0-19)
//   2. int * 0.1 → double (0.0-1.9)    ← int 被隐式提升为 double
//   3. 80 + double → double (80.0-81.9)
//   4. 赋值给 int → 截断为 80 或 81   ← 精度丢失
```

---

### Bug 3：MavlinkParser 缺失方法实现

- **问题**：`encodeVfrHud`、`encodeSysStatus`、`decodeGlobalPosition`、`decodeSysStatus` 4个方法只有声明没有实现，如果被调用会链接失败
- **修复**：补全全部实现，encode 系列序列化数据到帧，decode 系列反序列化帧到 DroneInfo
- **知识点**：⚠️ C++ 链接错误 — 声明和定义必须匹配

### Bug 4：`#endif` 注释不规范

- **问题**：`#endif MAVLINKMESSAGE_H` 等写法不符合 C++ 标准（`#endif` 后面只能跟注释）
- **修复**：改为 `#endif // MAVLINKMESSAGE_H`
- **知识点**：⚠️ 预处理器指令语法

---

### 阶段二验收总结

**测试结果**：
- **算法测试**：53/53 通过
- **数据库测试**：全部 CRUD 操作通过
- **数据流全链路**：主程序启动 → 数据库建表 → 模拟5架无人机 → ZeroMQ PUB 分发 → 控制台正确输出遥测
- **编译**：0 error, 0 warning

**阶段二完成的功能**：
1. ✅ SQLite 数据库：7 张表，3 个 DAO，完整 CRUD
2. ✅ 地理围栏：3 种形状检测（圆/矩形/多边形）+ 缓冲区预警
3. ✅ 坐标变换：WGS84 ↔ GCJ02 ↔ BD09 全套
4. ✅ 路径规划：A* 搜索 + Douglas-Peucker 简化 + 禁飞区规避
5. ✅ 数据流全链路：模拟 → 解析 → 分发 → 管理 → 入库

**项目总进度**：37/90 任务完成 (41%)

---

## Day 8 — 主窗口框架 + 无人机列表 UI

### 知识点 1：QSplitter 布局（面试常考）

**⚠️ 面试题：QSplitter vs QHBoxLayout 的区别？**

| 特性 | QSplitter | QHBoxLayout |
|------|-----------|-------------|
| 用户可拖拽调整 | ✅ 有拖拽手柄 | ❌ 自动分配固定比例 |
| 最小尺寸 | `setMinimumSize` | 同样支持 |
| 伸缩因子 | `setStretchFactor(idx, 0/1)` | `setStretchFactor` |
| 嵌套 | ✅ 水平和垂直可嵌套 | ✅ 同样支持 |
| 保存状态 | `saveState()` / `restoreState()` | ❌ 不支持 |
| 典型场景 | IDE 面板、文件管理器 | 表单、按钮行 |
| 本项目 | 主窗口三栏：左200px/中伸缩/右280px | 子面板内部布局 |

```cpp
// 三栏布局核心代码
m_mainSplitter = new QSplitter(Qt::Horizontal, this);
m_mainSplitter->addWidget(leftPanel);    // index 0
m_mainSplitter->addWidget(m_centerStack); // index 1
m_mainSplitter->addWidget(m_flightParam); // index 2
m_mainSplitter->setStretchFactor(0, 0);  // 左不伸缩
m_mainSplitter->setStretchFactor(1, 1);  // 中伸缩
m_mainSplitter->setStretchFactor(2, 0);  // 右不伸缩
m_mainSplitter->setSizes({200, 700, 280});
```

**面试追问**：
- **QSplitter 的数据是怎么保存的？** → `saveState()` 返回 QByteArray，用 `QSettings` 保存到注册表，下次启动时 `restoreState()` 恢复
- **splitter 的 Handle 能自定义吗？** → 可以，`setHandleWidth()` 和 `setChildrenCollapsible(false)`

---

### 知识点 2：QStackedWidget 页面切换模式

**⚠️ 面试题：QStackedWidget vs QTabWidget vs QDialog（弹出窗口）页面切换方案对比**

| 方案 | 特点 | 适合场景 |
|------|------|---------|
| **QStackedWidget + QListWidget**（本项目） | 导航栏和内容区分离，页面切换无缝 | 监控系统、后台管理，导航在左侧 |
| **QTabWidget** | 自带标签页头，自带切换 | 设置对话框、少页面（<5）场景 |
| **QDialog** | 弹出独立窗口，模态/非模态 | 临时编辑、设置弹窗 |
| **QStackedWidget + QPushButton** | 按钮控制切换 | 引导流程 A→B→C→完成 |

---

### 知识点 3：QTableWidget（面试必问）

**⚠️ 面试题：QTableWidget vs QTableView + QAbstractTableModel？**

```cpp
// 为什么本项目选 QTableWidget
QTableWidget *table = new QTableWidget(0, 5, this);
table->setHorizontalHeaderLabels({"ID", "名称", "状态", "电量", "信号"});
table->setItem(row, 0, new QTableWidgetItem("1"));                // 文本
table->setItem(row, 1, new QTableWidgetItem("UAV-01"));           // 文本
auto *statusItem = new QTableWidgetItem("飞行中");
statusItem->setForeground(QColor("#27ae60"));                     // 颜色
table->setItem(row, 2, statusItem);
table->setCellWidget(row, 3, new QProgressBar);                    // 嵌入 Widget！
```

| 对比 | QTableWidget | QTableView + Model |
|------|-------------|-------------------|
| 代码量 | 少，setItem / setCellWidget 直接操作 | 多，需要定义 Model 类 |
| 数据量 | < 100 行 | > 1000 行 |
| 自定义控件 | ✅ `setCellWidget` 一行嵌入 | ⚠️ 需要 delegate |
| 排序 | `setSortingEnabled(true)` 内置 | 需在 Model 中重写 sort |
| 拖拽 | 支持 | 需额外代码 |
| 本项目选 | ✅ 无人机最多几十架 | ❌ 过度设计 |

**⚠️ 面试题：为什么电池用 QProgressBar 而不是画图？**

`setCellWidget(row, col, widget)` 是 QTableWidget 的核心优势——单元格里可以放任何 QWidget（按钮、进度条、下拉框、甚至另一个表格），不需要手写 QPainter。

```cpp
auto *bar = new QProgressBar;
bar->setRange(0, 100);
bar->setValue(d.batteryPercent);
bar->setStyleSheet("QProgressBar::chunk{background:#27ae60;}");  // 绿色
m_table->setCellWidget(row, 3, bar);   // 嵌入
```

---

### 知识点 4：Qt 的 Item Data 机制

**⚠️ 面试题：Qt::UserRole 是干什么的？怎么用？**

```cpp
// 存：每个表格行藏一个「无人机ID」
auto *idItem = new QTableWidgetItem("1");
idItem->setData(Qt::UserRole, 1);      // 存 int
m_table->setItem(row, 0, idItem);

// 取：选中某行时取出藏着的 ID
int droneId = m_table->item(row, 0)->data(Qt::UserRole).toInt();
```

**Qt 预定义的 ItemRole**：

| Role | 类型 | 作用 |
|------|------|------|
| `Qt::DisplayRole` | QString | 显示的文本（默认） |
| `Qt::ForegroundRole` | QColor / QBrush | 文字颜色 |
| `Qt::BackgroundRole` | QBrush | 背景颜色 |
| `Qt::FontRole` | QFont | 字体 |
| `Qt::TextAlignmentRole` | Qt::Alignment | 对齐方式 |
| `Qt::UserRole` | QVariant | 用户任意数据（如存 ID） |
| `Qt::UserRole + 1` | QVariant | 可以继续扩展 |

**面试追问**：
- **UserRole 和 UserRole+1 有什么区别？** → 没有区别，只是一种命名约定，让你可以存多种不同类型的数据
- **QTableWidgetItem 的生命周期？** → 一旦 `setItem`，QTableWidget 接管所有权，不需要手动 delete

---

### 知识点 5：QGroupBox + QFormLayout 参数面板

**⚠️ 面试题：QGroupBox vs QFrame？**

| 特性 | QGroupBox | QFrame |
|------|-----------|--------|
| 标题 | ✅ 内置标题 `new QGroupBox("位置")` | ❌ 需要额外 QLabel |
| 可勾选 | ✅ `setCheckable(true)` 整组开关 | ❌ |
| 快捷键 | ✅ Alt+字母聚焦子控件 | ❌ |
| 快捷键例子 | `new QGroupBox("&位置")` → Alt+W 聚焦 | ❌ |

**QFormLayout vs QGridLayout 场景对比**：

```cpp
// QFormLayout（本项目）— 代码少
auto *form = new QFormLayout;
form->addRow("纬度:", latValue);   // ← 一行搞定
form->addRow("经度:", lngValue);

// QGridLayout — 代码多，但布局更灵活
auto *grid = new QGridLayout;
grid->addWidget(new QLabel("纬度:"), 0, 0);
grid->addWidget(latValue, 0, 1);
grid->addWidget(new QLabel("经度:"), 1, 0);
grid->addWidget(lngValue, 1, 1);
```

---

### 知识点 6：QLabel 富文本

**⚠️ 面试题：QLabel 支持 HTML 吗？**

QLabel 支持 HTML 子集（`<span>`、`<b>`、`<font>`、`<a>`、`<img>`），适合简单的富文本场景。

```cpp
// 同一行不同颜色
m_nameLabel->setText(
    QString("<span style='color:#2c3e50;'>%1</span>"
            "<span style='color:#7f8c8d;'> (ID:%2)</span>")
    .arg(drone.name).arg(drone.id));
```

**优点**：不需要两个 QLabel + 一个 QHBoxLayout，一个 QLabel 搞定。
**缺点**：不支持复杂 HTML（表格、表单、div），复杂的富文本应该用 QTextBrowser 或 QTextDocument。

---

### 知识点 7：QProgressBar 动态换色

```cpp
m_batteryBar->setValue(drone.batteryPercent);
QString color = d.batteryPercent > 60 ? "#27ae60" :
                d.batteryPercent > 20 ? "#f39c12" : "#e74c3c";
m_batteryBar->setStyleSheet(
    "QProgressBar::chunk{ background:" + color + "; }");
```

**⚠️ 面试题：setStyleSheet 和直接 setProperty 哪个好？**
- `setStyleSheet`：每次设置都会重新解析 CSS，频繁调用（100ms）有性能开销
- `setProperty` + `polish`：不重新解析 CSS，性能更好
- 本项目：5 架无人机 × 每秒 10 次 = 50 次 QSS 解析/秒，完全可接受

---

### 知识点 8：QTimer 更新时钟

```cpp
m_timer = new QTimer(this);
connect(m_timer, &QTimer::timeout, this, &StatusBarWidget::updateTime);
m_timer->start(1000);
```

**⚠️ 面试题：QTimer 有哪些精度问题？**
- QTimer 依赖事件循环，如果主线程有耗时操作（如数据库查询），定时器会延迟
- 精度通常是 1ms（Linux timerfd），但实际触发频率受事件循环影响
- `Qt::PreciseTimer` vs `Qt::CoarseTimer`：精确 vs 省电（精度 5% 误差可接受）
- 本项目用 `Qt::CoarseTimer`（默认），更新时间不需要精确到毫秒

**QTimer 实现原理**（Linux）：
```
QTimer::start(1000)
    ↓
Qt 调用 timerfd_create() 创建定时器 fd
    ↓
epoll 监听该 fd，1000ms 后可读
    ↓
事件循环读取 fd → 生成 QTimerEvent
    ↓
QObject::timerEvent() → emit timeout()
```

---

### 知识点 9：QSS 常见坑

本项目中遇到的 QSS 错误：

| 错误写法 | 正确写法 | 问题 |
|---------|---------|------|
| `border-botton` | `border-bottom` | 拼写错误 |
| `QListWidget::item::seleted` | `QListWidget::item:selected` | 伪状态用单冒号 + 拼写 |
| `font-weight：bold` | `font-weight:bold` | 冒号是全角：而不是半角: |
| `border-redius` | `border-radius` | 拼写错误 |
| `boder-radius` | `border-radius` | 拼写错误（缺 r） |

**调试方法**：运行程序时注意控制台报 `Unknown property XXX`，QSS 不会报编译错误，只会运行时警告。

---

### 当天总结

**Day 8 验收**：
- ✅ MainWindow 三栏布局（QSplitter）
- ✅ 导航菜单（QListWidget + QStackedWidget）
- ✅ 无人机列表（QTableWidget + 状态颜色 + 电量进度条）
- ✅ 飞行参数面板（QGroupBox + QFormLayout）
- ✅ 底部状态栏（QHBoxLayout + QTimer 更新时间）
- ✅ 数据联动：模拟链路 → DroneManager → 列表刷新 + 参数面板更新
- ✅ 编译：0 error, 0 warning, 0 crash
- ✅ QSS：0 解析警告

**项目进度**：41/90 任务完成 (45.6%)

---

## Day 9 — 自定义仪表盘控件 + 告警面板

### 知识点 1：QPainter 自定义绘制（CompassWidget）

**⚠️ 面试题：paintEvent 的执行流程？**

```
用户调用 update()
    ↓
Qt 将 paintEvent 加入事件队列（合并多个 update 为一次）
    ↓
事件循环取出，调用 paintEvent()
    ↓
QPainter 开始绘制（双缓冲，不会闪烁）
    ↓
绘制完成后 QPainter 自动析构
```

**`update()` vs `repaint()`**：

| 方法 | 行为 | 推荐 |
|------|------|------|
| `update()` | 合并到下一个事件循环，多次调用只重绘一次 | ✅ 推荐 |
| `repaint()` | 立即重绘，阻塞当前线程 | ❌ 不推荐（可能导致闪烁） |

**QPainter 坐标系统**（面试高频）：
- 原点左上角，x 向右，y 向下
- `qDegreesToRadians(deg)` — 角度转弧度
- `cos(θ+π/2)` 相当于 sin，航向 0° 是北（y 负方向），所以要 `qDegreesToRadians(m_heading - 90)`

**航向箭头数学**：
```
航向 0° = 北 = (0, -1) 方向
QPainter 坐标系 y 轴向下，所以：
  rad = heading - 90  （0°→-90°=指向正上方）
  箭头末梢 = (cx + r*cos(rad), cy + r*sin(rad))
  箭头头部 = 末梢两边各偏 ±20° 画三角形
```

**⚠️ 面试题：QPainter 反走样原理？**
`setRenderHint(QPainter::Antialiasing)` 开启多级灰度抗锯齿，代价是绘制效率降低约 30%。对于静态控件（罗盘 100ms 刷新一次）完全可以接受。

---

### 知识点 2：柱状仪表盘实现（AltitudeGaugeWidget）

**设计要点**：
- 用 `qBound(min, val, max)` 钳制值到 `[0, 500]` 范围
- 填充比例 `ratio = (val - min) / (max - min)`
- **三色阶**：<20% 红色（危险）、20-50% 黄色（警告）、>50% 绿色（正常）
- 虚线参考线在 50% 位置，方便读取

```cpp
// 核心绘制代码
double ratio = (m_value - m_min) / (m_max - m_min);
int fillH = static_cast<int>(barH * ratio);
int fillY = barY + barH - fillH;  // 从底部向上画
p.drawRoundedRect(barX, fillY, barW, fillH, 2, 2);
```

**⚠️ 面试题：为什么 QWidget 的 `paintEvent` 里不能手动 delete painter？**
`QPainter` 以栈对象创建，构造时传入 `this`，析构时自动结束绘制，手动 `delete` 会导致双重析构。

---

### 知识点 3：AlarmWidget 过滤 + 等级着色

**过滤实现**（不删行，只隐藏）：
```cpp
m_table->setRowHidden(r, !visible);  // 性能更好，恢复快
```

**四档告警等级视觉区分**：

| 等级 | 文字 | 颜色 | 加粗 |
|------|------|------|------|
| Info | 提示 | `#3498db`（蓝） | ❌ |
| Warning | 警告 | `#f39c12`（橙） | ✅ |
| Critical | 严重 | `#e74c3c`（红） | ✅ |
| Emergency | 紧急 | 红字 + 粉红底色 | ✅ |

**`QOverload<int>::of` 解决信号歧义**：
```cpp
// QComboBox 有两个重载的 currentIndexChanged：
//   currentIndexChanged(int)
//   currentIndexChanged(const QString&)  // 已废弃
// 用 QOverload 明确指定取 int 版本
connect(m_filter, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &AlarmWidget::applyFilter);
```

**⚠️ 面试题：ComboBox 新旧语法对比**：
```cpp
// 旧语法（运行时检查，字符串不匹配无编译错误）
connect(m_filter, SIGNAL(currentIndexChanged(int)), this, SLOT(applyFilter(int)));

// 新语法（编译期检查，推荐）
connect(m_filter, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &AlarmWidget::applyFilter);
```

---

### 知识点 4：告警自动触发逻辑

在 `onTelemetryReceived` 中实时判断：

```cpp
if (drone.batteryPercent < 20) {
    // 构造 AlarmInfo，调用 m_alarmWidget->addAlarm(a)
}
if (drone.signalStr < 30) {
    // signalStr < 15 → Emergency，否则 Warning
}
```

**为什么不把告警逻辑放进 DroneManager？**
- 当前设计：DroneManager 只管数据管理，UI 层决定何时触发告警
- 关心分离（Separation of Concerns）：
  - DroneManager：数据存储 + 状态同步
  - MainWindow：策略层（决定"电量<20% 是什么级别告警"）
  - AlarmWidget：展示层（只管显示和过滤）
- 面试常问：**MVC 架构各层职责如何划分？**

---

### 当天总结

**Day 9 验收**：
- ✅ CompassWidget：QPainter 圆盘 + 箭头 + N/E/S/W 标注 + 航向角度数字
- ✅ AltitudeGaugeWidget：垂直柱状表 + 三色阶 + 虚线参考线
- ✅ AlarmWidget：5 列表格 + ComboBox 类型过滤 + 等级着色 + 告警触发
- ✅ 集成到 MainWindow：导航栏第 6 项"告警中心"→ QStackedWidget 第 6 页
- ✅ 自动告警：低电量 <30% / 信号弱 <30% 自动插告警行
- ✅ 编译：0 error, 0 warning, 0 crash
- ✅ 测试：53/53 算法测试 + 全数据库 CRUD 通过

**项目进度**：44/90 任务完成 (48.9%)

---

## Day 10 — 地图容器 + 高德地图 JS API

### 知识点 1：QWebEngineView 嵌入 Web 页面

**⚠️ 面试题：QWebEngineView vs QWebView（Qt WebKit）的区别**

| 对比 | QWebEngineView（Qt5.6+） | QWebView（Qt WebKit，已废弃） |
|------|--------------------------|-------------------------------|
| 内核 | Chromium | WebKit |
| Qt 支持 | 5.6+ | 5.0-5.5 |
| 性能 | 高（多进程架构） | 低（进程内渲染） |
| QWebChannel | ✅ 原生支持 | ❌ 兼容性差 |
| 内存占用 | 高（每个 view 一个子进程） | 低 |
| 主推 | ✅ Qt6 继续支持 | ❌ Qt6 已完全移除 |

**安装方式**（Ubuntu 20.04）：
```bash
sudo apt install qtwebengine5-dev libqt5webenginewidgets5
```

`.pro` 配置：
```qmake
QT += webenginewidgets webchannel
```

---

### 知识点 2：QWebChannel 双向通信

**架构示意**：
```
C++                          JS
───                          ──
QWebChannel::registerObject("bridge", bridge)
    │                            │
    │   QWebChannel 自动注入     │
    └────────────────────────→  qwebchannel.js
                                 │
                                 │ channel.objects.bridge 可用
                                 │
bridge.onMapReady() ──────→     bridge.onMapReady(lng,lat)
    │                            │
    └── emit mapReady() ───→    bridge.mapReady.connect(callback)
```

**⚠️ 面试题：QWebChannel 相比 eval 的优缺点**

| 方案 | 优点 | 缺点 |
|------|------|------|
| `page()->runJavaScript(js)` | 简单，不需要 QWebChannel | 单向，C++→JS，不能 JS→C++ |
| `QWebChannel` | **双向**，JS 可以调用 C++ 槽函数 | 需要额外的 qwebchannel.js |
| `QWebEnginePage::javaScriptConsoleMessage` | 调试 JS 错误 | 只读，不能交互 |

本项目：**双管齐下**：
- C++→JS：`runJavaScript` 推送位置数据
- JS→C++：`QWebChannel` → `JsBridge::onMapReady` 通知 C++ 地图就绪

**`QWebChannel` 的必要条件**：
1. C++ 端：`Q_OBJECT` 宏必须有
2. JS 端：`<script src="qrc:///qtwebchannel/qwebchannel.js"></script>` 必须引入
3. JS 端：必须先 `new QWebChannel(qt.webChannelTransport, callback)` 拿到 bridge 对象

---

### 知识点 3：Qt 资源系统（Resource System）

```qmake
RESOURCES += resources/resources.qrc
```

```xml
<!-- resources.qrc -->
<RCC>
    <qresource prefix="/">
        <file>html/map.html</file>
    </qresource>
</RCC>
```

**访问方式**：`QFile file(":/html/map.html")` 或者 JS 中用 `qrc:///qtwebchannel/qwebchannel.js`

**rcc 编译器**：
- `.qrc` 文件在编译时被 `rcc` 工具编译为 C++ 源码（`qrc_resources.cpp`）
- 数据嵌入到二进制文件中，无需外部文件依赖
- 缺点是修改 HTML 后需要重新编译整个程序

**⚠️ 面试题：Qt 资源文件 vs 外部文件的取舍**

| 方案 | 优点 | 缺点 |
|------|------|------|
| Qt 资源（本项目） | 部署简单，单二进制 | 修改 HTML 要重编译 |
| 外部文件 + `setUrl()` | 修改 HTML 不重编译 | 路径依赖，部署必须带 HTML |
| `setHtml(html, baseUrl)` | 灵活，可从数据库/网络加载 | 跨域限制（CORS） |

---

### 知识点 4：`runJavaScript` 的异步与调试

```cpp
// 无回调
m_view->page()->runJavaScript(js);

// 有回调（获取 JS 返回值）
m_view->page()->runJavaScript("1+1", [](const QVariant &v) {
    qDebug() << "JS result:" << v.toInt();  // 2
});
```

**调试 JS 错误**：
```cpp
// mainwindow.cpp 构造函数加：
m_mapWidget->page()->javaScriptConsoleMessage(
    [](QWebEnginePage::JavaScriptConsoleMessageLevel level,
       const QString &msg, int line, const QString &source) {
        qDebug() << "js:" << msg;
    });
```

本项目 JS 告警 `AMap is not defined` 的原因是：
1. `YOUR_KEY_HERE` 未替换为有效 Key → 高德 JS SDK 不加载
2. 即使有 Key，外网脚本需要网络访问
3. 加载异步，JS 在 AMap ready 前试图调用 `AMap.Map()`

---

### 知识点 5：Qt 中嵌入地图的选型对比

| 方案 | 难度 | 免费 | 离线可用 | 本项目 |
|------|------|------|----------|--------|
| **高德 JS API + QWebEngineView** | 中 | ✅ 每天 100 万次 | ❌ 需网络 | ✅ 采用 |
| 百度地图 JS API + QWebEngineView | 中 | ✅ 有限免费 | ❌ 需网络 | ❌ |
| Leaflet + OpenStreetMap | 中 | ✅ 完全免费 | ✅ 可预缓存瓦片 | ❌ |
| QGroundControl 方案（Qt Location） | 高 | ✅ | ✅ | ❌ |
| QCustomPlot / 自绘地图 | 高 | ✅ | ✅ | ❌ 不现实 |

---

### 当天总结

**Day 10 进度**（部分完成 - 待高德 Key）：
- ✅ 安装 qtwebengine5-dev
- ✅ MapWidget 容器（QWebEngineView + QWebChannel + JsBridge）
- ✅ map.html：AMap 地图初始化 + updateMarker / addTrailPoint JS 接口
- ✅ Qt 资源文件（resources.qrc + html/map.html）
- ✅ 集成 MainWindow：替换占位地图 + onTelemetryReceived 推送到地图
- ✅ 编译：0 error, 0 crash
- ⏳ 高德 Key：需要你替换 `YOUR_KEY_HERE` 为有效 Key
- ⏳ 地图验证：有 Key 后运行确认地图加载和无人机标记

**项目进度**：46/90 任务完成 (51.1%) — 待 Key 后可更新地图相关任务

---

## Day 11 — 地图图层 + 禁飞区可视化 + Bug 修复

### Bug 修复：地图晃动

**现象**：地图在 5 架无人机位置之间来回弹跳。

**原因**：`onTelemetryReceived` 每 100ms 收到 5 架无人机数据，每调用一次 `updateMarker` 都会执行 `map.setCenter([lng, lat])`，等于在 5 个坐标间快速切换。

**修复**：删除 `updateMarker` 中的 `map.setCenter()`，地图停留在用户拖拽的位置。

```javascript
// 修复前
markers[id].setPosition([lng, lat]);
markers[id].setAngle(heading);
map.setCenter([lng, lat]);   // ← 5架×10次/秒 = 50次setCenter

// 修复后
markers[id].setPosition([lng, lat]);
markers[id].setAngle(heading);
// setCenter 已删除
```

---

### 知识点 1：AMap 三种禁飞区形状

```javascript
// 圆形
new AMap.Circle({ center: [lng, lat], radius: 500 });

// 矩形
new AMap.Rectangle({ bounds: new AMap.Bounds([sw], [ne]) });

// 多边形
new AMap.Polygon({ path: [[lng,lat], [lng,lat], ...] });
```

**⚠️ 面试题：AMap.Bounds 参数顺序？**

`new AMap.Bounds(min, max)` 接受两个点，**第一个是西南角，第二个是东北角**，与经纬度顺序无关。高德内部自动判断大小。

**`fillOpacity: 0.2` 的设计理由**：
- 禁飞区遮住地图，完全不透明会遮挡底图信息
- 20% 透明度 = 红色底可见 + 地图道路依然可辨认
- 配合 `strokeColor: '#c0392b'` 实线边框 = 醒目但不遮挡

---

### 知识点 2：AMap.InfoWindow 点击弹窗

```javascript
var infoWindow = new AMap.InfoWindow({ offset: new AMap.Pixel(0, -30) });

marker.on('click', function(e) {
    infoWindow.setContent('<b>' + name + '</b><br>ID: ' + id);
    infoWindow.open(map, e.target.getPosition());
});
```

**⚠️ 面试题：闭包陷阱 — marker.on('click') 里的变量捕获**

```javascript
// ❌ 错误写法：循环结束后 name/id/lng/lat 都是最后一个值
for (var i = 0; i < drones.length; i++) {
    marker.on('click', function(e) {
        infoWindow.setContent(drones[i].name);  // i = length-1 后的值
    });
}

// ✅ 本项目正确写法：在 if(!markers[id]) 内部注册事件，每个 id 唯一的闭包
if (!markers[id]) {
    var m = new AMap.Marker(...);
    m.on('click', function(e) { ... });  // name/id 都来自当前函数参数
    markers[id] = m;
}
```

---

### 知识点 3：图层显隐控制

**三图层独立控制**：

```javascript
var layerVisibles = { markers: true, trails: true, fences: true };

function toggleLayer(name, visible) {
    layerVisibles[name] = visible;
    if (name === 'markers') {
        for (var id in markers)
            markers[id].setMap(visible ? map : null);
    }
    // 同理 trails / fences
}
```

**`setMap(null)` 和 `setMap(map)` 切换**：
- `setMap(null)` — 从地图移除，但不销毁对象
- `setMap(map)` — 重新添加到地图
- 比 `remove()` + 重建性能好得多

**图层控制 CSS**（右上角浮动面板）：
```css
#layerCtrl {
    position: absolute;
    top: 10px; right: 10px;
    background: white;
    box-shadow: 0 2px 6px rgba(0,0,0,0.2);
    padding: 8px 12px;
    z-index: 1000;
}
```

---

### 当天总结

**Day 11 验收**：
- ✅ 禁飞区图层：Circle / Rectangle / Polygon 三种形状绘制
- ✅ 禁飞区样式：半透明红底(20%) + 红边框 + 文字标签
- ✅ 图层控制：右上角浮动面板，显隐无人机/轨迹/禁飞区
- ✅ 点击弹窗：点击无人机标记弹出 InfoWindow 显示位置信息
- ✅ Bug 修复：删除 updateMarker 中的 setCenter，地图不再晃动
- ✅ MapWidget.h：加 view() 访问器，允许外部调用 runJavaScript
- ✅ 编译：0 error, 0 crash

**项目进度**：46/90 任务完成 (51.1%)

---