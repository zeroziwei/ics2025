#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  if (s == NULL) {
     panic("Null Pointer Exception"); 
  }
  size_t len = 0;
  while (*s != '\0') {
    len++;
    s++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  if (dst == NULL || src == NULL) {
    panic("Null Pointer Exception"); 
  }
  size_t len = strlen(src);
  for (size_t i = 0; i < len; i++) {
    dst[i] = src[i];
  }
  dst[len] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  if (dst == NULL || src == NULL) {
    panic("Null Pointer Exception"); 
  }
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  // 填充剩余的 '\0'
  for (; i < n; i++) {
    dst[i] = '\0';
  }
  return dst;
}


char *strcat(char *dst, const char *src) {
  if (dst == NULL || src == NULL) {
    panic("Null Pointer Exception");
  }
  
  // 找到 dst 的末尾
  size_t dst_len = strlen(dst);
  
  // 从 dst 的末尾位置开始复制 src
  size_t i = 0;
  while (src[i] != '\0') {
    dst[dst_len + i] = src[i];
    i++;
  }
  dst[dst_len + i] = '\0';  // 添加终止符
  
  return dst;
}


int strcmp(const char *s1, const char *s2) {
  panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
