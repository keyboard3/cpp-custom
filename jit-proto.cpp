#include "stdlib.h"
#include "sys/mman.h"
#include "stdio.h"
#include "string"
//来源 https://eli.thegreenplace.net/2013/11/05/how-to-jit-an-introduction

long add(long num)
{
    return num + 1;
}
//分配一块指定大小的可读可写内存并返回指针
//与malloc不同，内存分配在内存页的边界上，因此更适合调用mprotect
//为什么不用malloc，因为保护位只能在虚拟内存页边界上设置
//如果我们使用malloc,我们必须手动确保分配在页边界对齐，否则mprotect可能会出现问题
void *alloc_writable_memory(size_t size)
{
    //不直接设置PROT_EXEC，为了防止出现这块内存变成一个安全漏洞
    void *ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == (void *)-1)
    {
        perror("mmap");
        return NULL;
    }
    return ptr;
}
//在给定的内存上设置读、执行权限。它必须是页对齐的
int make_memory_executable(void *m, size_t size)
{
    if (mprotect(m, size, PROT_READ | PROT_EXEC) == -1)
    {
        perror("mprotect");
        return -1;
    }
    return 0;
}
//将add函数的机器码（动态生成）拷贝到这块可执行内存
void emit_code_into_memory(unsigned char *m)
{
    unsigned char code[] = {
        0x48,
        0x89,
        0xf8, // mov %rdi, %rax
        0x48,
        0x83,
        0xc0,
        0x01, // add $1, %rax
        0xc3  // ret
    };
    memcpy(m, code, sizeof(code));
}

const size_t SIZE = 1024;
typedef long (*JittedFunc)(long);
//运行可执行内存
void run_from_rwx()
{
    void *m = alloc_writable_memory(SIZE);
    emit_code_into_memory((unsigned char *)m);
    make_memory_executable(m, SIZE);

    //将这块指向这块内存的函数指针，转成可调用函数类型调用
    JittedFunc func = (JittedFunc)m;
    int result = func(2);
    printf("result = %d\n", result);
}
int main()
{
    run_from_rwx();
}