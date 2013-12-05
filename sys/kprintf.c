/*
 * This file contains all routines required for the kernel printf.
 *
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 *
 */

#include<sys/kstdio.h>
#include<stdarg.h>
#include<common.h>

char screen_pos_x, screen_pos_y;

void putchar(const char c);
void putint(s32int, int);
void putptr(u64int);
void itoa(s32int, char*, int);
void ltoa(u64int, char*);
void update_cursor();
int kprintf(const char *format, ...);
void scroll_screen_up();
void reset_terminal_cursor();

static char char_buffer[1024]; /* Temporary block to use. */
static u8int video_back_buffer[4096]; /* The double buffer. */

u64int VIDEO_MEM_START = 0xB8000;
/*
 * This is the kernel's printf function.
 * It can handle basic formatting like %d, %c, %s. 
 * Internally, it invokes the putchar, putint and puts
 * functions.
 */
int kprintf(const char *format, ...)
{
	int i=0;
	char ch;
	va_list arguments;
	va_start (arguments, format);
	while((ch = *(format+i)) != '\0') {
		if(ch == '%') {
			i++;
			ch = *(format+i);
			switch(ch) {
			case 'c':
				putchar(va_arg(arguments, int));
				break;
		        case 'd':
				putint(va_arg(arguments, s32int), 10);
				break;
			case 'x':
				putint(va_arg(arguments, s32int), 16);
				break;
		        case 's':
				puts(va_arg(arguments, char*));
				break;
			case 'p':
				putptr(va_arg(arguments, u64int));
				break;
		        default:
				putchar(va_arg(arguments, int));
				break;
			}
		} else {
			putchar(ch);
		}
		i++;
	}
	va_end(arguments);
	return 0;
}

/*
 * This function invokes the itoa() function
 * to get a string representation of the integer.
 */
void putint(s32int n, int base)
{
	int i = 0;
	char *str = char_buffer;
	itoa(n, str, base);
	while(*(str+i) != '\0') {
		putchar(*(str+i));
		i++;
	}
}

/*
 * This function invokes the ltoa() function
 * to get a string representation of a pointer.
 */
void putptr(u64int n)
{
	int i = 0;
	char *str = char_buffer;
	ltoa(n, str);
	while(*(str+i) != '\0') {
		putchar(*(str+i));
		i++;
	}
}

void reverse(char *str, int length)
{
	int i=0, j = length-1;
	char tmp;
	while(i<j) {
		tmp = str[i];
		str[i] = str[j];
		str[j] = tmp;
		i++;
		j--;
	}
}

/*
 * This function will convert an unsigned 64 bit
 * pointer into string.
 */
void ltoa(u64int n, char* str)
{
	int i = 0;
   	while(n>0){
		if (n%16 < 10){
			str[i++] = '0'+n%16;
		} else {
			str[i++] = 65+(n%16-10);
		}
		n/=16;
	}	
	//	str[i++] = '0';
	str[i++] = 'x';
	str[i++] = '0';
	str[i] = '\0';
	reverse(str, i);
	return;
}

/*
 * This function will convert a 32 bit integer
 * into string.
 */
void itoa(s32int n, char* str, int base)
{
	int i = 0;
	int sign = n<0?1:0;
	if (n<0) {
		str[i++] = '-';
		n=-n; 
	} else if (n == 0) {
		str[i++] = '0';
	}
	/* We'll handle only base 10 and 16 */
   	while(n>0){
		if (base == 10){
			str[i++] = '0'+n%base;
		} else if (base == 16){
			if (n%base <10){
				str[i++] = '0'+n%base;
			} else {
				str[i++] = 65+(n%base-10);
			}
		}
		n/=base;
	}
	if (base == 16) {
		str[i++] = '0';
		str[i++] = 'x';
		str[i++] = '0';
	}	
	str[i] = '\0';
	reverse(str+sign, i - sign);
	return;
}

void putchar(const char c)
{
	u64int mem_loc = ((screen_pos_x*80)+screen_pos_y)*2+VIDEO_MEM_START;
	volatile char *video = (volatile char*)mem_loc;
	*video++ = c;
	*video++ = 0x07;
	if(c == '\n'){
		screen_pos_y=0;
		screen_pos_x++;
	} else if(c == '\t'){
		if(screen_pos_y+6 >= 80){
			screen_pos_y = 0;
			screen_pos_x++;
		} else {
			screen_pos_y+=6;
		}
	} else{
		if (screen_pos_y<80){
			screen_pos_y++;
		} else {
			screen_pos_y=0;
			screen_pos_x++;
		}
		if (screen_pos_x > 23){ 
			scroll_screen_up();
		}
	}
	update_cursor();
}

void puts(const char* str)
{
	while(*str != '\0'){
		putchar(*str++);
	}
}

void reset_terminal_cursor()
{
	screen_pos_x = 0;
	screen_pos_y = 0;
	update_cursor();
}

/*
 * This function will write to the CRT controller registers
 * and update the cursor.
 * The CRT controller has two registers - Index Register and
 * the Data Register which are mapped to memory locations 
 * 0x3D5 and 0x3D4.
 * The index offsets for setting the high and low bytes of the 
 * cursor location are 0xE and 0xF respectively. These should
 * be written to the Index Registers before writing the actual
 * locations to the Data Registers.
 */
void update_cursor()
{
	int cursor_loc = screen_pos_y + screen_pos_x*80;
	u8int low_byte = cursor_loc & 0xFF;
	u8int hi_byte = (cursor_loc >> 8) & 0xFF;

	/* Write the low byte */
	outb(0x3D4, 0x0f);
	outb(0x3D5, low_byte);
	/* Write the higher byte */
	outb(0x3D4, 0x0e);
	outb(0x3D5, hi_byte);
}


void scroll_screen_up()
{
	volatile char *new_screen = (volatile char*)VIDEO_MEM_START;
	volatile char *old_screen = (volatile char*)VIDEO_MEM_START+0xA0;
	int i = 0;
	/* Clear the old buffer */
	for(i=0; i<4096; i++) {
		video_back_buffer[i] = 0;
	}
	/* Copy a scrolled up screen */
	for(i=0;i<80*23*2;i++){
		video_back_buffer[i] = *(old_screen+i);
	}
	/* Copy back to the video memory */
	for(i=0;i<80*24*2;i++){
		*(new_screen+i) = video_back_buffer[i];
	}
	screen_pos_x--;
	update_cursor();
}

void clear_terminal()
{
	int i,j;
	for(i=0; i<=80; i++){
		for(j=0; j<=25; j++){
			putchar(' ');
		}
	}
	reset_terminal_cursor();
}

void do_bkspace(void)
{
	if(screen_pos_y > 0) {
		screen_pos_y--;
		putchar(' ');
		screen_pos_y--;
	}
}
