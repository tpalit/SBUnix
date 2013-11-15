/**
 * The ELF formats.
 */

#include<stdio.h>
#include<kstring.h>
#include<sys/vm_mgr.h>
#include<sys/elf64.h>

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
