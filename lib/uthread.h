
/**
 * @file uthread.h
 * @author thinktan
 * @brief coroutine library for c++
 * @date 2019-12-15
 */

#ifndef _UTHREAD_H_INCLUDED_
#define _UTHREAD_H_INCLUDED_

#include <ucontext.h>

#define DEFAULT_STACK_SIZE (1024*128)
#define MAX_UTHREAD_SIZE 1024

enum enUThreadState
{
    FREE,
    RUNNABLE,
    RUNNING,
    SUSPEND
};

typedef void (*UThreadFunc)(void *arg);

struct uthread_t
{
    ucontext_t ctx;
    UThreadFunc func;
    void *arg;
    enum enUThreadState state;
    char stack[DEFAULT_STACK_SIZE];
};

struct schedule_t
{
    ucontext_t main;
    int running_index;
    int max_index;
    uthread_t *uthreads;

    schedule_t():running_index(-1), max_index(0)
    {
        uthreads = new uthread_t[MAX_UTHREAD_SIZE];
        for (int i = 0; i < MAX_UTHREAD_SIZE; i++)
        {
            uthreads[i].state = FREE;
        }
    }

    ~schedule_t()
    {
        delete[] uthreads;
    }
};

// state machine
// FREE -(create)-> RUNNABLE -(body)-> RUNNING
// -(yield)-> SUSPEND -(resume)-> RUNNING -(body)-> FREE

int uthread_create(schedule_t &schedule, UThreadFunc func, void *arg);

void uthread_yield(schedule_t &schedule);

void uthread_resume(schedule_t &schedule, int id);

int schedule_finished(const schedule_t &schedule);


#endif //_UTHREAD_H_INCLUDED_
