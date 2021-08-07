/**
* @file  myCoroutine.cpp
* @author gzy
* @version 0.1
* @brief   :A asymmetric coroutine library for C++
*/

#ifndef MY_coroutine_CPP
#define MY_coroutine_CPP


#include "myCoroutine.h"
//#include <stdio.h>
#include <iostream>

// 恢复第id个协程运行
void coroutine_resume(schedule_t &schedule , int id)
{
    if(id < 0 || id >= schedule.max_index){
        return;
    }

    coroutine_t *t = &(schedule.threads[id]);

    // 如果其处于暂停状态，切换其运行状态。
    if (t->state == SUSPEND) {
        t->state = RUNNING;
        schedule.running_thread = id;
        swapcontext(&(schedule.main),&(t->ctx));
    }else if(t->state == RUNNABLE){
        // 如果是新创建的协程，先指定栈空间
        getcontext(&(t->ctx));
        t->ctx.uc_stack.ss_sp = t->stack;
        t->ctx.uc_stack.ss_size = DEFAULT_STACK_SZIE;
        t->ctx.uc_stack.ss_flags = 0;
        t->ctx.uc_link = &(schedule.main); // 后续执行main
        schedule.running_thread = id; // 设置当前的运行协程为该协程
        // 设置新协程要执行的函数
        // 将uthread_body 转成 void (*)(void)类型
        makecontext(&(t->ctx),(void (*)(void))(coroutine_body),1,&schedule);
        // 保存当前上下文到main,开始执行协程.
        t->state = RUNNING;
        swapcontext(&(schedule.main), &(t->ctx));
        // 由于当前协程里面调用了yield让出了当前cpu，所以或继续执行这里 返回到main函数
        std::cout << id <<": resume" << std::endl;
    }
}

// 挂起当前协程，返回主函数
void coroutine_yield(schedule_t &schedule)
{
    if(schedule.running_thread != -1 ){
        // 获取正在运行的协程
        coroutine_t *t = &(schedule.threads[schedule.running_thread]);
        // 挂起状态
        t->state = SUSPEND;
        schedule.running_thread = -1;

        swapcontext(&(t->ctx),&(schedule.main));
    }
}

// 获取正在运行的协程并执行他的入口函数，执行完毕后重新设置为FREE
void coroutine_body(schedule_t *ps)
{
    int id = ps->running_thread;

    if(id != -1){
        coroutine_t *t = &(ps->threads[id]);
        
        t->func(t->arg);
        std::cout << id <<  ": t->func over " << std::endl;
        // 协程跑完后，当前的位置重新设置为FREE
        t->state = FREE;
        ps->running_thread = -1;
    }
}

// 创建协程
int coroutine_create(schedule_t &schedule,Fun func,void *arg)
{
    int id = 0;
    // 寻找到一个空位置 用来存储新创建的协程
    for(id = 0; id < schedule.max_index; ++id ){
        if(schedule.threads[id].state == FREE){
            break;
        }
    }
    
    // 维护最大使用的协程id
    if (id == schedule.max_index) {
        schedule.max_index++;
    }

    coroutine_t *t = &(schedule.threads[id]);

    t->state = RUNNABLE;
    t->func = func;
    t->arg = arg;

    return id;
}

// 判断schedule中所有的协程是否都执行完毕，是返回1，否则返回0.
// 注意：如果有协程处于挂起状态时算作未全部执行完毕，返回0.
int schedule_finished(const schedule_t &schedule)
{
    if (schedule.running_thread != -1){
        return 0;
    }else{
        for(int i = 0; i < schedule.max_index; ++i){
            if(schedule.threads[i].state != FREE){
                return 0;
            }
        }
    }

    return 1;
}

#endif
