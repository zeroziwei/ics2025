

基于 `am.h`，说明AM的结构和各个模块的作用：

## 问题5：TRM、IOE、CTE等模块的作用是什么？

### 分类游戏：程序功能属于哪个模块？

| 程序需求 | 属于哪个模块？ | 使用的API |
|---------|--------------|----------|
| **打印 "Hello World"** | **IOE** 或 **TRM** | `putch()` (TRM) 或 `ioe_write(AM_UART_TX, ...)` (IOE) |
| **处理键盘输入** | **IOE** | `ioe_read(AM_INPUT_KEYBRD, ...)` |
| **分配内存** | **TRM** | `heap` (通过malloc/free使用) |
| **处理中断** | **CTE** | `cte_init()`, 中断处理函数 |
| **切换进程/线程** | **CTE** | `yield()`, `kcontext()` |
| **虚拟内存管理** | **VME** | `vme_init()`, `map()`, `protect()` |
| **多核通信** | **MPE** | `cpu_count()`, `atomic_xchg()` |
| **获取时间** | **IOE** | `ioe_read(AM_TIMER_UPTIME, ...)` |
| **显示图形** | **IOE** | `ioe_write(AM_GPU_FBDRAW, ...)` |
| **播放音频** | **IOE** | `ioe_write(AM_AUDIO_PLAY, ...)` |
| **程序结束** | **TRM** | `halt()` |

---

## AM定义的API（从 `am.h` 看）

### 1. TRM (Turing Machine) - 图灵机模块

**作用：** 最基本的运行时环境，提供计算能力

```c
// ----------------------- TRM: Turing Machine -----------------------
extern   Area        heap;        // 堆内存区域
void     putch       (char ch);   // 输出一个字符
void     halt        (int code);  // 结束程序运行
```

**特点：** 最基础，所有程序都需要

---

### 2. IOE (I/O Extension) - 输入输出扩展模块

**作用：** 提供输入输出设备访问

```c
// -------------------- IOE: Input/Output Devices --------------------
bool     ioe_init    (void);              // 初始化I/O设备
void     ioe_read    (int reg, void *buf); // 读设备寄存器
void     ioe_write   (int reg, void *buf); // 写设备寄存器
```

**支持的设备（从 `amdev.h` 看）：**
- **UART** (串口): `AM_UART_TX`, `AM_UART_RX`
- **TIMER** (定时器): `AM_TIMER_UPTIME`, `AM_TIMER_RTC`
- **INPUT** (输入): `AM_INPUT_KEYBRD` (键盘)
- **GPU** (图形): `AM_GPU_FBDRAW`, `AM_GPU_CONFIG`
- **AUDIO** (音频): `AM_AUDIO_PLAY`, `AM_AUDIO_CONFIG`
- **DISK** (磁盘): `AM_DISK_BLKIO`
- **NET** (网络): `AM_NET_TX`, `AM_NET_RX`

---

### 3. CTE (Context Extension) - 上下文扩展模块

**作用：** 中断处理和上下文切换

```c
// ---------- CTE: Interrupt Handling and Context Switching ----------
bool     cte_init    (Context *(*handler)(Event ev, Context *ctx));
                                          // 初始化中断处理
void     yield       (void);             // 主动让出CPU
bool     ienabled    (void);             // 查询中断是否开启
void     iset        (bool enable);      // 开启/关闭中断
Context *kcontext    (Area kstack, void (*entry)(void *), void *arg);
                                          // 创建内核上下文
```

**支持的Event类型：**
- `EVENT_YIELD` - 主动让出
- `EVENT_SYSCALL` - 系统调用
- `EVENT_PAGEFAULT` - 页错误
- `EVENT_IRQ_TIMER` - 定时器中断
- `EVENT_IRQ_IODEV` - I/O设备中断

---

### 4. VME (Virtual Memory Extension) - 虚存扩展模块

**作用：** 虚拟内存管理

```c
// ----------------------- VME: Virtual Memory -----------------------
bool     vme_init    (void *(*pgalloc)(int), void (*pgfree)(void *));
                                          // 初始化虚存系统
void     protect     (AddrSpace *as);    // 保护地址空间
void     unprotect   (AddrSpace *as);    // 取消保护
void     map         (AddrSpace *as, void *vaddr, void *paddr, int prot);
                                          // 映射虚拟页到物理页
Context *ucontext    (AddrSpace *as, Area kstack, void *entry);
                                          // 创建用户上下文
```

---

### 5. MPE (Multi-Processor Extension) - 多处理器扩展模块

**作用：** 多核/多处理器支持

```c
// ---------------------- MPE: Multi-Processing ----------------------
bool     mpe_init    (void (*entry)());  // 初始化多处理器
int      cpu_count   (void);             // 获取CPU数量
int      cpu_current (void);             // 获取当前CPU编号
int      atomic_xchg (int *addr, int newval); // 原子交换操作
```

---

## 为什么AM要分成这些模块？

### 原因1：按需使用（模块化设计）

**不模块化的问题：**
```c
// 如果所有API都在一起
// 一个简单程序（只需要打印）也要链接所有代码
// 包括：虚拟内存、多处理器、音频、网络等
// 浪费资源，增加复杂性
```

**模块化的好处：**
```c
// 简单程序（dummy.c）只需要TRM
AM = TRM

// 需要I/O的程序
AM = TRM + IOE

// 需要多任务的程序（OS）
AM = TRM + IOE + CTE

// 完整的系统
AM = TRM + IOE + CTE + VME + MPE
```

程序只链接需要的模块，降低代码大小和复杂度。

---

### 原因2：功能分类清晰

每个模块职责单一：

| 模块 | 职责 | 类比 |
|------|------|------|
| **TRM** | 基本计算能力 | 就像计算机的基本功能 |
| **IOE** | 输入输出 | 就像键盘、鼠标、显示器 |
| **CTE** | 中断和切换 | 就像操作系统的调度功能 |
| **VME** | 内存管理 | 就像操作系统的内存保护 |
| **MPE** | 多核支持 | 就像多核处理器的协调 |

---

### 原因3：渐进式学习

PA的设计理念：

```
PA2第一阶段: TRM          ← 最简单的程序
PA2第二阶段: TRM + IOE    ← 需要I/O的程序
PA3:         TRM + IOE + CTE  ← 需要多任务的程序
PA4:         TRM + IOE + CTE + VME  ← 完整的操作系统
```

逐步添加模块，逐步理解。

---

### 原因4：实现解耦

不同模块可以独立实现和测试：

```c
// TRM的实现
abstract-machine/am/src/platform/nemu/trm.c

// IOE的实现
abstract-machine/am/src/platform/nemu/ioe/

// CTE的实现
abstract-machine/am/src/riscv/nemu/cte.c
abstract-machine/am/src/x86/nemu/cte.c  // 不同ISA实现不同

// VME的实现
abstract-machine/am/src/riscv/nemu/vme.c
```

每个模块相对独立，修改一个模块不影响其他模块。

---

### 原因5：匹配硬件发展历史

模块的顺序反映了计算机系统的发展：

```
1. TRM  - 最初的计算机（只能计算）
2. IOE  - 添加输入输出设备
3. CTE  - 添加中断和多任务
4. VME  - 添加内存保护
5. MPE  - 添加多核支持
```

---

## 总结：AM模块化的优势

| 优势 | 说明 |
|------|------|
| **按需使用** | 程序只使用需要的模块 |
| **功能清晰** | 每个模块职责明确 |
| **渐进学习** | 可以逐步添加模块 |
| **易于维护** | 模块之间解耦，易于修改 |
| **匹配硬件** | 反映计算机系统的发展 |

---

## 思考：如果所有API放在一起会怎样？

**问题：**
1. 简单程序也要链接所有代码 → 浪费资源
2. 所有功能混在一起 → 难以理解
3. 无法渐进学习 → 必须一次性理解所有内容
4. 修改困难 → 改动可能影响所有功能
5. 不符合单一职责原则 → 违反软件设计原则

**结论：**
模块化设计让AM更清晰、更灵活、更易维护，也更适合教学。

这就是为什么AM要分成这些模块的原因。