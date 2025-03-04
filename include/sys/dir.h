/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#ifndef _DIR_H
#define _DIR_H

/**
 * Header file for the dir structures. 
 */

#include<sys/pm_mgr.h>
#include<sys/elf64.h>
#include<common.h>
#include<sys/tarfs.h>
#include<sys/kstring.h>
#include<sys/kmalloc.h>
struct file_des{
    int fd_number;
    char file_path[30];
    char * file_pointer;
   struct  file_des *next;
};
typedef struct file_des file_des;
struct dir_ptr{
    int dr_number;
    posix_header_ustar* dir_entry;
    char dir_path[30];
    struct dir_ptr *next;
    int count;
};
typedef struct dir_ptr dir_ptr;
int find_dir(char*);
void clean_fd();
int add_des(char*,char *);
char * read_ptr(int);
int find_file(char*);
int close_fd(int);
dir_ptr * read_dir(int);
int add_dird(char* ,posix_header_ustar *);
int close_dird(int);
#endif
