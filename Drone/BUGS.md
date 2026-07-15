# Bug 记录

> 记录开发过程中发现的 Bug 及修复情况

| 编号 | 日期 | 模块 | Bug 描述 | 严重程度 | 状态 | 修复方案 |
|------|------|------|----------|----------|------|----------|
| #001 | 2026-07-13 | mainwindow | ZeroMQ recv 阻塞导致程序卡死 | 高 | ✅ 已修复 | 调整 send/recv 顺序，加 usleep |
| #002 | 2026-07-13 | mainwindow | QSqlError 编译报错 | 中 | ✅ 已修复 | 添加 #include \<QSqlError\> |
| #003 | 2026-07-15 | MavlinkParser | 头文件保护写成 `#include` 导致编译失败 | 高 | ✅ 已修复 | 改为 `#ifndef` |
| #004 | 2026-07-15 | SimulatedLink | 头文件 `stopSimulaton` 拼写错误导致链接失败 | 高 | ✅ 已修复 | 改为 `stopSimulation` |
| #005 | 2026-07-15 | MavlinkParser | `encodeGlobalPosition` 函数体为空导致编译错误 | 高 | ✅ 已修复 | 实现完整的编码逻辑 |
| #006 | 2026-07-15 | MavlinkParser | `reinterpret_cast` 语法错误 | 高 | ✅ 已修复 | 修正 `const char*>` 括号位置 |

---

**严重程度说明**：
- **高**：崩溃/数据丢失/功能完全不可用
- **中**：功能异常但有 workaround
- **低**：界面/体验问题

**状态说明**：
- 🔴 未修复
- 🟡 修复中
- ✅ 已修复
- ❌ 不修复（wontfix）
