/**
 * The ELF formats.
 */

#include<stdio.h>
#include<sys/vm_mgr.h>
#include<sys/elf64.h>
#include<common.h>
#include<sys/tarfs.h>
#include<sys/kstring.h>
#include<sys/kmalloc.h>

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

/**
 * Build a task_struct ready to run from an elf file.
 */
task_struct* make_process_from_elf(char* path)
{
	Elf64_Ehdr* ehdr = find_elf(path);
	task_struct* new_task = NULL;
	if(NULL != ehdr){
		/* Parse and load the segments */
		parse_load_elf_segments(ehdr);
		new_task = (task_struct*)kmalloc(sizeof(task_struct));
		create_new_process(new_task, (u64int)ehdr->e_entry);
	}
	return new_task;
}

/**
 * Find the Elf file specified by the path.
 */
Elf64_Ehdr* find_elf(char* path) 
{
	posix_header_ustar* ptr = (posix_header_ustar*)&_binary_tarfs_start;
	int size = 0, byte_size=0;
	while((ptr+2) < (posix_header_ustar*)&_binary_tarfs_end) {
		size = 0;
		byte_size = 0;
		kprintf("name = %s ", ptr->name);
		// Read the ELF file header and contents, if the size is > 0
		if (tarfs_atoi(ptr->size,8) > 0) {
			Elf64_Ehdr* ehdr_ptr = (Elf64_Ehdr*)(ptr+1);
			if(ehdr_ptr->e_ident[1] == 'E' && ehdr_ptr->e_ident[2] == 'L' && ehdr_ptr->e_ident[3] == 'F'){
				if(kstrcmp(path, ptr->name)) {
					return ehdr_ptr;
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
	return NULL;
}

/**
 * Load all program headers of an ELF file into memory.
 */
void parse_load_elf_segments(Elf64_Ehdr* elf64_ehdr_ptr) 
{
	int headerCount = elf64_ehdr_ptr->e_phnum;
	Elf64_Phdr* phdr_ptr = (Elf64_Phdr*)(((u64int)elf64_ehdr_ptr)+elf64_ehdr_ptr->e_phoff);
	for (;headerCount>0;headerCount--) {
		/*
		  kprintf("Program header type = %x\n", phdr_ptr->p_type);
		  kprintf("Program offset = %x\n", phdr_ptr->p_offset);
		  kprintf("Program virtual address = %x\n", phdr_ptr->p_vaddr);
		  kprintf("Program flags = %x\n", phdr_ptr->p_flags);
		  kprintf("Program size in memory = %x\n", phdr_ptr->p_memsz);
		*/
		load_elf_segment(elf64_ehdr_ptr, phdr_ptr);
		phdr_ptr++;
	}	
}

/**
 * Load the Elf segment defined by the prog header into memory.
 */
void load_elf_segment(Elf64_Ehdr* elf64_ehdr_ptr, Elf64_Phdr* elf64_phdr_ptr)
{
	/* First map the region into memory */
	kmmap((void*)elf64_phdr_ptr->p_vaddr, elf64_phdr_ptr->p_memsz, 0,0,0,0);
	/* Then get copy the region into memory using memcpy */
	kmemcpy((void*)elf64_phdr_ptr->p_vaddr,
		(void*)((u64int)elf64_ehdr_ptr)+elf64_phdr_ptr->p_offset, 
		elf64_phdr_ptr->p_memsz);
}
