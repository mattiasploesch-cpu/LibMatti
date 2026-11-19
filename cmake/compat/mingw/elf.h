#pragma once

#include <stdint.h>

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Xword;
typedef uint32_t Elf64_Word;
typedef uint16_t Elf64_Half;

typedef struct
{
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    Elf64_Addr e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
    Elf64_Word sh_name;
    Elf64_Word sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr sh_addr;
    uint64_t sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word sh_link;
    Elf64_Word sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
} Elf64_Shdr;

typedef struct
{
    Elf64_Word st_name;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t st_shndx;
    Elf64_Addr st_value;
    Elf64_Xword st_size;
} Elf64_Sym;

typedef struct
{
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
    int64_t r_addend;
} Elf64_Rela;

#define EI_CLASS 4
#define ELFCLASS64 2
#define SHT_RELA 4
#define SHT_DYNSYM 11
#define R_X86_64_GLOB_DAT 6
#define R_X86_64_JUMP_SLOT 7
#define ELF64_R_SYM(value) ((Elf64_Word) ((value) >> 32))
#define ELF64_R_TYPE(value) ((Elf64_Word) ((value) & 0xffffffffu))
