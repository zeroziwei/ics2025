

## AM是什么？多层面的理解

### 1. **概念层面：AM是一组API（接口规范）**

文章中的定义：
> "由于这组统一抽象的API代表了程序运行对计算机的需求, 所以我们把这组API称为抽象计算机."

AM是接口规范，定义程序需要哪些功能，但不关心具体如何实现。

```c
// am.h - 这就是AM！
// AM定义了一组函数签名（接口）

void halt(int code);           // 程序需要能结束
void putch(char ch);           // 程序需要能输出
Area heap;                     // 程序需要堆内存
// ... 等等
```

### 2. **实现层面：AM是一个库（Library）**

AM不只是接口，也有具体实现。每个架构都有自己的实现：

```
AM实现的结构：
abstract-machine/am/src/
├── riscv/nemu/        ← RISC-V在NEMU上的AM实现
├── x86/nemu/          ← x86在NEMU上的AM实现
├── mips32/nemu/       ← MIPS在NEMU上的AM实现
└── platform/nemu/     ← NEMU平台通用的AM实现
```

### 3. **功能层面：AM是运行时环境（Runtime Environment）**

AM为程序提供运行所需的基础功能：

```
AM = TRM + IOE + CTE + VME + MPE
```

每个模块提供不同的功能：

```c
// TRM: 基本计算能力
void halt(int code);
void putch(char ch);
Area heap;

// IOE: 输入输出
void ioe_read(int reg, void *buf);
void ioe_write(int reg, void *buf);

// CTE: 中断和上下文管理
bool cte_init(Context *(*handler)(Event ev, Context *ctx));
void yield(void);

// VME: 虚拟内存
void protect(AddrSpace *as);
void map(AddrSpace *as, void *vaddr, void *paddr, int prot);

// MPE: 多处理器
int cpu_count(void);
int cpu_current(void);
```

## 用类比理解AM

### 类比1：AM像“操作系统API”

| 真实世界 | AM世界 |
|---------|--------|
| 操作系统（Linux/Windows） | AM |
| 系统调用（syscall） | AM API |
| 应用程序调用系统调用 | 程序调用AM API |
| 不同OS提供不同实现 | 不同架构提供不同AM实现 |

区别：
- 操作系统：完整的系统软件
- AM：最小化的运行时环境库（Bare-metal Runtime）

### 类比2：AM像“硬件抽象层（HAL）”

```
应用程序
    ↓ (调用)
AM API (统一接口)
    ↓ (实现)
硬件 (x86/RISC-V/MIPS)
```

AM隐藏了硬件差异，为程序提供统一接口。

## AM在整个系统中的位置

```
┌─────────────────────────────────────────┐
│          应用程序 (Application)         │
│  (你的程序，比如dummy.c, hello等)        │
└──────────────┬──────────────────────────┘
               │ 调用AM API
┌──────────────▼──────────────────────────┐
│         AM (Abstract Machine)           │
│  ┌──────┬──────┬──────┬──────┐         │
│  │ TRM  │ IOE  │ CTE  │ VME  │         │
│  └──────┴──────┴──────┴──────┘         │
│  运行时环境库 (Runtime Library)          │
└──────────────┬──────────────────────────┘
               │ 使用硬件功能
┌──────────────▼──────────────────────────┐
│        硬件/模拟器 (NEMU)                │
│  CPU + 内存 + I/O设备                     │
└─────────────────────────────────────────┘
```

## AM的本质：三个身份

### 身份1：接口规范（Interface Specification）
定义“程序需要什么功能”

```c
// am.h 定义了所有接口
void halt(int code);  // 规范：程序需要能结束
```

### 身份2：库实现（Library Implementation）
提供“如何实现这些功能”

```c
// trm.c 实现了halt函数
void halt(int code) {
  nemu_trap(code);  // 具体实现：通过nemu_trap指令
}
```

### 身份3：抽象层（Abstraction Layer）
隐藏“硬件差异”

```c
// 程序调用
halt(0);

// x86上实现为：
asm volatile("int $0x80");  // x86的中断指令

// RISC-V上实现为：
asm volatile("ebreak");     // RISC-V的断点指令

// 但程序不需要知道这些差异！
```

## 关键点总结

1. AM = 接口 + 实现
   - 接口：定义程序需要什么（am.h）
   - 实现：提供具体功能（各个架构的实现）

2. AM = 运行时环境
   - 不是操作系统，而是最小化的运行时库
   - 运行在“裸机”（bare-metal）上

3. AM = 抽象层
   - 将程序与硬件解耦
   - 同一程序可在不同架构上运行

4. AM = 模块化设计
   ```
   AM = TRM + IOE + CTE + VME + MPE
   ```
   程序可以只使用需要的模块

## 对比：AM vs 其他概念

| 概念 | 是什么 | 和AM的关系 |
|------|--------|-----------|
| **操作系统** | 完整的系统软件 | AM比OS更简单，运行在裸机上 |
| **运行时环境** | 程序运行需要的环境 | AM就是运行时环境 |
| **库函数** | 可调用的函数集合 | AM是一个特殊的库（系统级库）|
| **硬件抽象层** | 隐藏硬件差异的层 | AM就是硬件抽象层 |
| **API** | 应用程序接口 | AM定义了一组API |

## 思考题

1. 为什么需要AM？如果不用AM会怎样？
   - 需要：程序要为每个架构写不同代码
   - 有了AM：程序写一次，可在多个架构运行

2. AM和操作系统的区别？
   - AM：最小化的运行时环境
   - OS：完整的系统软件（进程管理、文件系统等）

3. AM是硬件还是软件？
   - 软件，运行在硬件（或NEMU模拟器）之上

简单总结：AM是一套接口规范 + 实现库，为程序提供统一的运行时环境，隐藏硬件差异，让程序可以在不同架构上运行。

需要我进一步解释某个具体方面吗？