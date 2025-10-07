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

#include <isa.h>
#include <memory/paddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256,  // 这里空格为什么要用256
  TK_EQ,
  TK_NEQ,
  TK_AND,
  TK_NUM, //数字
  TK_HEX, //十六进制数
  TK_REG, //寄存器
  TK_DEREF, //指针解引用
  // 只有多字符的匹配需要在这里定义
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // not equal
  {"&&", TK_AND},       // logical and
  {"0x[0-9a-fA-F]+", TK_HEX},  // hexadecimal number
  {"\\$[a-zA-Z0-9_]+", TK_REG}, // register
  {"[0-9]+", TK_NUM},     // decimal number
  {"\\*", '*'},         // multiply or dereference
  {"\\/", '/'},         // divide
  {"\\(", '('},   // left parenthesis
  {"\\)", ')'},   // right parenthesis
};



#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  // 这个函数是取出表达式中的token,并记录到tokens数组中
  // position 是当前匹配的位置
  int position = 0;
  // i是当前匹配的规则索引 , 遍历所有规则
  int i;
  // pmatch是当前匹配的结果, 存储匹配的起始和结束位置
  regmatch_t pmatch;

  //重置token计数器，准备存储新的token
  nr_token = 0;

  // 遍历表达式中的每个字符，直到遇到结束符
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    // 遍历所有规则，尝试匹配当前位置的字符
    for (i = 0; i < NR_REGEX; i ++) {
      // 如果当前位置的字符与规则匹配，则记录匹配结果
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        // 记录匹配的起始和结束位置
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        // 记录匹配的规则和位置
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        // 更新当前匹配位置
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            //跳过空格不记录
            break;
          case TK_NUM:
          case TK_HEX:
          case TK_REG:
            //记录数字、十六进制数、寄存器
            tokens[nr_token].type = rules[i].token_type;
            if (substr_len >= 32) {
              printf("Token长度超过32\n");
              return false;
            }
            // strncpy
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            // 这一句非常重要
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          case '+':
          case '-':
          case '*':
          case '/':
          case '(':
          case ')':
          case TK_EQ:
          case TK_NEQ:
          case TK_AND:
            //记录运算符
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static void print_tokens() {
  printf("Total tokens: %d\n", nr_token);
  for (int i = 0; i < nr_token; i++) {
    printf("Token[%d]: type=%d, str=\"%s\"\n", 
           i, tokens[i].type, tokens[i].str);
  }
}

/* 检查表达式 tokens[p..q] 是否被一对匹配的括号包围
 * 返回值:
 *   true - 被一对括号包围
 *   false - 不被括号包围或括号不匹配
 */
static bool check_parentheses(int p, int q) {
  // 首先检查最外层是否是括号
  if (tokens[p].type != '(' || tokens[q].type != ')') {
    return false;
  }
  
  // 检查括号是否匹配
  int count = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      count++;
    } else if (tokens[i].type == ')') {
      count--;
    }
    
    // 如果在中间位置count变为0，说明最外层括号不是包围整个表达式的
    if (count == 0 && i < q) {
      return false;
    }
    
    // 如果count变为负数，说明括号不匹配
    if (count < 0) {
      return false;
    }
  }
  
  // 最后count应该为0
  return count == 0;
}

/* 找到主运算符的位置
 * 主运算符是最后被计算的运算符
 * 优先级: + - < * /
 * 在括号内的运算符不是主运算符
 * 返回主运算符在tokens数组中的下标
 */
static int find_main_op(int p, int q) {
  int main_op = -1;
  int min_priority = 100;  // 用一个大数表示最高优先级
  int paren_count = 0;  // 括号计数器
  
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      paren_count++;
      continue;
    } else if (tokens[i].type == ')') {
      paren_count--;
      continue;
    }
    
    // 括号内的运算符不考虑
    if (paren_count > 0) {
      continue;
    }
    
    // 根据运算符类型确定优先级
    int priority = -1;
    switch (tokens[i].type) {
      case TK_AND:
        priority = 0;  // 最低优先级
        break;
      case TK_EQ:
      case TK_NEQ:
        priority = 1;  // 比较运算符
        break;
      case '+':
      case '-':
        priority = 2;  // 加减法
        break;
      case '*':
      case '/':
        priority = 3;  // 乘除法
        break;
      default:
        continue;  // 不是运算符，跳过
    }
    
    // 选择优先级最低的（最后计算），如果优先级相同，选择最右边的
    if (priority <= min_priority) {
      min_priority = priority;
      main_op = i;
    }
  }
  
  return main_op;
}

/* 递归求值 tokens[p..q] 表示的表达式
 * 返回表达式的值
 */
static uint32_t eval(int p, int q, bool *success) {
  if (p > q) {
    /* Bad expression */
    *success = false;
    printf("Error: Bad expression (p > q)\n");
    return 0;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number, hex, register, or dereference.
     * Return the value of the token.
     */
    if (tokens[p].type == TK_NUM) {
      uint32_t num;
      sscanf(tokens[p].str, "%u", &num);
      return num;
    } else if (tokens[p].type == TK_HEX) {
      uint32_t num;
      sscanf(tokens[p].str, "%x", &num);
      return num;
    } else if (tokens[p].type == TK_REG) {
      // 获取寄存器值
      bool reg_success = false;
      word_t reg_val = isa_reg_str2val(tokens[p].str, &reg_success);
      if (!reg_success) {
        *success = false;
        printf("Error: Invalid register name '%s'\n", tokens[p].str);
        return 0;
      }
      return reg_val;
    } else if (tokens[p].type == TK_DEREF) {
      // 指针解引用
      *success = false;
      printf("Error: Invalid dereference at position %d\n", p);
      return 0;
    } else {
      *success = false;
      printf("Error: Expected a number, hex, register, or dereference at position %d\n", p);
      return 0;
    }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  }
  else {
    /* We should do more things here. */
    int op = find_main_op(p, q);
    
    if (op == -1) {
      // 检查是否是单目运算符（如指针解引用）
      if (p == q - 1 && tokens[p].type == TK_DEREF) {
        uint32_t addr = eval(p + 1, q, success);
        if (!*success) return 0;
        // 从内存中读取值
        return paddr_read(addr, 4);
      }
      *success = false;
      printf("Error: No main operator found\n");
      return 0;
    }
    
    uint32_t val1 = eval(p, op - 1, success);
    if (!*success) return 0;
    
    uint32_t val2 = eval(op + 1, q, success);
    if (!*success) return 0;

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/':
        if (val2 == 0) {
          *success = false;
          printf("Error: Division by zero\n");
          return 0;
        }
        return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      default:
        *success = false;
        printf("Error: Unknown operator\n");
        return 0;
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  
  print_tokens();
  
  // 检查是否有token
  if (nr_token == 0) {
    *success = false;
    printf("Error: Empty expression\n");
    return 0;
  }
  
  // 识别指针解引用：如果*前面是运算符、括号或开头，则为解引用
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*' && 
        (i == 0 || tokens[i - 1].type == '+' || tokens[i - 1].type == '-' || 
         tokens[i - 1].type == '*' || tokens[i - 1].type == '/' || 
         tokens[i - 1].type == '(' || tokens[i - 1].type == TK_EQ || 
         tokens[i - 1].type == TK_NEQ || tokens[i - 1].type == TK_AND)) {
      tokens[i].type = TK_DEREF;
    }
  }
  
  // 检查括号匹配
  int paren_count = 0;
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '(') {
      paren_count++;
    } else if (tokens[i].type == ')') {
      paren_count--;
    }
    if (paren_count < 0) {
      *success = false;
      printf("Error: Unmatched parentheses\n");
      return 0;
    }
  }
  if (paren_count != 0) {
    *success = false;
    printf("Error: Unmatched parentheses\n");
    return 0;
  }
  
  *success = true;
  return eval(0, nr_token - 1, success);
}
