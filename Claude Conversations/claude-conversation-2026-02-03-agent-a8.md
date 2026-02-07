# Claude Conversation Log

Session ID: agent-a8c0acc
Date: 2026-02-03 04:26:48

---

## 👤 User

Explore the codebase structure to understand the NEMU project. Look for:
1. Main directories and their purposes
2. Key source files and their organization
3. The overall architecture of the NEMU emulator
4. What components exist (CPU, memory, devices, etc.)

Provide a comprehensive overview of the codebase structure.

---

## 🤖 Claude



I'll help you explore the NEMU project codebase structure. Let me start by examining the directory layout and key files.

---

## 🤖 Claude

Let me explore the NEMU directory structure in more detail:

---

## 🤖 Claude



Let me continue exploring the key components and provide a comprehensive overview.

---

## 🤖 Claude

Based on my comprehensive exploration of the NEMU codebase, here is a detailed overview of the project structure:

## NEMU Project - Comprehensive Codebase Overview

### Project Summary
NEMU is a CPU emulator developed at Nanjing University as part of the ICS (Introduction to Computer System) course. It's a full-system emulator that supports multiple instruction set architectures (ISAs) and includes debugging capabilities.

**Repository**: https://github.com/NJU-ProjectN/nemu
**License**: Mulan PSL v2
**Total Source Files**: 57 C files, 10 header files in src/

---

### Main Directory Structure

```
/mnt/c/Users/Omni/Documents/ics2025/
├── nemu/                          # Main NEMU emulator
├── abstract-machine/              # Hardware abstraction layer
├── learning/                      # Learning notes and documentation
└── Makefile, README.md, init.sh
```

---

### NEMU Core Architecture

#### 1. **CPU Module** (`/nemu/src/cpu/`)
- **cpu-exec.c**: Main CPU execution loop
  - Implements instruction fetch-decode-execute cycle
  - Handles tracing and differential testing
  - Manages CPU state and program counter
  - Supports single-step execution for debugging

- **difftest/**: Differential testing framework
  - dut.c: Device Under Test (NEMU) interface
  - ref.c: Reference implementation interface
  - Validates NEMU behavior against reference implementations

**Key Files**:
- `/nemu/include/cpu/cpu.h`: CPU execution interface
- `/nemu/include/cpu/decode.h`: Instruction decoding with pattern matching
- `/nemu/include/cpu/ifetch.h`: Instruction fetch interface

#### 2. **Memory Module** (`/nemu/src/memory/`)
- **paddr.c**: Physical address memory management
  - Handles guest-to-host address translation
  - Manages physical memory (pmem) allocation
  - Routes memory access to MMIO devices when needed
  - Supports configurable memory size and base address

- **vaddr.c**: Virtual address translation
  - Currently implements direct translation (no MMU)
  - Delegates to physical address layer

**Key Files**:
- `/nemu/include/memory/paddr.h`: Physical memory interface
- `/nemu/include/memory/vaddr.h`: Virtual memory interface
- `/nemu/include/memory/host.h`: Host memory operations

**Memory Configuration**:
- PAGE_SIZE: 4KB (1 << 12)
- Configurable MBASE and MSIZE
- Support for 32-bit and 64-bit physical addresses

#### 3. **Device Module** (`/nemu/src/device/`)
Implements various I/O devices:

- **device.c**: Device initialization and management
- **io/map.c**: I/O address space mapping
- **io/mmio.c**: Memory-mapped I/O handling
- **io/port-io.c**: Port-based I/O handling
- **serial.c**: Serial port (UART) - outputs to stderr
- **timer.c**: Real-time clock (RTC) device
- **keyboard.c**: Keyboard input device
- **vga.c**: Video graphics adapter
- **audio.c**: Audio device
- **disk.c**: Disk storage device
- **sdcard.c**: SD card device
- **alarm.c**: Alarm/interrupt handling
- **intr.c**: Interrupt management

**Device Mapping**:
- IOMap structure tracks device address ranges
- Supports both MMIO and port I/O
- Callback mechanism for device-specific handlers
- Maximum 16 concurrent device mappings

#### 4. **ISA-Specific Modules** (`/nemu/src/isa/`)
Supports 4 instruction set architectures:

**a) x86 (`/nemu/src/isa/x86/`)**
- **inst.c** (220 lines): Instruction implementation
- **reg.c**: Register operations and testing
- **init.c**: ISA initialization with built-in test image
- **system/intr.c**: Interrupt/exception handling
- **system/mmu.c**: Memory management unit
- **include/isa-def.h**: CPU state definition
  - 8 general-purpose registers (GPR)
  - Support for 32-bit, 16-bit, and 8-bit register access
  - Program counter (pc)

**b) MIPS32 (`/nemu/src/isa/mips32/`)**
- **inst.c** (76 lines): Instruction implementation
- **reg.c**: 32 general-purpose registers
- **init.c**: ISA initialization
- **system/intr.c, system/mmu.c**: System support

**c) RISC-V (`/nemu/src/isa/riscv32/`)**
- **inst.c** (78 lines): Instruction implementation
- **reg.c**: 32 registers (or 16 for RVE variant)
- **init.c**: ISA initialization
- **system/intr.c, system/mmu.c**: System support
- Supports both RV32 and RV64 variants

**d) LoongArch32r (`/nemu/src/isa/loongarch32r/`)**
- **inst.c** (74 lines): Instruction implementation
- **reg.c**: 32 general-purpose registers
- **init.c**: ISA initialization
- **system/intr.c, system/mmu.c**: System support

**Common ISA Interface** (`/nemu/include/isa.h`):
- `init_isa()`: Initialize ISA
- `isa_exec_once()`: Execute single instruction
- `isa_reg_display()`: Display registers
- `isa_reg_str2val()`: Convert register name to value
- `isa_mmu_translate()`: Virtual-to-physical address translation
- `isa_raise_intr()`: Raise interrupt/exception
- `isa_query_intr()`: Query pending interrupts
- `isa_difftest_checkregs()`: Differential testing support

#### 5. **Monitor/Debugger Module** (`/nemu/src/monitor/`)
- **monitor.c**: Main monitor initialization
  - Loads guest image
  - Initializes all subsystems
  - Handles command-line arguments

- **sdb/sdb.c**: Simple Debugger (SDB)
  - Interactive command interface using readline
  - Commands: c (continue), q (quit), si (step), etc.
  - Batch mode support

- **sdb/expr.c**: Expression evaluator
  - Regex-based tokenization
  - Supports arithmetic operations
  - Register and memory access in expressions
  - Token types: numbers, hex, registers, operators

- **sdb/watchpoint.c**: Watchpoint management
  - Set/remove watchpoints
  - Monitor memory/register changes

#### 6. **Execution Engine** (`/nemu/src/engine/interpreter/`)
- **init.c**: Engine initialization
  - Selects between SDB mainloop (native) or direct execution (AM mode)
- **hostcall.c**: Host system calls interface

#### 7. **Utility Module** (`/nemu/src/utils/`)
- **log.c**: Logging system with file output
- **disasm.c**: Disassembly using Capstone library
- **state.c**: NEMU state management
- **timer.c**: Timing utilities

---

### Header Files Organization (`/nemu/include/`)

```
include/
├── common.h              # Common definitions, type definitions
├── debug.h               # Debug macros (Log, Assert, panic)
├── macro.h               # Utility macros
├── utils.h               # Utility functions
├── isa.h                 # ISA interface
├── cpu/
│   ├── cpu.h             # CPU execution interface
│   ├── decode.h          # Instruction decoding
│   ├── ifetch.h          # Instruction fetch
│   └── difftest.h        # Differential testing
├── memory/
│   ├── paddr.h           # Physical memory
│   ├── vaddr.h           # Virtual memory
│   └── host.h            # Host memory operations
└── device/
    ├── map.h             # I/O mapping
    ├── mmio.h            # MMIO interface
    └── alarm.h           # Alarm interface
```

---

### Build System

**Configuration** (`/nemu/Kconfig`):
- ISA selection: x86, mips32, riscv, loongarch32r
- Engine: Interpreter (only option currently)
- Mode: System mode (full-system emulation)
- Build target: Native ELF, Shared object, or Abstract-Machine
- Compiler options: GCC, G++, Clang
- Features: Tracing, differential testing, debugging

**Build Files**:
- `/nemu/Makefile`: Main build configuration
- `/nemu/scripts/build.mk`: Build rules
- `/nemu/scripts/config.mk`: Configuration rules
- `/nemu/scripts/native.mk`: Native build rules
- `filelist.mk` files: Source file organization

---

### Tools and Utilities (`/nemu/tools/`)

- **capstone/**: Disassembly library
- **kconfig/**: Kernel configuration system
- **gen-expr/**: Expression generator tool
- **fixdep/**: Dependency fixing tool
- **qemu-diff/**: QEMU differential testing
- **kvm-diff/**: KVM differential testing
- **spike-diff/**: Spike (RISC-V) differential testing

---

### Key Design Patterns

1. **Modular ISA Support**: Each ISA has identical interface but different implementations
2. **Device Abstraction**: Devices register with address ranges and callbacks
3. **Pattern Matching Decoding**: Instruction patterns use bit masks for efficient matching
4. **Differential Testing**: Compare NEMU behavior against reference implementations
5. **Configurable Build**: Kconfig system allows flexible feature selection

---

### Execution Flow

```
main() 
  ├─ init_monitor() / am_init_monitor()
  │   ├─ init_mem()
  │   ├─ init_device()
  │   ├─ init_isa()
  │   └─ init_sdb()
  └─ engine_start()
      ├─ sdb_mainloop() [native mode]
      │   └─ cpu_exec()
      │       └─ exec_once()
      │           └─ isa_exec_once()
      └─ cpu_exec(-1) [AM mode]
```

---

### Configuration Options

**Memory**:
- CONFIG_MBASE: Memory base address
- CONFIG_MSIZE: Memory size
- CONFIG_PMEM_MALLOC: Allocate pmem dynamically
- CONFIG_MEM_RANDOM: Initialize memory with random values

**Features**:
- CONFIG_ITRACE: Instruction tracing
- CONFIG_DIFFTEST: Differential testing
- CONFIG_DEVICE: Device support
- CONFIG_HAS_SERIAL, CONFIG_HAS_TIMER, CONFIG_HAS_KEYBOARD, etc.

**Debugging**:
- CONFIG_TRACE: Enable tracing
- CONFIG_CC_DEBUG: Debug symbols
- CONFIG_CC_ASAN: Address sanitizer

---

### File Statistics

- Total C source files: 57
- Total header files: 10
- Largest ISA implementation: x86 (220 lines in inst.c)
- Other ISAs: MIPS32 (76), RISC-V (78), LoongArch32r (74) lines

This architecture demonstrates a well-designed emulator with clear separation of concerns, supporting multiple ISAs through a unified interface, comprehensive debugging capabilities, and flexible configuration options.

---

