#ifndef _UNISTD_H
#define _UNISTD_H

#include <common.h>

u32int sleep(u32int);
u64int fork(void);
u32int execvpe(char* path, char* argv[], char* envp[]);

#endif
