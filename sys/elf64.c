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
		new_task = (task_struct*)kmalloc(sizeof(task_struct));
		create_kernel_process(new_task, (u64int)ehdr->e_entry);
		/* Parse and load the segments */
       		parse_load_elf_segments(ehdr, new_task);
		/* Give the task some heap memory */
		vm_struct* heap_vma = (vm_struct*)kmalloc(sizeof(vm_struct));
		heap_vma->vm_start = 0x5000e8;
		heap_vma->vm_end = 0x5000e8;
		kstrcpy(heap_vma->vm_type, "HEAP");
		heap_vma->vm_next = NULL;
		/* Attach this vma to the list given in the task_struct */
		if (new_task->vm_head == NULL){
			new_task->vm_head = heap_vma;
		} else {
			vm_struct* vma_ptr = new_task->vm_head;
			while(vma_ptr->vm_next != NULL){
				vma_ptr = vma_ptr->vm_next;
			}
			vma_ptr->vm_next = heap_vma;
		}
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
void parse_load_elf_segments(Elf64_Ehdr* elf64_ehdr_ptr, task_struct* task_ptr) 
{
	int headerCount = elf64_ehdr_ptr->e_phnum;
	Elf64_Phdr* phdr_ptr = (Elf64_Phdr*)(((u64int)elf64_ehdr_ptr)+elf64_ehdr_ptr->e_phoff);
	for (;headerCount>0;headerCount--) {
	       	load_elf_segment(elf64_ehdr_ptr, phdr_ptr, task_ptr);
		phdr_ptr++;
	}	
}

/**
 * Load the Elf segment defined by the prog header into memory.
 */
void load_elf_segment(Elf64_Ehdr* elf64_ehdr_ptr, Elf64_Phdr* elf64_phdr_ptr, task_struct* task_ptr)
{
	u64int old_cr3 = 0L;
	/* First switch the address space to that of the process associated with this Elf file. */
	__asm__ __volatile__(
			     "movq %%cr3, %0\n\t"
			     :"=r"(old_cr3));
	__asm__ __volatile__(
			     "movq %0, %%cr3\n\t"
			     ::"r"(task_ptr->cr3_register));
	/* First map the region into memory */
	kmmap((void*)elf64_phdr_ptr->p_vaddr, elf64_phdr_ptr->p_memsz, 0,0,0,0);
	/* Then get copy the region into memory using memcpy */
	kmemcpy((void*)elf64_phdr_ptr->p_vaddr,
		(void*)((u64int)elf64_ehdr_ptr)+elf64_phdr_ptr->p_offset, 
		elf64_phdr_ptr->p_memsz);
	/* Load back the old cr3 */
	__asm__ __volatile__(
			     "movq %0, %%cr3\n\t"
			     ::"r"(task_ptr->cr3_register));	
	/* Creae a VMA struct for this region */
	vm_struct* proc_vma = (vm_struct*)kmalloc(sizeof(vm_struct));
	proc_vma->vm_start = elf64_phdr_ptr->p_vaddr;
	proc_vma->vm_end = elf64_phdr_ptr->p_vaddr + elf64_phdr_ptr->p_memsz;
	if (elf64_phdr_ptr->p_flags == 0x06) {
		kstrcpy(proc_vma->vm_type, "DATA");
	} else if (elf64_phdr_ptr->p_flags == 0x05) {
		kstrcpy(proc_vma->vm_type, "CODE");
	} else {
		kstrcpy(proc_vma->vm_type, "OTHR");
	}
	proc_vma->vm_next = NULL;
	/* Attach this vma to the list */
	if (task_ptr->vm_head == NULL){
		task_ptr->vm_head = proc_vma;
	} else {
		vm_struct* vma_ptr = task_ptr->vm_head;
		while(vma_ptr->vm_next != NULL){
			vma_ptr = vma_ptr->vm_next;
		}
		vma_ptr->vm_next = proc_vma;
	}
}
