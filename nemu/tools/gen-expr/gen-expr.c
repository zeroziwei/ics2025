/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
// 这里是模板， %s 是占位符，用于生成代码
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

//当前缓冲区位置
static int buf_pos = 0;

// 生成小于n的随机数
static uint32_t choose (uint32_t n ){
  return rand() % n;
}

// 向缓冲区添加字符

static void gen(char c ){
  if (buf_pos < sizeof(buf) - 1) {
    buf[buf_pos++] = c;
    // buf[buf_pos] = '\0';
  }
}

//向缓冲区添加字符串
static void gen_str(const char *str){
  // while (*s) {
  //   gen(*s++);
  // }
  while (*str && buf_pos < sizeof(buf) - 1) {
    buf[buf_pos++] = *str++;
    // buf[buf_pos] = '\0';
  }
}

// 生成随机数字
static void gen_num() {
  uint32_t num = choose(1000) + 1;
  char num_str[32];
  sprintf(num_str, "%u", num);
  gen_str(num_str);
}

// 生成随机运算符
static void gen_rand_op() {
  const char ops[] = {'+', '-','*','/'};
  char op = ops[choose(4)];
  gen(op);
}

// 生成随机空格
static void gen_rand_space() {
  int space_count = choose(3); // 0-2个空格
  for (int i = 0; i < space_count; i++) {
    gen(' ');
  }
}

// 判断是否溢出
static int would_overflow(int additional_chars) {
  return buf_pos + additional_chars >= sizeof(buf) - 1;
}

// 使用递归下载方式来生成表达式
static void gen_rand_expr() {
  // 重置缓冲区位置
  buf_pos = 0;
  // 如果缓冲区可能溢出，生成简单数字
  if (would_overflow(10)) {
    gen_num();
    return;
  }
  switch (choose(3)) {
    case 0: 
      gen_num(); 
      break;
    case 1: 
      gen('('); 
      gen_rand_space();
      gen_rand_expr(); 
      gen_rand_space();
      gen(')'); 
      break;
    default: 
      gen_rand_expr(); 
      gen_rand_space();
      gen_rand_op(); 
      gen_rand_space();
      gen_rand_expr(); 
      break;
  }
  // 保字符串以null结尾
  buf[buf_pos] = '\0';
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    // 将生成的表达式插入到c程序模板中
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    //这里是将生成好了的模板文件传上去咯
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    // 是一个标准C库函数，用于创建一个管道并执行shell命令。
    // 它允许程序与另一个进程进行通信。
    fp = popen("/tmp/.expr", "r");
    // 返回值是一个文件指针，运行失败是NULL喽
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    //fscanf 是从文件里面读喽
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
