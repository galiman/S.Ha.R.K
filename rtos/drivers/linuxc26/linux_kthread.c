#include <kernel/func.h>

extern int exec_shadow;

PID kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
	PID p;
	SOFT_TASK_MODEL m;

        soft_task_default_model(m);
        soft_task_def_arg(m,(void*)arg);
        soft_task_def_met(m, 5000);
        soft_task_def_wcet(m, 5000);
        soft_task_def_period(m, 25000);
        soft_task_def_ctrl_jet(m);
        soft_task_def_aperiodic(m);
        soft_task_def_usemath(m);

        p= task_create("linux26", (void*)fn, &m, NULL);
        if (p >0) task_activate(p);

	return p;
}

int kill_proc(pid_t pid, int sig, int priv)
{
	task_kill(pid);

	return 0;
}

void schedule(void) 
{
	task_endcycle();
}
