// elf_header.h - Definiciones de cabecera ELF

#ifndef ELF_HEADER_H
#define ELF_HEADER_H

#include <stdint.h>
#include <stddef.h>

// Constantes
#define ELF_HEADER_32_SIZE 52
#define ELF_HEADER_64_SIZE 64

// Tipos ELF
typedef enum {
    ET_NONE = 0,
    ET_REL = 1,      // Relocatable object
    ET_EXEC = 2,     // Executable
    ET_DYN = 3,      // Shared object
    ET_CORE = 4,
} elf_type_t;

// Máquinas soportadas
typedef enum {
    EM_NONE = 0,
    EM_386 = 3,
    EM_S390 = 22,
    EM_PPC = 20,
    EM_PPC64 = 21,
    EM_ARM = 40,
    EM_X86_64 = 62,
    EM_AARCH64 = 183,
} elf_machine_t;

// Clase ELF
typedef enum {
    ELFCLASS32 = 1,
    ELFCLASS64 = 2,
} elf_class_t;

// Codificación (endianness)
typedef enum {
    ELFDATA2LSB = 1,  // Little-endian
    ELFDATA2MSB = 2,  // Big-endian
} elf_encoding_t;

// Versión ELF
typedef enum {
    EV_NONE = 0,
    EV_CURRENT = 1,
} elf_version_t;

// OS/ABI
typedef enum {
    ELFOSABI_SYSV = 0,
    ELFOSABI_LINUX = 3,
} elf_osabi_t;

// Estructura de cabecera ELF (64-bit)
typedef struct {
    uint8_t e_ident[16];      // Identificación ELF
    uint16_t e_type;           // Tipo de archivo
    uint16_t e_machine;        // Máquina
    uint32_t e_version;        // Versión
    uint64_t e_entry;          // Entry point
    uint64_t e_phoff;          // Offset de Program Header Table
    uint64_t e_shoff;          // Offset de Section Header Table
    uint32_t e_flags;          // Flags específicos de máquina
    uint16_t e_ehsize;         // Tamaño de este header
    uint16_t e_phentsize;      // Tamaño de una entrada PHDR
    uint16_t e_phnum;          // Número de PHDRs
    uint16_t e_shentsize;      // Tamaño de una entrada SHDR
    uint16_t e_shnum;          // Número de SHDRs
    uint16_t e_shstrndx;       // Índice de string table
} elf_header_t;

// Status codes
typedef enum {
    ELF_HDR_OK = 0,
    ELF_HDR_ERR_MAGIC = 1,
    ELF_HDR_ERR_CLASS = 2,
    ELF_HDR_ERR_ENCODING = 3,
    ELF_HDR_ERR_VERSION = 4,
    ELF_HDR_ERR_TYPE = 5,
    ELF_HDR_ERR_MACHINE = 6,
    ELF_HDR_ERR_SIZE = 7,
} elf_hdr_status_t;

// Funciones C
int elf_header_parse(const uint8_t *data, size_t size, elf_header_t *out);
int elf_header_validate_magic(const uint8_t *data);
int elf_header_get_class(const uint8_t *data);
int elf_header_get_encoding(const uint8_t *data);
int elf_header_get_version(const uint8_t *data);
int elf_header_is_64bit(const uint8_t *data);
int elf_header_is_little_endian(const uint8_t *data);

// Funciones ASM (optimizadas)
extern int elf_header_parse_fast(const uint8_t *data, size_t size, elf_header_t *out);
extern uint64_t elf_read_u64_le(const uint8_t *data);
extern uint64_t elf_read_u64_be(const uint8_t *data);
extern uint32_t elf_read_u32_le(const uint8_t *data);
extern uint32_t elf_read_u32_be(const uint8_t *data);
extern uint16_t elf_read_u16_le(const uint8_t *data);
extern uint16_t elf_read_u16_be(const uint8_t *data);

#endif
