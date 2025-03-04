/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */
#include<stdio.h>
#include<common.h>
#include<sys/tarfs.h>
#include<sys/elf64.h>
#include<sys/kstring.h>

/* Test user mode */
extern void switch_to_user_mode(void*);

/*
 * Just an utility routine to see what's in tarfs. 
 */
void dump_tarfs_contents(void) 
{
	posix_header_ustar* ptr = (posix_header_ustar*)&_binary_tarfs_start;
	int size = 0, byte_size=0;
	while((ptr+2) < (posix_header_ustar*)&_binary_tarfs_end) {
		size = 0;
		byte_size = 0;
		/*
		kprintf("ptr = %p", ptr);
		kprintf("name = %s ", ptr->name);
		kprintf("mode = %s ", ptr->mode);
		kprintf("uid = %s ", ptr->uid);
		kprintf("gid = %s ", ptr->gid);
		kprintf("size = %s \n", ptr->size);
		kprintf("mtime = %s ", ptr->mtime);
		kprintf("checksum = %s ", ptr->checksum);
		kprintf("typeflag = %s ", ptr->typeflag);
		kprintf("linkname = %s ", ptr->linkname);
		kprintf("magic = %s", ptr->magic);
		kprintf("version = %s ", ptr->version);
		kprintf("uname = %s ", ptr->uname);
		kprintf("gname = %s ", ptr->gname);
		kprintf("devmajor = %s ", ptr->devmajor);
		kprintf("devminor = %s ", ptr->devminor);
		kprintf("prefix = %s ", ptr->prefix);
		kprintf("pad = %s", ptr->pad);
		*/
		// Read the ELF file header and contents, if the size is > 0
		if (tarfs_atoi(ptr->size,8) > 0) {
			Elf64_Ehdr* ehdr_ptr = (Elf64_Ehdr*)(ptr+1);
			if(ehdr_ptr->e_ident[1] == 'E' && ehdr_ptr->e_ident[2] == 'L' && ehdr_ptr->e_ident[3] == 'F'){
				if(kstrcmp("bin/hello", ptr->name)) {
					/*
					kprintf("################name = %s############\n", ptr->name);
					kprintf("num of pheaders = %d\n", ehdr_ptr->e_phnum);
					*/
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
						//_elf_segment(ehdr_ptr, phdr_ptr);
						phdr_ptr++;
					}
				}
			}
		}
		byte_size = tarfs_atoi(ptr->size,8) + sizeof(posix_header_ustar); // size in bytes
		size = byte_size/sizeof(posix_header_ustar);

		if (byte_size%sizeof(posix_header_ustar) != 0)
			size++;
		if (size < 0L) break; /* Not sure what's happening here!*/
		ptr = ptr + size;
	}
	switch_to_user_mode((void*)0x4000e8);
}
