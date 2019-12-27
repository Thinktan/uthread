
/**
 * @file uthread.h
 * @author thinktan
 * @brief coroutine library for c++
 * @date 2019-12-15
 */

#include "uthread.h"

static void uthread_body(schedule_t *schedule)
{
    int id = schedule->running_index;

    if (id != -1)
    {
        uthread_t *t = &(schedule->uthreads[id]);
        t->state = RUNNING;
        t->func(t->arg); // running UThreadFunc
        t->state = FREE;
        schedule->running_index = -1;
    }
}

int uthread_create(schedule_t &schedule, UThreadFunc func, void *arg)
{
    int id = 0;
    for (id = 0; id < schedule.max_index; id++)
    {
        if (schedule.uthreads[id].state == FREE)
        {
            break;
        }
    }

    if ( id == schedule.max_index )
    {
        schedule.max_index++;
    }

    if ( schedule.max_index >= MAX_UTHREAD_SIZE )
    {
        return -1;
    }

    uthread_t *t = &(schedule.uthreads[id]);
    t->state = RUNNING;
    t->func = func;
    t->arg = arg;

    getcontext(&(t->ctx));

    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = DEFAULT_STACK_SIZE;
    t->ctx.uc_stack.ss_flags = 0;
    t->ctx.uc_link = &(schedule.main);

    schedule.running_index = id;

    makecontext(&(t->ctx), (void (*)(void))(uthread_body), 1, &schedule);
    swapcontext(&(schedule.main), &(t->ctx));

    return id;
}

// call by coroutine
void uthread_yield(schedule_t &schedule)
{
    if ( schedule.running_index != -1 )
    {
        uthread_t *t = &(schedule.uthreads[schedule.running_index]);
        t->state = SUSPEND;
        schedule.running_index = -1;

        swapcontext(&(t->ctx), &(schedule.main));
    }
}

void uthread_resume(schedule_t &schedule, int id)
{
    if (id < 0 || id >= schedule.max_index)
    {
        return;
    }

    uthread_t *t = &(schedule.uthreads[id]);
    if (t->state == SUSPEND)
    {
        schedule.running_index = id;
        swapcontext(&(schedule.main), &(t->ctx));
    }
}

int schedule_finished(const schedule_t &schedule)
{
    if (schedule.running_index != -1)
    {
        return 0;
    }
    else
    {
        for (int i = 0; i < schedule.max_index; i++)
        {
            if (schedule.uthreads[i].state != FREE)
            {
                return 0;
            }
        }
    }

    return 1;
}

