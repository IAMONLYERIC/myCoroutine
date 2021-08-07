/**
* @file  main.cpp
* @author gzy
* @version 0.1
* @brief   :Test for asymmetric coroutine library for C++
*/
#include "myCoroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void func1(void * arg)
{
    puts("1");
    puts("11");
    puts("111");
    puts("1111");

}

void func2(void * arg)
{
    puts("22");
    puts("22");
    coroutine_yield(*(schedule_t *)arg);
    puts("22");
    puts("22");
}

void func3(void *arg)
{
    puts("3333");
    puts("3333");
    coroutine_yield(*(schedule_t *)arg);
    puts("3333");
    puts("3333");

}

void context_test()
{
    char stack[1024*128];
    ucontext_t uc1,ucmain;

    // 获取当前上下文到uc1中
    getcontext(&uc1);
    // 给uci分配新的栈空间，并指定后继是ucmain
    uc1.uc_stack.ss_sp = stack;
    uc1.uc_stack.ss_size = 1024*128;
    uc1.uc_stack.ss_flags = 0;
    uc1.uc_link = &ucmain;
    
    // 设置uc1的入口函数为func1,执行完成后 执行uc1.uc_link
    makecontext(&uc1,(void (*)())func1,0);

    // 保存当前上下文到ucmain, 切换到uc1执行
    swapcontext(&ucmain,&uc1);
    puts("main");
}


void schedule_test()
{
    schedule_t s;
    
    // 创建两个协程 返回在协程数组里面的下标id
    int id1 = coroutine_create(s,func3,&s);
    int id2 = coroutine_create(s,func2,&s);
    puts("create over \n");
    
    while(!schedule_finished(s)){
        coroutine_resume(s,id1);
        coroutine_resume(s,id2);
    }
    puts("main over");

}

int test_perfor()
{
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    schedule_t s;
    int c_id[10000];
    for(int i = 0; i < 10000; i++)
    {
        c_id[i] = coroutine_create(s, func2, &s);
    }
    gettimeofday(&end, NULL);
    double timeuse = 1000 * ( end.tv_sec - start.tv_sec ) + 0.001*(end.tv_usec-start.tv_usec);
    printf("time: %f ms\n", timeuse);

    gettimeofday(&start, NULL);
    while(!schedule_finished(s))
        for(auto id : c_id)
        {
            coroutine_resume(s, id);
        }
    gettimeofday(&end, NULL);
    timeuse = 1000 * ( end.tv_sec - start.tv_sec ) + 0.001*(end.tv_usec-start.tv_usec);
    printf("time: %f ms\n", timeuse);
    

}
int main()
{

    // context_test();
    // puts("----------------");
    // schedule_test();

    test_perfor();

    return 0;
}
