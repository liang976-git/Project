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
- **面试相关**：⚠️ **阻塞 vs 非阻塞 IO**、**发布订阅模式 vs 观察者模式**

---

## Day 2 — 核心数据结构与模型层

### 知识点 1：explicit 关键字
- **用法**：`explicit DroneManager(QObject *parent = nullptr);`
- **作用**：禁止编译器使用该构造函数进行隐式类型转换
- **不加 explicit 时**：`DroneManager dm = nullptr;` 编译器会自动把 `nullptr` 隐式转为 `DroneManager` 对象
- **加了 explicit 后**：只能显式调用 `DroneManager dm(nullptr);`
- **面试相关**：⚠️ **C++ 面试高频题**，常见于"谈谈 C++11 新特性"、"构造函数相关问题"

### 知识点 2：Qt 信号槽机制（面试必问）
- **基本概念**：对象间通信机制，替代回调函数
- **核心语法**：
  ```cpp
  // 连接信号和槽
  connect(sender, &Sender::signal, receiver, &Receiver::slot);
  ```
- **emit 的作用**：触发信号，本身不做任何逻辑，只是一个标记
- **信号可以连接多个槽**：一对多关系
- **槽可以被多个信号连接**：多对一关系
- **连接方式**：
  - **直连（DirectConnection）**：同线程，同步调用（默认）
  - **队列连接（QueuedConnection）**：跨线程，异步调用
  - **自动连接（AutoConnection）**：Qt 自动判断（默认）
- **面试相关**：⚠️ **Qt 面试第一问**，必须掌握

### 知识点 3：QMap 容器
- **底层实现**：红黑树（平衡二叉搜索树）
- **查找/插入/删除时间复杂度**：O(log n)
- **与 QHash 对比**：QMap 有序，QHash 无序但平均 O(1)
- **面试相关**：⚠️ **C++ STL 容器对比**，map vs hash_map vs unordered_map

### 知识点 4：MVC/信号槽 vs 观察者模式
- **观察者模式**：设计模式，一对多依赖关系
- **信号槽**：Qt 的观察者模式实现，但更灵活（松耦合、可跨线程）
- **发布订阅**：ZeroMQ 的 PUB-SUB，类似但更松散（无直接引用）
- **面试相关**：⚠️ **设计模式对比题**

---

## 待记录

_后续开发中遇到的问题在此追加_

