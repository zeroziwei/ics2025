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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  
  /* TODO: Add more members if necessary */
  char expr[256];  // 监视的表达式
  word_t old_val;  // 表达式的旧值

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

// 创建新的监视点
void new_wp(char *expr_str) {
  if (free_ == NULL) {
    printf("No free watchpoint available\n");
    return;
  }
  
  // 从空闲链表中取出一个监视点
  WP *wp = free_;
  free_ = free_->next;
  
  // 初始化监视点
  strncpy(wp->expr, expr_str, 255);
  wp->expr[255] = '\0';
  
  // 计算表达式的初始值
  bool success = false;
  wp->old_val = expr(wp->expr, &success);
  if (!success) {
    printf("Invalid expression: %s\n", expr_str);
    // 将监视点放回空闲链表
    wp->next = free_;
    free_ = wp;
    return;
  }
  
  // 将监视点添加到使用链表
  wp->next = head;
  head = wp;
  
  printf("Watchpoint %d: %s = %u (0x%x)\n", wp->NO, wp->expr, wp->old_val, wp->old_val);
}

// 释放监视点（内部函数）
static void free_wp_internal(WP *wp) {
  if (wp == NULL) return;
  
  // 从使用链表中移除
  if (head == wp) {
    head = wp->next;
  } else {
    WP *prev = head;
    while (prev && prev->next != wp) {
      prev = prev->next;
    }
    if (prev) {
      prev->next = wp->next;
    }
  }
  
  // 添加到空闲链表
  wp->next = free_;
  free_ = wp;
  
  printf("Watchpoint %d deleted\n", wp->NO);
}

// 删除指定编号的监视点
void delete_wp(int NO) {
  WP *wp = head;
  while (wp) {
    if (wp->NO == NO) {
      free_wp_internal(wp);
      return;
    }
    wp = wp->next;
  }
  printf("Watchpoint %d not found\n", NO);
}

// 显示所有监视点
void display_wp() {
  if (head == NULL) {
    printf("No watchpoints\n");
    return;
  }
  
  printf("Num     Type           Disp Enb Address            What\n");
  WP *wp = head;
  while (wp) {
    printf("%-8d%-15s%-5s%-5s%-20s%s\n", 
           wp->NO, "watchpoint", "keep", "y", "", wp->expr);
    wp = wp->next;
  }
}

// 检查所有监视点
bool check_wp() {
  bool changed = false;
  WP *wp = head;
  
  while (wp) {
    bool success = false;
    word_t new_val = expr(wp->expr, &success);
    
    if (success && new_val != wp->old_val) {
      printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
      printf("  Old value = %u (0x%x)\n", wp->old_val, wp->old_val);
      printf("  New value = %u (0x%x)\n", new_val, new_val);
      wp->old_val = new_val;
      changed = true;
    }
    
    wp = wp->next;
  }
  
  return changed;
}

