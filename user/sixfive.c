#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void solve(int fd, char *filename) {
    char c;
    int num = 0;
    int in_numbers = 0;

    while (read(fd, &c, 1) == 1) {
        if (c >= '0' && c <= '9') {
            num = num * 10 + (c - '0');
            in_numbers = 1;
        } else {
            // 读到非数字字符，判断之前的数字
            if (in_numbers && (num % 5 == 0 || num % 6 == 0)) {
                printf("%d\n", num); 
            }
            // 重置状态
            num = 0;
            in_numbers = 0;
        }
    }

    // 必须处理文件末尾没有分隔符的情况
    if (in_numbers && (num % 5 == 0 || num % 6 == 0)) {
        printf("%d\n", num);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: sixfive <file1> [file2 ...]\n");
        exit(1);
    }

    // 循环处理传入的多个文件
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            printf("sixfive: open %s failed\n", argv[i]);
            continue;
        }

        solve(fd, argv[i]);
        close(fd);
    }

    exit(0);
}