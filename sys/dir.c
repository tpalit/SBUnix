/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<stdio.h>
#include<sys/vm_mgr.h>
#include<sys/pm_mgr.h>
#include<sys/elf64.h>
#include<common.h>
#include<sys/tarfs.h>
#include<sys/kstring.h>
#include<sys/kmalloc.h>
#include<sys/dir.h>

file_des * deshead =NULL;
dir_ptr * dirdeshead=NULL;

int find_dir(char* path){
    posix_header_ustar* ptr = (posix_header_ustar*)&_binary_tarfs_start;
    int size = 0, byte_size=0;
    while((ptr+2) < (posix_header_ustar*)&_binary_tarfs_end) {
        size = 0;
        byte_size = 0;
        if (tarfs_atoi(ptr->size,8) > 0) {

            if(kstrcmpsz(path, ptr->name)) {
               
               return add_dird(path,ptr);
            }
        }
        byte_size = tarfs_atoi(ptr->size,8) + sizeof(posix_header_ustar); // size in bytes
        size = byte_size/sizeof(posix_header_ustar);
        if (byte_size%sizeof(posix_header_ustar) != 0)
            size++;
        if (size < 0l) break; /* not sure what's happening here!*/
        ptr = ptr + size;
    }
    return 0;
}
int find_file(char* path){
    posix_header_ustar* ptr = (posix_header_ustar*)&_binary_tarfs_start;
    int size = 0, byte_size=0,des_number=0;
    while((ptr+2) < (posix_header_ustar*)&_binary_tarfs_end) {
        size = 0;
        byte_size = 0;
        if (tarfs_atoi(ptr->size,8) > 0) {

            if(kstrcmp(path, ptr->name)) {
                des_number=add_des(ptr->name,(char*)(ptr+1));
                return des_number;
            }
        }
        byte_size = tarfs_atoi(ptr->size,8) + sizeof(posix_header_ustar); // size in bytes
        size = byte_size/sizeof(posix_header_ustar);
        if (byte_size%sizeof(posix_header_ustar) != 0)
            size++;
        if (size < 0l) break; /* not sure what's happening here!*/
        ptr = ptr + size;
    }
    return des_number;
}
int close_fd(int fd){
    file_des * temp = deshead;
    file_des * temp1 = NULL;
    //file_des * temp2 = NULL;
    while(temp!=NULL){
        if(temp->fd_number==fd){
            if(temp==deshead){
            deshead=NULL;
            //free(temp);
            return 0;
            }
            else{
            //temp2=temp;
            temp=temp->next;
            temp1->next=temp;
            //free temp2;
            return 0;
            }
        }
        temp1=temp;
        temp=temp->next;
    }
    return -1;
}

int close_dird(int dird){
    dir_ptr * temp = dirdeshead;
    dir_ptr * temp1 = NULL;
    //dir_ptr * temp2 = NULL;
    while(temp!=NULL){
        if(temp->dr_number==dird){
            if(temp==dirdeshead){
                dirdeshead=NULL;
                //free(temp);
                return 0;
            }
            else{
                //temp2=temp;
                temp=temp->next;
                temp1->next=temp;
                //free temp2;
                return 0;
            }
        }
        temp1=temp;
        temp=temp->next;
    }
    return -1;
}

char * read_ptr(int fd){
    file_des * temp =deshead;
    if(temp==NULL)
    {
        return 0;
    }
    while(temp!=NULL){
        if(temp->fd_number==fd){
            return temp->file_pointer;
        }
        temp=temp->next;
    }
    return NULL;
}
dir_ptr * read_dir(int dird){
    dir_ptr * temp =dirdeshead;
    if(temp==NULL)
    {
        return 0;
    }
    while(temp!=NULL){
        if(temp->dr_number==dird){
            return temp;
        }
        temp=temp->next;
    }
    return 0;
}
int add_dird(char* path,posix_header_ustar * ptr)
{
    int i;
    dir_ptr* tempptr = dirdeshead;
    dir_ptr* newdes =NULL;
    dir_ptr* temp = NULL;
    if(tempptr==NULL)
    {

        newdes = (dir_ptr*)kmalloc(sizeof(dir_ptr));
        newdes->dr_number=1;
        kstrcpy(newdes->dir_path,path);
        newdes->dir_entry=ptr;
        newdes->next=NULL;
        newdes->count=0;
        dirdeshead=newdes;
        return 1;
    }
    i=0;
    while(tempptr!=NULL){
        if(tempptr->dir_entry==ptr){
            tempptr->count=0;
            return tempptr->dr_number;
        }
        i++;
        temp=tempptr;
        tempptr=tempptr->next;
    }
    if(i==32){
        return 0;
    }
    newdes = (dir_ptr*)kmalloc(sizeof(dir_ptr));

    newdes->dr_number=temp->dr_number+1;
    kstrcpy(newdes->dir_path,path);
    newdes->dir_entry=ptr;
    newdes->next=NULL;
    newdes->count=0;
    temp->next=newdes;
    return newdes->dr_number;
}
int add_des(char* path,char * ptr)
{
    int i;
    file_des* tempptr = deshead;
    file_des* newdes =NULL;
    file_des* temp =NULL;
    if(tempptr==NULL)
    {

        newdes = (file_des*)kmalloc(sizeof(file_des));
        newdes->fd_number=1;
        kstrcpy(newdes->file_path,path);
        newdes->file_pointer=ptr;
        newdes->next=NULL;
        deshead=newdes;
        return 1;
    }
    i=0;
    while(tempptr!=NULL){
        if(tempptr->file_pointer==ptr){
            return tempptr->fd_number;
        }
        i++;
        temp=tempptr;
        tempptr=tempptr->next;
    }
    if(i==32){
        return 0;
    }
    newdes = (file_des*)kmalloc(sizeof(file_des));

    newdes->fd_number=temp->fd_number+1;
    kstrcpy(newdes->file_path,path);
    newdes->file_pointer=ptr;
    newdes->next=NULL;
    temp->next=newdes;
    return newdes->fd_number;
}
void clean_fd(){
    file_des * temp = deshead;
    int i;
    if(temp!=NULL){
        i=1;
        while(temp!=NULL){
            temp->fd_number=i;
            i++;
            temp=temp->next;

        }
    }
}
