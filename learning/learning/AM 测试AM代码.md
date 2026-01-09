查看当前代码和测试相关文件，了解如何运行测试。


[3 tools called]


查看项目结构，查找测试相关文件：


[2 tools called]


查看 Makefile 和项目根目录，了解测试运行方式：


[2 tools called]


查看 NEMU 的 Makefile，了解如何运行测试：


[2 tools called]


根据文档和项目结构，测试库函数的步骤如下：

## 测试流程

### 1. 确保环境变量设置正确

首先确保设置了必要的环境变量（通常在 `init.sh` 或 `.bashrc` 中）：

```bash
export AM_HOME=/path/to/abstract-machine
export NEMU_HOME=/path/to/nemu
```

### 2. 编译 klib 库

在 `abstract-machine/klib` 目录下编译库：

```bash
cd abstract-machine/klib
make ARCH=riscv32-nemu  # 根据你的 ISA 选择，比如 riscv32-nemu, x86-nemu, mips32-nemu
```

这会生成 `build/klib-riscv32-nemu.a`（或对应架构的库文件）。

### 3. 运行测试程序

根据文档，测试用例在 `am-kernels` 子项目中。你需要：

#### 方法 A：如果有 am-kernels 项目

```bash
# 进入 am-kernels 目录
cd am-kernels/tests/cpu-tests

# 编译并运行 string 测试
make ARCH=riscv32-nemu run STRING=on

# 或者编译并运行 hello-str 测试（需要 sprintf）
make ARCH=riscv32-nemu run HELLO_STR=on
```

#### 方法 B：直接使用 NEMU 运行编译好的程序

```bash
# 1. 在 am-kernels/tests/cpu-tests 目录下编译测试程序
cd am-kernels/tests/cpu-tests
make ARCH=riscv32-nemu

# 2. 在 NEMU 中运行（假设你已经实现了批处理模式）
cd ../../../nemu
./build/riscv32-nemu-interpreter -b abstract-machine/am-kernels/tests/cpu-tests/build/string-riscv32-nemu.bin
```

### 4. 测试 string 函数

测试 `string` 用例会调用你实现的字符串函数。如果实现正确，程序会正常结束；如果有问题，可能会：
- 触发 `panic()` 导致程序异常终止
- 输出错误信息
- NEMU 报告程序异常退出

### 5. 测试 sprintf（hello-str）

测试 `hello-str` 需要实现 `sprintf`：

```bash
# 在 am-kernels/tests/cpu-tests 目录下
make ARCH=riscv32-nemu run  # 运行 hello-str 测试
```

### 6. 调试技巧

如果测试失败，可以：

1. 查看测试源码：查看 `am-kernels/tests/cpu-tests/tests/string.c` 了解测试逻辑
2. 使用 NEMU 的调试功能：
   ```bash
   # 在 NEMU 中单步调试
   (nemu) si
   (nemu) p $pc
   ```
3. 检查实现：确保函数行为符合 C 标准（参考 `man` 手册）

### 7. 快速验证单个函数

你也可以写一个简单的测试程序来验证单个函数：

```c
// test_string.c
#include <klib.h>

int main() {
    char buf[100];
    strcpy(buf, "hello");
    assert(strlen(buf) == 5);
    assert(strcmp(buf, "hello") == 0);
    return 0;
}
```

然后编译运行：

```bash
# 在 abstract-machine 目录下创建测试项目
# 设置 NAME=test_string, SRCS=test_string.c
# 然后 make ARCH=riscv32-nemu run
```

### 注意事项

1. 确保 `ARCH` 与你的 NEMU 配置一致（如 `riscv32-nemu`, `x86-nemu` 等）
2. 如果 `am-kernels` 不在当前目录，需要先克隆：
   ```bash
   git clone https://github.com/NJU-ProjectN/am-kernels.git
   ```
3. 批处理模式：文档提到需要实现批处理模式，让 NEMU 启动后直接运行程序，避免每次手动输入 `c`

需要我帮你检查 `am-kernels` 是否存在，或查看具体的测试用例代码吗？