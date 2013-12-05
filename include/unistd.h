/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#ifndef _UNISTD_H
#define _UNISTD_H

#include <common.h>

u64int sleep(u32int);
u64int fork(void);
u64int execvpe(char* path, char* argv[], char* envp[]);
u64int wait(void);
u64int waitpid(u64int);
u64int getpid(void);
int getprocinfo(char *);
#endif
