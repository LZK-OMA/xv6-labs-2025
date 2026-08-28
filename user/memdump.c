#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data);

int
main(int argc, char *argv[])
{
  if(argc == 1){
    printf("Example 1:\n");
    int a[2] = { 61810, 2025 };
    memdump("ii", (char*) a);
    
    printf("Example 2:\n");
    memdump("S", "a string");
    
    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *) &s);

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;
    
    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");
    
    printf("Example 4:\n");
    memdump("pihcS", (char*) &example);
    
    printf("Example 5:\n");
    memdump("sccccc", (char*) &example);
  } else if(argc == 2){
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while(n < sizeof(data)){
      int nn = read(0, data + n, sizeof(data) - n);
      if(nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}

void
memdump(char *fmt, char *data)
{
  // Your code here.
  for(; *fmt; ++fmt){
    if(*fmt == 'i'){
        // 打印 4 字节 32 位整数
        int val;
        memcpy(&val, data, 4);
        printf("%d\n", val);
        data = data + 4;
    } else if(*fmt == 'p'){
        // 打印 8 字节 64 位十六进制指针
        uint64 val;
        memcpy(&val, data, 8);
        printf("%lx\n", val);
        data = data + 8;
    } else if(*fmt == 'h'){
        // 打印 2 字节 16 位整数
        short val;
        memcpy(&val, data, 2);
        printf("%d\n", val);
        data = data + 2;
    } else if(*fmt == 'c'){
        // 打印 1 字节字符
        printf("%c\n", *data);
        data = data + 1;
    } else if(*fmt == 's'){
        // 提取 8 字节指针并打印其指向的字符串
        char *dst;
        memcpy(&dst, data, 8);
        printf("%s\n", dst);
        data = data + 8;
    } else if(*fmt == 'S'){
        // 打印当前数据位置开始的整个字符串
        printf("%s\n", data);
        // 'S' 之后通常认为处理结束
        break;
    }
}
}
