查看链接脚本和编译配置，以解答问题7。


[3 tools called]


查看一个具体的架构配置文件以了解编译选项：


[3 tools called]


查看一个 start.S 文件，了解入口点的实现：


[1 tool called]


以下是问题7的答案：

## 问题7：为什么不能直接用gcc默认选项编译？

### 1. Linux程序 vs NEMU环境的依赖对比

**Linux下的程序依赖：**
- 操作系统：Linux内核提供系统调用接口
- C运行时库（glibc/musl）：提供 `_start`, `exit()`, `malloc()` 等
- 动态链接器：加载和链接共享库
- 标准库：依赖系统提供的库函数
- 程序入口：通常从 `_start` 开始，由C运行时库初始化

**NEMU能提供什么：**
- ❌ 无操作系统：没有Linux内核，没有系统调用
- ❌ 无C运行时库：没有 `exit()`, 没有 `malloc()` 等标准库函数
- ❌ 无动态链接器：不支持动态链接
- ✅ AM运行时：提供 `halt()`, `putch()` 等抽象接口
- ✅ 链接脚本：自定义内存布局和入口点

### 2. 链接脚本 `linker.ld` 的作用

查看 `abstract-machine/scripts/linker.ld`：

```ld
ENTRY(_start)              /* 定义程序入口点为 _start */
PHDRS { text PT_LOAD; data PT_LOAD; }

SECTIONS {
  /* 从特定地址开始（_pmem_start + _entry_offset） */
  . = _pmem_start + _entry_offset;  /* 例如 0x80000000 */
  
  .text : {
    *(entry)               /* 先放 entry 段（start.S）*/
    *(.text*)              /* 然后其他代码 */
  } : text
  
  .rodata : { *(.rodata*) }
  .data : { *(.data*) } : data
  .bss : { 
    _bss_start = .;
    *(.bss*)
  }
  
  _stack_top = ALIGN(0x1000);        /* 栈顶位置 */
  . = _stack_top + 0x8000;           /* 栈大小 32KB */
  _stack_pointer = .;                /* 栈指针 */
  
  _heap_start = ALIGN(0x1000);       /* 堆起始位置 */
}
```

**链接脚本的关键作用：**
1. 定义入口点：`ENTRY(_start)`，确保从 `start.S` 开始执行
2. 设置内存布局：代码、数据、栈、堆的位置
3. 定义符号：`_stack_pointer`, `_heap_start` 等，供代码使用
4. 控制链接顺序：先链接 `entry` 段（`start.S`），保证入口正确

### 3. AM的编译选项（vs gcc默认选项）

对比AM的编译选项（来自 `abstract-machine/Makefile`）：

**AM的关键编译选项：**
```makefile
# 禁止某些默认行为
-fno-asynchronous-unwind-tables  # 不要异步展开表
-fno-builtin                      # 不要使用内置函数
-fno-stack-protector              # 不要栈保护（NEMU不需要）
-Wno-main                         # main函数可以是其他形式
-U_FORTIFY_SOURCE                 # 禁用安全强化

# 链接选项
-z noexecstack                    # 栈不可执行
-T linker.ld                      # 使用自定义链接脚本
--defsym=_pmem_start=0x80000000   # 定义内存起始地址
-e _start                         # 入口点为 _start
```

**如果用gcc默认选项会发生什么？**

❌ 问题1：默认会链接C运行时库
```bash
gcc main.c  # 默认会链接 libc，但NEMU没有libc
```
- 会尝试链接 `libc.so`，导致链接失败或运行时错误

❌ 问题2：默认的入口点不对
```bash
gcc main.c  # 默认入口是 _start (来自libc)
```
- Linux的 `_start` 会调用系统调用，但NEMU没有操作系统

❌ 问题3：默认的内存布局不对
- 默认布局假设有操作系统管理内存
- NEMU需要从固定地址（如 `0x80000000`）开始

❌ 问题4：会使用内置函数
```c
memcpy();  // 默认可能用gcc内置优化版本
```
- 内置函数可能使用特殊指令，在NEMU中不支持

### 4. 完整的编译流程对比

**普通Linux程序：**
```bash
gcc main.c -o main
# 默认：链接libc，使用系统链接脚本，入口点_start，动态链接
```

**NEMU程序（AM方式）：**
```bash
# 1. 编译（禁用默认行为）
riscv64-linux-gnu-gcc -fno-builtin -fno-stack-protector -c main.c

# 2. 链接（使用自定义脚本和选项）
riscv64-linux-gnu-ld \
  -T linker.ld \              # 自定义链接脚本
  -e _start \                 # 入口点
  --defsym=_pmem_start=0x80000000 \  # 内存地址
  start.o main.o am.a klib.a  # 静态链接，不链接libc
```

### 总结对比表

| 特性 | Linux程序（gcc默认） | NEMU程序（AM） |
|------|---------------------|----------------|
| **运行时库** | glibc（动态链接） | AM + klib（静态链接） |
| **入口点** | `_start`（libc提供） | `_start`（start.S提供） |
| **链接脚本** | 系统默认（/usr/lib/ldscripts） | 自定义（linker.ld） |
| **内存布局** | 操作系统管理 | 固定地址（0x80000000） |
| **系统调用** | 有（通过内核） | 无（直接硬件操作） |
| **初始化** | libc的_start初始化 | start.S → _trm_init → main |

**核心原因：**
NEMU是裸机环境（bare metal），没有操作系统和标准C库。必须用自定义链接脚本、静态链接，并禁用gcc默认行为，让程序直接运行在硬件抽象层（AM）上。

**实践建议：**
1. 尝试用普通gcc编译一个简单程序，观察链接错误
2. 对比默认链接脚本和 `linker.ld` 的差异
3. 用 `readelf -l` 查看两种程序的内存布局差异
4. 用 `objdump -d` 查看入口点代码的差异

这样可以更直观地理解为什么需要这些特殊配置。