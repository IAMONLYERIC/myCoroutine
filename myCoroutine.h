/**
* @file  myCoroutine.h
* @author gzy
* @version 0.1
* @brief   :A asymmetric coroutine library for C++
*/

#ifndef MY_coroutine_H
#define MY_coroutine_H

#ifdef __APPLE__
#define _XOPEN_SOURCE
#endif 

#include <ucontext.h>
#include <vector>

#define DEFAULT_STACK_SZIE (1024*128) // 默认128K大小的栈空间
#define MAX_UTHREAD_SIZE   10240 // 最多允许的协程数量

// 协程状态
enum ThreadState{FREE,RUNNABLE,RUNNING,SUSPEND};

// 协程调度器
struct schedule_t;

// typedef void (*Fun)(void *arg);
using Fun = void(*) (void* arg);

// 协程的结构体

struct coroutine_t
{
    // 该协程的上下文
    ucontext_t ctx;
    // 协程要执行的函数
    Fun func;
    // func的参数
    void *arg;
    // 协程的状态
    enum ThreadState state;
    // 协程的栈空间
    char stack[DEFAULT_STACK_SZIE];
};

// 协程调度器
struct schedule_t
{
    // 主函数的上下文
    ucontext_t main;
    // 指向当前正在执行的协程的编号
    int running_thread;
    // 所有协程的数组
    coroutine_t *threads;
    // 曾经使用到的最大的index + 1
    int max_index; 

    schedule_t():running_thread(-1), max_index(0) {
        threads = new coroutine_t[MAX_UTHREAD_SIZE];

        // 初始化协程数组里面每个协程位置的状态都是FREE
        for (int i = 0; i < MAX_UTHREAD_SIZE; i++) {
            threads[i].state = FREE;
        }
    }
    
    ~schedule_t() {
        delete [] threads;
    }
};

/*help the thread running in the schedule*/
static void coroutine_body(schedule_t *ps);

// 创建协程
int  coroutine_create(schedule_t &schedule,Fun func,void *arg);

// 挂起当前正在运行的协程，返回主函数
void coroutine_yield(schedule_t &schedule);

// 恢复或运行第id个协程
void coroutine_resume(schedule_t &schedule,int id);

// 检测协程是否全部运行完毕
int schedule_finished(const schedule_t &schedule);

#endif
