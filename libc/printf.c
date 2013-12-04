#include<syscall.h>
#include<stdlib.h>
#include<common.h>
#include<stdarg.h>
#include<stdio.h>

static char char_buffer[1024]; /* Temporary block to use. */
static char final_buffer[1024]; /* This will be sent to the kernel to be printed */
static int final_buffer_indx = 0x0;

int printf1(const char *format, ...) {
	register volatile u64int ret_val = 0;
	/* Store the pointer in the rdx register. */
	__asm__ __volatile__("movq %[s], %%rdx\n\t"
			     :
			     :[s]"m"(format));
	/* Store the system call index in the rax register. */
	__asm__ __volatile__("movq $0, %rax\n\t");
	__asm__("int $0x80\n\t");
	/* The return value is also in rax register. */
	__asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
	return ret_val;
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



void puts(const char* str)
{
	while(*str != '\0'){
		putchar(*str++);
	}
}

void putchar(const char c)
{
	final_buffer[final_buffer_indx++] = c;
}

/*
 * This function invokes the itoa() function
 * to get a string representation of the integer.
 */
void putint(s32int n, int base)
{
	char *str = char_buffer;
	itoa(n, str, base);
	puts(str);
	/*
	while(*(str+i) != '\0') {
		putchar(*(str+i));
		i++;
	}
	*/
}

/*
 * This function invokes the ltoa() function
 * to get a string representation of a pointer.
 */
void putptr(u64int n)
{
	char *str = char_buffer;
	ltoa(n, str);
	/*
	while(*(str+i) != '\0') {
		putchar(*(str+i));
		i++;
	}
	*/
	puts(str);
}



int printf(const char *format, ...)
{
	int i=0;
	char ch;
	va_list arguments;
	va_start (arguments, format);
	final_buffer_indx = 0;
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

	final_buffer[final_buffer_indx++] = 0;
	u64int buff_addr = (u64int)&final_buffer;
	register volatile u64int ret_val = 0;
	__asm__ __volatile__("movq %[s], %%rdi\n\t"
			     :
			     :[s]"m"(buff_addr));
	__asm__ __volatile__("movq $0, %rax\n\t");
	__asm__("int $0x80\n\t");
	__asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
	final_buffer_indx = 0;
	return ret_val;
}

