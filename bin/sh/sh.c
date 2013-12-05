/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<file.h>
char buffer[100];
int main(int argc, char* argv[], char* envp[])
{
    u64int child_pid = 0;
    while(1) {
        printf("User@Sbunix $");
        scanf("%s", buffer);
        printf("\n");
        char** tokens = strtok(buffer);
        if(!isnullstring(tokens[0])) {
            child_pid = fork();
            if(child_pid==0){
                execvpe(tokens[0], tokens, NULL);
            } else {
                waitpid(child_pid);
            }
        }
        printf("\n");
    }
    return 0;
}
