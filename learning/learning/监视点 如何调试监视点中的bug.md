
整理调试监视点实现中 bug 的方法和工具。

## 调试监视点实现中的 Bug

### 1. Address Sanitizer（地址消毒器）

#### 什么是 Address Sanitizer？
自动检测内存错误的工具，包括：
- 缓冲区溢出（越界访问）
- 使用已释放的内存（use-after-free）
- 内存泄漏
- 未初始化的内存使用

#### 如何启用？

方法1：通过 menuconfig
```
make menuconfig
→ Build Options
  → [*] Enable address sanitizer
```

方法2：手动编译
```bash
make clean
make CFLAGS="-fsanitize=address -g"  # -g 添加调试信息
```

#### 示例：故意触发一个 bug

在 `watchpoint.c` 中添加一个空指针访问：

```c
// 在 new_wp() 函数中故意添加bug
void new_wp(char *expr_str) {
  // ... 原有代码 ...
  
  // 故意访问空指针
  WP *null_wp = NULL;
  null_wp->NO = 0;  // 这会触发段错误
}
```

运行后 Address Sanitizer 会报告：
```
==12345==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000000
    #0 0x401234 in new_wp watchpoint.c:47
    #1 0x401567 in cmd_w sdb.c:143
    #2 0x401890 in sdb_mainloop sdb.c:240
    #3 0x401abc in main nemu-main.c:56

==12345==ABORTING
```

#### Address Sanitizer 的优势
- 自动检测：无需手动添加检查代码
- 精确定位：指出错误发生的文件和行号
- 检测多种错误：越界、use-after-free、泄漏等

#### 注意事项
- 性能开销：程序会变慢（约 2-3 倍），调试时使用即可
- 内存占用：会增加约 2-3 倍内存使用

### 2. GDB 调试器

#### 基本使用流程

```bash
# 1. 编译时添加调试信息
make clean
make CFLAGS="-g -O0"  # -O0 关闭优化，便于调试

# 2. 启动 GDB
gdb ./build/nemu

# 3. 设置断点
(gdb) break watchpoint.c:47        # 在指定行设置断点
(gdb) break new_wp                  # 在函数入口设置断点
(gdb) break check_wp                # 在检查函数设置断点

# 4. 运行程序
(gdb) run

# 5. 程序暂停后，查看状态
(gdb) print wp                      # 打印变量值
(gdb) print head                    # 查看链表头
(gdb) print free_                   # 查看空闲链表
(gdb) print wp->expr                # 查看表达式字符串
(gdb) print wp->old_val             # 查看旧值

# 6. 单步执行
(gdb) step                          # 单步进入函数
(gdb) next                          # 单步跳过函数
(gdb) continue                      # 继续执行

# 7. 查看调用栈
(gdb) backtrace                     # 查看函数调用栈
(gdb) frame 1                       # 切换到上一层栈帧

# 8. 查看代码
(gdb) list                          # 显示当前代码
(gdb) list 40,60                    # 显示指定行范围的代码
```

#### 实际调试示例

假设监视点链表操作有问题：

```bash
# 启动 GDB
gdb ./build/nemu

# 在关键位置设置断点
(gdb) break free_wp_internal
(gdb) break new_wp

# 运行
(gdb) run

# 当程序暂停在 new_wp 时
(gdb) print head
(gdb) print free_
(gdb) print wp_pool[0]
(gdb) print wp_pool[0].next

# 单步执行，观察链表变化
(gdb) step
(gdb) print free_                   # 观察 free_ 是否更新
(gdb) print head                    # 观察 head 是否更新
```

#### GDB 高级技巧

```bash
# 条件断点
(gdb) break watchpoint.c:141 if new_val != wp->old_val

# 监视变量变化
(gdb) watch wp->old_val              # 当变量值改变时暂停

# 查看内存
(gdb) x/10x &wp_pool                # 查看内存内容（16进制）
(gdb) x/10s wp->expr                # 查看字符串

# 查看寄存器
(gdb) info registers

# 查看所有断点
(gdb) info breakpoints

# 删除断点
(gdb) delete 1                      # 删除编号为1的断点
```

### 3. printf 调试

#### 在关键位置添加输出

在 `watchpoint.c` 中添加调试输出：

```c
void new_wp(char *expr_str) {
  printf("[DEBUG] new_wp called with expr: %s\n", expr_str);
  
  if (free_ == NULL) {
    printf("[DEBUG] No free watchpoint available\n");
    printf("No free watchpoint available\n");
    return;
  }
  
  WP *wp = free_;
  printf("[DEBUG] Got watchpoint from pool, NO=%d\n", wp->NO);
  printf("[DEBUG] Before: free_=%p, head=%p\n", free_, head);
  
  free_ = free_->next;
  printf("[DEBUG] After updating free_: free_=%p\n", free_);
  
  // ... 其他代码 ...
  
  wp->next = head;
  head = wp;
  printf("[DEBUG] After adding to head: head=%p, head->next=%p\n", 
         head, head->next);
}
```

#### 使用宏简化调试输出

```c
// 在文件开头定义
#ifdef DEBUG
#define DBG_PRINT(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#else
#define DBG_PRINT(fmt, ...)
#endif

// 使用时
DBG_PRINT("new_wp: expr=%s, free_=%p\n", expr_str, free_);
```

### 4. assert 断言

#### 在关键位置添加断言

```c
#include <assert.h>

void new_wp(char *expr_str) {
  assert(expr_str != NULL);         // 确保表达式不为空
  assert(free_ != NULL);             // 确保有空闲监视点
  
  WP *wp = free_;
  assert(wp != NULL);               // 确保获取到监视点
  
  free_ = free_->next;
  
  // 检查链表完整性
  assert(wp->NO >= 0 && wp->NO < NR_WP);
  
  // ... 其他代码 ...
}

static void free_wp_internal(WP *wp) {
  assert(wp != NULL);                // 确保指针不为空
  
  // 检查 wp 是否在 head 链表中
  WP *p = head;
  bool found = false;
  while (p) {
    if (p == wp) {
      found = true;
      break;
    }
    p = p->next;
  }
  assert(found);                     // 确保要删除的节点在链表中
  
  // ... 其他代码 ...
}

bool check_wp() {
  WP *wp = head;
  while (wp) {
    assert(wp->expr != NULL);        // 确保表达式字符串有效
    assert(wp->NO >= 0 && wp->NO < NR_WP);  // 确保编号有效
    
    // ... 其他代码 ...
    
    wp = wp->next;
  }
  return changed;
}
```

#### assert 的优势
- 自动检查：条件不满足时立即终止并报告
- 零开销（Release 模式）：可禁用，不影响性能
- 文档作用：明确代码的前置条件

### 5. 调试策略建议

根据文档建议，按以下顺序使用：

```
1. 编译时检查
   ↓ 使用 -Wall -Werror
   
2. 运行时断言
   ↓ 使用 assert()
   
3. 启用 Sanitizer
   ↓ 使用 Address Sanitizer
   
4. 打印调试
   ↓ 使用 printf()
   
5. 交互式调试
   ↓ 使用 GDB
```

### 6. 常见 Bug 类型及调试方法

#### Bug 1：空指针解引用

症状：段错误（Segmentation Fault）

调试：
```c
// 在访问指针前添加断言
assert(wp != NULL);
assert(head != NULL);
```

或使用 Address Sanitizer 自动检测。

#### Bug 2：链表操作错误

症状：程序崩溃、链表损坏

调试：
```c
// 在链表操作前后打印状态
printf("Before: head=%p, free_=%p\n", head, free_);
// ... 链表操作 ...
printf("After: head=%p, free_=%p\n", head, free_);

// 检查链表完整性
WP *p = head;
int count = 0;
while (p) {
    assert(count < NR_WP);  // 防止无限循环
    count++;
    p = p->next;
}
```

#### Bug 3：数组越界

症状：内存损坏、不可预测的行为

调试：
```c
// 使用断言检查索引
assert(wp->NO >= 0 && wp->NO < NR_WP);
assert(i >= 0 && i < NR_WP);
```

或使用 Address Sanitizer。

#### Bug 4：表达式求值错误

症状：监视点不触发或误触发

调试：
```c
// 在 check_wp() 中添加详细输出
bool check_wp() {
  WP *wp = head;
  while (wp) {
    printf("[DEBUG] Checking watchpoint %d: %s\n", wp->NO, wp->expr);
    printf("[DEBUG] Old value: %u (0x%x)\n", wp->old_val, wp->old_val);
    
    bool success = false;
    word_t new_val = expr(wp->expr, &success);
    printf("[DEBUG] New value: %u (0x%x), success=%d\n", 
           new_val, new_val, success);
    
    if (success && new_val != wp->old_val) {
      printf("[DEBUG] Watchpoint triggered!\n");
      // ...
    }
    wp = wp->next;
  }
}
```

### 7. 完整调试示例

假设遇到“删除监视点后程序崩溃”的问题：

```bash
# 步骤1：启用 Address Sanitizer
make menuconfig  # 启用 address sanitizer
make clean && make

# 步骤2：运行程序
./build/nemu

# 步骤3：如果 Address Sanitizer 报告错误，查看详细信息
# 如果信息不够详细，使用 GDB

# 步骤4：使用 GDB 调试
gdb ./build/nemu
(gdb) break delete_wp
(gdb) break free_wp_internal
(gdb) run

# 步骤5：在 GDB 中观察
(gdb) print head
(gdb) print free_
(gdb) print wp_pool
(gdb) step
(gdb) print head        # 观察链表变化
(gdb) print free_       # 观察空闲链表变化
```

## 总结

调试监视点的工具链：
1. Address Sanitizer：自动检测内存错误
2. GDB：交互式调试，查看程序状态
3. printf：追踪程序执行流程
4. assert：检查代码假设条件

建议：
- 开发时启用 `-Wall -Werror`
- 关键位置添加 `assert()`
- 调试时启用 Address Sanitizer
- 复杂问题使用 GDB 深入分析

遵循这些方法，可以更快定位和修复监视点实现中的 bug。