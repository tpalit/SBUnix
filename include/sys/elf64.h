/**
 * Header file for the ELF64 structures. 
 */

#include<common.h>
#include<sys/proc_mgr.h>

typedef u64int Elf64_Addr;
typedef u64int Elf64_Off;
typedef u16int Elf64_Half;
typedef u32int Elf64_Word;
typedef s32int Elf64_Sword;
typedef u64int Elf64_Xword;
typedef s64int Elf64_Sxword;

/* The ELF header */
typedef struct
{
	unsigned char e_ident[16]; /* ELF identification */
	Elf64_Half e_type; /* Object file type */
	Elf64_Half e_machine; /* Machine type */
	Elf64_Word e_version; /* Object file version */
	Elf64_Addr e_entry; /* Entry point address */
	Elf64_Off e_phoff; /* Program header offset */
	Elf64_Off e_shoff; /* Section header offset */
	Elf64_Word e_flags; /* Processor-specific flags */
	Elf64_Half e_ehsize; /* ELF header size */
	Elf64_Half e_phentsize; /* Size of program header entry */
	Elf64_Half e_phnum; /* Number of program header entries */
	Elf64_Half e_shentsize; /* Size of section header entry */
	Elf64_Half e_shnum; /* Number of section header entries */
	Elf64_Half e_shstrndx; /* Section name string table index */
} Elf64_Ehdr;

/* The ELF sections */
typedef struct
{
	Elf64_Word sh_name; /* Section name */
	Elf64_Word sh_type; /* Section type */
	Elf64_Xword sh_flags; /* Section attributes */
	Elf64_Addr sh_addr; /* Virtual address in memory */
	Elf64_Off sh_offset; /* Offset in file */
	Elf64_Xword sh_size; /* Size of section */
	Elf64_Word sh_link; /* Link to other section */
	Elf64_Word sh_info; /* Miscellaneous information */
	Elf64_Xword sh_addralign; /* Address alignment boundary */
	Elf64_Xword sh_entsize; /* Size of entries, if section has table */
} Elf64_Shdr;

/* The Symbol table */
typedef struct
{
	Elf64_Word st_name; /* Symbol name */
	unsigned char st_info; /* Type and Binding attributes */
	unsigned char st_other; /* Reserved */
	Elf64_Half st_shndx; /* Section table index */
	Elf64_Addr st_value; /* Symbol value */
	Elf64_Xword st_size; /* Size of object (e.g., common) */
} Elf64_Sym;


/* The structures for Relocations */
typedef struct
{
	Elf64_Addr r_offset; /* Address of reference */
	Elf64_Xword r_info; /* Symbol index and type of relocation */
} Elf64_Rel;

typedef struct
{
	Elf64_Addr r_offset; /* Address of reference */
	Elf64_Xword r_info; /* Symbol index and type of relocation */
	Elf64_Sxword r_addend; /* Constant part of expression */
} Elf64_Rela;

/* The Program Header table */
typedef struct
{
	Elf64_Word p_type; /* Type of segment */
	Elf64_Word p_flags; /* Segment attributes */
	Elf64_Off p_offset; /* Offset in file */
	Elf64_Addr p_vaddr; /* Virtual address in memory */
	Elf64_Addr p_paddr; /* Reserved */
	Elf64_Xword p_filesz; /* Size of segment in file */
	Elf64_Xword p_memsz; /* Size of segment in memory */
	Elf64_Xword p_align; /* Alignment of segment */
} Elf64_Phdr;

u64int tarfs_atoi(char*, u8int);

void load_elf_segment(Elf64_Ehdr* , Elf64_Phdr*, task_struct*);

Elf64_Ehdr* find_elf(char*);

void parse_load_elf_segments(Elf64_Ehdr*, task_struct*);

task_struct* make_process_from_elf(char*);
