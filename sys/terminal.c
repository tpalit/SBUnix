/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */
#include<sys/kstdio.h>
#include<sys/elf64.h>
#include<sys/kstring.h>
#include<sys/proc_mgr.h>

extern task_struct* BLOCKED_ON_READ;
extern char* BLOCKED_BUFFER;

unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b',
    '\t',
    'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`',   0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/',   0,
    '*',
    0,
    ' ',
    0,
    0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,
    0,
    0,
    0,
    0,   0,   0,
    0,
    0,
    0,
};

unsigned char kbdus1[128] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '_', '+', '\b',
    '\t',
    'Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '\'', '~',   0,
    '\\', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M', '<', '>', '?',   0,
    '*',
    0,
    ' ',
    0,
    0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,
    0,
    0,
    0,
    0,   0,   0,
    0,
    0,
    0,
};
#define BUFF_SIZE 100
char buff[BUFF_SIZE]={NULL};
unsigned int buffp=0;

void buffer (unsigned int op, unsigned char key)
{
	if(op==0&&buffp!=0){
		buff[--buffp]=NULL;
	}
	else if(op==1){
		buff[buffp++]=key;
	}
	else if(op==3){
		if(buff[0]!=NULL){
			kstrcpy(STDIN,buff);
			buffp = 0;
			writebuff=1;
			/* Wake up the process waiting on the read */
			if (BLOCKED_ON_READ != NULL){
				u64int old_cr3 = 0L;
				__asm__ __volatile__(
						     "movq %%cr3, %0\n\t"
						     :"=r"(old_cr3));
				__asm__ __volatile__(
						     "movq %0, %%cr3\n\t"
						     ::"r"(BLOCKED_ON_READ->cr3_register));
				kstrcpy(BLOCKED_BUFFER, buff);
				__asm__ __volatile__(
						     "movq %0, %%cr3\n\t"
						     ::"r"(old_cr3));	
				BLOCKED_ON_READ->waiting = 0;
				BLOCKED_ON_READ = NULL;
				BLOCKED_BUFFER = NULL;
			}

		}
		int i;
		for(i = 0; i<BUFF_SIZE; i++) {
			buff[i]=NULL; // clearing the buffer
		}
	}
}

void terminal(unsigned char scancode)
{
    //	unsigned char* temp;
    //	temp = get_video();
    int func=0;
    if (scancode & 0x80) {
        //currently do nothing on key release
    }
    else {
        if(scancode == 42){ //Shift is pressed
            func = 1;
        }else if(scancode==29){ //ctrl is pressed
            func=2;
        }else if(scancode==56){ // alt is pressed
            func=3;
        }else if(scancode==0xe){
            do_bkspace();
            buffer(0,0);
        }else if(scancode==0x1c){
            buffer(3,0);
        }
        else{
            if(func ==1){
                if (buffp < BUFF_SIZE){
                    kprintf("%c",kbdus1[scancode]);
                    buffer(1,kbdus1[scancode]);
                    func=0;
                }				
            }
            else{
                if (buffp < BUFF_SIZE){
                    kprintf ("%c",kbdus[scancode]);
                    buffer(1,kbdus[scancode]);
                }
            }
        }
    }

}



