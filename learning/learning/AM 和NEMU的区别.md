

从代码和系统架构角度说明 AM 和 NEMU 的区别：

## AM 和 NEMU 的区别

### 核心区别：角色不同

```
┌─────────────────────────────────┐
│       应用程序 (Application)     │
│      (你的程序，如dummy.c)        │
└──────────────┬──────────────────┘
               │ 调用AM API
┌──────────────▼──────────────────┐
│     AM (Abstract Machine)       │  ← 运行时环境库（软件）
│     运行时环境 / 软件层           │
│     - halt(), putch()           │
│     - 提供统一API接口            │
└──────────────┬──────────────────┘
               │ 使用硬件功能
┌──────────────▼──────────────────┐
│     NEMU (NJU Emulator)         │  ← 硬件模拟器
│     CPU模拟器 / 硬件层            │
│     - cpu_exec()                │
│     - pmem[] (物理内存)          │
│     - 设备模拟                   │
└─────────────────────────────────┘
```

---

## 详细对比

### 1. **本质区别**

| 方面 | NEMU | AM |
|------|------|-----|
| **身份** | 硬件模拟器 (Emulator) | 运行时环境库 (Runtime Library) |
| **作用** | 模拟CPU和硬件 | 为程序提供运行时环境 |
| **层级** | 底层（硬件层） | 中间层（软件层） |
| **类比** | 就像真实的CPU和内存 | 就像操作系统的基础库 |

### 2. **提供的功能**

**NEMU提供（硬件功能）：**
```c
// NEMU提供的硬件功能
- CPU执行指令 (cpu_exec())
- 物理内存 (pmem[])
- 设备模拟 (串口、键盘、VGA等)
- 指令执行 (取指、译码、执行)
```

**AM提供（软件接口）：**
```c
// AM提供的API
- halt(int code)      // 程序结束
- putch(char ch)      // 输出字符
- heap                // 堆内存
- ioe_read/write()    // I/O操作
- cte_init()          // 中断处理
```

### 3. **代码位置**

**NEMU的代码：**
```
nemu/
├── src/
│   ├── cpu/cpu-exec.c    ← CPU执行指令
│   ├── memory/paddr.c    ← 物理内存管理
│   └── device/           ← 设备模拟
```

**AM的代码：**
```
abstract-machine/am/
├── src/
│   ├── platform/nemu/
│   │   └── trm.c         ← AM在NEMU上的实现
│   └── $ISA/nemu/
│       └── start.S       ← 程序入口
```

---

## 关键理解：它们如何协作

### 例子1：程序结束 (`halt()`)

**程序的视角（调用AM）：**
```c
// 你的程序
halt(0);  // 调用AM的API
```

**AM的实现（调用NEMU）：**
```c
// abstract-machine/am/src/platform/nemu/trm.c
void halt(int code) {
  nemu_trap(code);  // 调用NEMU的特殊指令
}
```

**NEMU的处理（硬件层）：**
```c
// NEMU识别nemu_trap指令，设置状态
nemu_state.state = NEMU_END;
nemu_state.halt_ret = code;
// 从cpu_exec()循环中退出
```

执行流程：
```
程序 → AM API → NEMU硬件功能
```

### 例子2：输出字符 (`putch()`)

**程序的视角：**
```c
putch('A');  // 调用AM API
```

**AM的实现：**
```c
// abstract-machine/am/src/platform/nemu/trm.c
void putch(char ch) {
  outb(SERIAL_PORT, ch);  // 通过I/O指令写入串口
}
```

**NEMU的处理：**
```c
// NEMU的I/O处理
// 识别I/O指令 → 调用serial_io_handler() → 输出到终端
```

---

## 从执行流程看区别

### NEMU的执行流程（硬件视角）：
```c
// nemu/src/cpu/cpu-exec.c
void cpu_exec(uint64_t n) {
  execute(n);  // 执行n条指令
}

static void execute(uint64_t n) {
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);  // 执行一条指令
    // 取指 → 译码 → 执行 → 更新PC
  }
}
```

NEMU关注：如何执行指令

### AM的执行流程（软件视角）：
```c
// abstract-machine/am/src/platform/nemu/trm.c
void _trm_init() {
  int ret = main(mainargs);  // 调用用户程序
  halt(ret);                  // 程序结束
}
```

AM关注：如何为程序提供运行环境

---

## 类比理解

### 类比1：建筑工地

| 角色 | 对应 |
|------|------|
| **NEMU** | 建筑材料（钢筋、水泥、砖块） |
| **AM** | 建筑工具和规范（脚手架、施工标准） |
| **应用程序** | 建筑工人（使用工具，按照规范工作） |

### 类比2：汽车系统

| 角色 | 对应 |
|------|------|
| **NEMU** | 引擎、车轮、底盘（硬件） |
| **AM** | 方向盘、油门、刹车（接口） |
| **应用程序** | 司机（使用接口驾驶） |

---

## 关键表格总结

文章中的表格很好地说明了它们的关系：

| 程序需求 | AM API | NEMU硬件功能 |
|---------|--------|-------------|
| 计算 | - | `cpu_exec()` (执行指令) |
| 内存申请 | `heap` | `pmem[]` (物理内存数组) |
| 结束运行 | `halt()` | `nemu_trap`指令 + `nemu_state` |
| 打印信息 | `putch()` | I/O指令 + `serial_io_handler()` |

---

## 为什么需要分开？

### 1. **职责分离**
- NEMU：专注于硬件模拟（如何执行指令）
- AM：专注于运行时环境（如何支撑程序运行）

### 2. **可移植性**
- 程序只调用AM API，不关心底层是NEMU还是真实硬件
- AM可以在不同硬件上实现（NEMU、QEMU、真实CPU等）

### 3. **教学清晰**
```
硬件功能 (NEMU) → 运行时环境 (AM) → 应用程序
```
层次分明，便于理解

---

## 总结

| 问题 | NEMU | AM |
|------|------|-----|
| **是什么？** | 硬件模拟器 | 运行时环境库 |
| **做什么？** | 模拟CPU、内存、设备 | 为程序提供API接口 |
| **在哪里？** | `nemu/` 目录 | `abstract-machine/am/` 目录 |
| **如何工作？** | 执行指令，管理硬件资源 | 调用NEMU功能，提供统一接口 |

**简单记忆：**
- **NEMU = 虚拟的CPU和硬件**
- **AM = 运行在硬件上的软件库**

它们的关系就像：
- **NEMU = 真实的计算机硬件**
- **AM = 操作系统的基础库**

需要我进一步解释它们如何协作吗？