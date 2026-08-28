#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

#define NULL 0

// 查找函数：递归遍历目录并匹配文件名
void find(char *path, char *filename, char *tip_comm, char *command, char *parameter)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // 打开当前路径
    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 获取路径状态
    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // 如果当前路径是一个文件
    if(st.type == T_FILE){
        // 提取路径中的最后一个文件名部分
        char *name = path + strlen(path);
        while(name >= path && *name != '/')
            name--;
        name++;

        // 检查文件名是否匹配
        if(strcmp(name, filename) == 0){
            // 如果没有 -exec 命令，直接打印路径
            if(tip_comm == NULL){
                printf("%s\n", path);
            } else {
                // 如果有 -exec，则 fork 子进程执行指定命令
                int pid = fork();
                if(pid > 0){
                    wait(0);
                } else if(pid == 0){
                    if(parameter != NULL){
                        char *argv_s[] = { command, parameter, path, 0 };
                        exec(command, argv_s);
                    } else{
                        char *argv_s[] = { command, path, 0 };
                        exec(command, argv_s);
                    }
                    exit(1);
                }
            }
        }
        close(fd);
        return;
    }

    // 如果当前路径是一个目录
    if(st.type == T_DIR){
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
            fprintf(2, "find: path too long\n");
            close(fd);
            return;
        }
        
        // 准备缓冲区，拼接子路径
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        // 读取目录项
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            // 跳过无效项
            if(de.inum == 0){
                continue;
            }

            // 不递归进入 "." 和 ".."
            if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0){
                continue;
            }

            // 拼接子文件名并递归调用
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            find(buf, filename, tip_comm, command, parameter);
        }
    }
    close(fd);
}

int main(int argc, char **argv)
{
    // 参数检查
    if(argc < 3){
        fprintf(2, "Usage: find <directory> <filename> [-exec <command> <parameter>]\n");
        exit(1);
    }

    // 处理带有 -exec 的特殊情况 (argc == 6)
    if(argc == 6 && strcmp(argv[3], "-exec") == 0){
        find(argv[1], argv[2], argv[3], argv[4], argv[5]);
        exit(0);
    }

    // 普通查找情况
    find(argv[1], argv[2], NULL, NULL, NULL);
    exit(0);
}