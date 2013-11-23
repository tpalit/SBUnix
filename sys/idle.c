#include<common.h>
#include<sys/proc_mgr.h>
#include<sys/kmalloc.h>

void idle(void)
{
	while(1);
}

void start_idle_process(void)
{
	task_struct* idle_task = (task_struct*)kmalloc(sizeof(task_struct));
	create_kernel_process(idle_task, (u64int)idle);
}
