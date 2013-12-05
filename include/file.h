/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#ifndef _FILE_H
#define _FILE_H

#include<common.h>

int closedir(int);
int close(int);
int read(char *, int,int);
int readdir(char *, int);
int open(char*);
int opendir(char*);
#endif
