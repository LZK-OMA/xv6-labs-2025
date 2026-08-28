#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"
#define DATASIZE 4096

int
main(int argc, char *argv[])
{
  // Your code here.
  // 1. 申请大块内存。
  // 由于内核没清零，buf 里的内容是之前进程留下的
  char *buf = sbrk(DATASIZE);
  if(buf == (char*)-1) {
    exit(1);
  }

  char ch[DATASIZE / 4];
  int j = 0;

  // 2. 扫描这块内存空间
  for(int i = 0; i < DATASIZE; ++i){
    char c = buf[i];
    
    // 检查
    if(((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) && j < (DATASIZE / 4) - 1){
      ch[j++] = c;
    } 
    // 如果读到 \0 且之前已经积累了一定长度的字符，说明可能找到
    else if(c == '\0' && j >= 2){ 
      ch[j] = '\0'; // 确保字符串正确结束
      printf("%s\n", ch);
      break; // 找到第一个符合条件的字符串即认为找到，退出循环
    }
    else {
      // 如果遇到非字符且不是 \0，或者是太短的干扰信息，重置计数器重新寻找
      j = 0;
    }
  }
  exit(1);
}
