下面把 “NEMU 运行程序” 分成两步：先生成要跑的 AM 二进制，再用 NEMU 载入运行。用 `string` 测试为例（riscv32-nemu）：

## 前置
- 环境变量：`AM_HOME` 指向 `abstract-machine`，`NEMU_HOME` 指向 `nemu`。
- 你选的架构要一致，例如 `ARCH=riscv32-nemu`。

## Step 1：编译生成待运行的测试程序
在 `am-kernels/tests/cpu-tests` 目录（如果还没克隆 am-kernels，需要先 clone）：
```bash
cd $AM_HOME/../am-kernels/tests/cpu-tests
make ARCH=riscv32-nemu string   # 生成 string 测试的 elf/bin
```
生成物通常在：`am-kernels/tests/cpu-tests/build/string-riscv32-nemu.bin`

## Step 2：用 NEMU 运行
在 NEMU 根目录：
```bash
cd $NEMU_HOME
./build/riscv32-nemu-interpreter -b $AM_HOME/../am-kernels/tests/cpu-tests/build/string-riscv32-nemu.bin
```
解释：
- `./build/riscv32-nemu-interpreter` 是 NEMU 的可执行文件（对应 menuconfig/ARCH 编译产物）。
- `-b` 启用批处理模式，启动后直接运行，不需要在 NEMU 里手动输入 `c`。
- 后面的路径是刚才生成的 `.bin`。

### 如果没有批处理模式
不加 `-b` 启动后，会进入 NEMU 命令行，需要键入 `c` 继续执行。

### 常见坑
- 你的 NEMU 架构配置（`CONFIG_ISA`）要和 `ARCH` 一致，否则加载会失败。
- 确认 `am-kernels` 路径正确；如果不在同级目录，调整上面命令中的路径。
- 先确保 klib 实现补全并能通过编译，否则在 Step 1 就会报错。

需要的话我可以帮你确认 am-kernels 是否已存在，或你当前 NEMU 的 ARCH 配置。