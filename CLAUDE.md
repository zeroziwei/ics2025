# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the ICS2025 (Introduction to Computer Systems) programming assignment from Nanjing University. The project consists of multiple subprojects that work together to build a complete computer system from the ground up.

**Main Components:**
- **NEMU** (NJU Emulator): A full-system emulator supporting x86, mips32, riscv32, and riscv64 architectures
- **Abstract Machine** (AM): A minimal hardware abstraction layer providing a unified interface across different platforms
- **Learning**: Personal notes and documentation (not part of core project)

## Environment Setup

The project requires environment variables to be set:
- `NEMU_HOME`: Path to the nemu directory
- `AM_HOME`: Path to the abstract-machine directory

Initialize subprojects using:
```bash
bash init.sh nemu              # Initialize NEMU
bash init.sh abstract-machine  # Initialize Abstract Machine
```

## Building and Running NEMU

### Configuration
NEMU uses Kconfig for configuration. Before building, you must configure it:

```bash
cd nemu
make menuconfig                           # Interactive configuration
# OR use a predefined config:
make riscv32-am_defconfig                # RISC-V 32-bit
make riscv64-am_defconfig                # RISC-V 64-bit
make mips32-am_defconfig                 # MIPS32
```

Configuration options include:
- Base ISA (x86, mips32, riscv32/64, loongarch32r)
- Execution engine (interpreter)
- Memory mode (system/user)
- Device support (serial, timer, keyboard, VGA, audio)
- Differential testing with reference implementations

### Building
```bash
cd nemu
make                                     # Build NEMU
```

The binary is created at `nemu/build/<ISA>-nemu-<engine>`.

### Running
```bash
cd nemu
make run                                 # Run with default settings
make run IMG=/path/to/image              # Run specific program image
make gdb IMG=/path/to/image              # Debug with GDB
```

Default arguments include `--log=build/nemu-log.txt` for execution logging.

### Cleaning
```bash
cd nemu
make clean                               # Clean build artifacts
make distclean                           # Also remove configuration
make clean-all                           # Clean everything including tools
```

## Building Programs on Abstract Machine

Abstract Machine provides a platform-independent API for building bare-metal programs.

### Build Commands
```bash
cd <program-directory>
make ARCH=<arch>                         # Build for specific architecture
make ARCH=<arch> run                     # Build and run
```

**Available architectures:**
- `native`: Native Linux/macOS application
- `riscv32-nemu`, `riscv64-nemu`: RISC-V on NEMU
- `mips32-nemu`: MIPS32 on NEMU
- `x86-nemu`: x86 on NEMU
- `x86-qemu`, `x86_64-qemu`: x86/x86-64 on QEMU

### Example Workflow
```bash
# Build a program for RISC-V 32-bit NEMU
cd abstract-machine/klib
make ARCH=riscv32-nemu

# Run the program on NEMU
cd nemu
make run IMG=/path/to/program-image
```

## NEMU Debugger (SDB)

NEMU includes a simple debugger with the following commands:

- `c`: Continue execution
- `q`: Quit
- `si [N]`: Step N instructions (default 1)
- `info r`: Display register values
- `info w`: Display watchpoints
- `x N ADDR`: Examine N words of memory starting at ADDR
- `p EXPR`: Evaluate expression
- `w EXPR`: Set watchpoint on expression
- `d N`: Delete watchpoint N

The debugger supports expression evaluation and watchpoints for monitoring memory/register changes.

## Architecture Overview

### NEMU Structure
```
nemu/
├── src/
│   ├── cpu/           # CPU execution and differential testing
│   ├── isa/           # ISA-specific implementations (x86, mips32, riscv32, etc.)
│   ├── monitor/       # Monitor and debugger (sdb)
│   ├── memory/        # Physical/virtual memory management
│   ├── device/        # Device emulation (serial, timer, keyboard, VGA, audio)
│   ├── engine/        # Execution engines (interpreter)
│   └── utils/         # Utilities
├── include/           # Header files
├── tools/             # Build tools (kconfig, difftest, etc.)
└── configs/           # Predefined configurations
```

**Key concepts:**
- **ISA abstraction**: Each ISA (x86, MIPS, RISC-V) has its own implementation in `src/isa/<isa>/`
- **Differential testing**: NEMU can compare execution with reference implementations (QEMU, Spike) to verify correctness
- **Device I/O**: Supports both port-mapped I/O and memory-mapped I/O

### Abstract Machine Structure
```
abstract-machine/
├── am/                # Abstract Machine runtime
│   ├── src/           # Platform-specific implementations
│   └── include/       # AM API headers
├── klib/              # Kernel library (libc subset)
└── scripts/           # Build scripts for different architectures
```

**AM provides:**
- TRM (Turing Machine): Basic computation and memory
- IOE (I/O Extension): Input/output devices
- CTE (Context Extension): Interrupt/exception handling
- VME (Virtual Memory Extension): Virtual memory and protection
- MPE (Multiprocessing Extension): Multi-processor support

## Git Workflow

The project uses automatic git commits for tracking progress:
- Commits are created automatically when building/running NEMU
- Student ID and name are configured in the root Makefile
- Use `make submit` to submit assignments (requires network access)

## Common Development Patterns

### Adding ISA Instructions
1. Locate the ISA directory: `nemu/src/isa/<isa>/`
2. Add instruction decoding in `inst.c`
3. Implement instruction execution
4. Update instruction tables if needed

### Implementing Device Support
1. Add device implementation in `nemu/src/device/`
2. Register device in `device.c`
3. Configure memory-mapped I/O regions
4. Implement device-specific I/O handlers

### Debugging Execution Issues
1. Enable instruction tracing in menuconfig (ITRACE)
2. Check execution log: `nemu/build/nemu-log.txt`
3. Use differential testing to compare with reference implementation
4. Use SDB debugger to step through execution and examine state

## Testing

The project includes differential testing support:
- Configure with `CONFIG_DIFFTEST` in menuconfig
- Specify reference implementation (QEMU, Spike, KVM)
- NEMU will compare execution state with reference at each instruction
- Useful for verifying correctness of ISA implementation
