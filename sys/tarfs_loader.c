#include<stdio.h>
#include<common.h>
#include<sys/tarfs.h>
#include<sys/elf64.h>
#include<kstring.h>

/* Test user mode */
extern void switch_to_user_mode(void*);

u64int tarfs_atoi(char* s, u8int base)
{
	u64int val = 0;
	u8int i = 0;
	for(i=0;i<11;i++) {
		val = val*base+s[i]-'0';
		//		kprintf("s[i] = %c\n", s[i]);
		//		kprintf("val = %d\n", val);
	}
	return val;
}

void funk()
{
	//	asm("pushq $0x4000e8\n\t");
	switch_to_user_mode((void*)0x4000e8);
}

/*
 * Just an utility routine to see what's in tarfs. 
 */
void dump_tarfs_contents(void) 
{
	posix_header_ustar* ptr = (posix_header_ustar*)&_binary_tarfs_start;
	u64int size = 0, byte_size=0;
	while((ptr+2) < (posix_header_ustar*)&_binary_tarfs_end) {
		size = 0;
		byte_size = 0;
		// Read the ELF file header and contents, if the size is > 0
		if (tarfs_atoi(ptr->size,8) > 0) {
			Elf64_Ehdr* ehdr_ptr = (Elf64_Ehdr*)(ptr+1);
			if(ehdr_ptr->e_ident[1] == 'E' && ehdr_ptr->e_ident[2] == 'L' && ehdr_ptr->e_ident[3] == 'F'){
				if(kstrcmp("bin/hello", ptr->name)) {
					kprintf("################name = %s############\n", ptr->name);
					kprintf("num of pheaders = %d\n", ehdr_ptr->e_phnum);
					int headerCount = ehdr_ptr->e_phnum;
					Elf64_Phdr* phdr_ptr = (Elf64_Phdr*)(((u64int)ehdr_ptr)+ehdr_ptr->e_phoff);
					for (;headerCount>0;headerCount--) {
						/*
						kprintf("Program header type = %x\n", phdr_ptr->p_type);
						kprintf("Program offset = %x\n", phdr_ptr->p_offset);
						kprintf("Program virtual address = %x\n", phdr_ptr->p_vaddr);
						kprintf("Program flags = %x\n", phdr_ptr->p_flags);
						kprintf("Program size in memory = %x\n", phdr_ptr->p_memsz);
					        */
						load_elf_segment(ehdr_ptr, phdr_ptr);
						phdr_ptr++;
					}
				}
			}
		}
		byte_size = tarfs_atoi(ptr->size,8) + sizeof(posix_header_ustar); // size in bytes
		size = byte_size/sizeof(posix_header_ustar);
		if (byte_size%sizeof(posix_header_ustar) != 0)
			size++;
		ptr = ptr + size;
		kprintf("hey!\n");
	}
	//	funk();
}
