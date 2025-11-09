// phdr.h - Program Header Parser

#ifndef PHDR_H
#define PHDR_H

#include <stdint.h>
#include <stddef.h>
#include "elf_parser.h"

// ============================================================================
// Program Header Types
// ============================================================================

typedef enum {
    PT_NULL = 0,
    PT_LOAD = 1,
    PT_DYNAMIC = 2,
    PT_INTERP = 3,
    PT_NOTE = 4,
    PT_SHLIB = 5,
    PT_PHDR = 6,
    PT_TLS = 7,
    PT_LOOS = 0x60000000,
    PT_GNU_EH_FRAME = 0x6474e550,
    PT_GNU_STACK = 0x6474e551,
    PT_GNU_RELRO = 0x6474e552,
    PT_HIOS = 0x6fffffff,
    PT_LOPROC = 0x70000000,
    PT_HIPROC = 0x7fffffff,
} phdr_type_t;

// ============================================================================
// Program Header Flags
// ============================================================================

typedef enum {
    PF_X = 1,
    PF_W = 2,
    PF_R = 4,
    PF_MASKOS = 0x0ff00000,
    PF_MASKPROC = 0xf0000000,
} phdr_flags_t;

// ============================================================================
// 64-bit Program Header Structure
// ============================================================================

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} phdr_t;

// ============================================================================
// Status codes
// ============================================================================

typedef enum {
    PHDR_OK = 0,
    PHDR_ERR_SIZE = 1,
    PHDR_ERR_TYPE = 2,
    PHDR_ERR_OFFSET = 3,
    PHDR_ERR_ALIGN = 4,
    PHDR_ERR_INVALID = 5,
} phdr_status_t;

// ============================================================================
// Constants
// ============================================================================

#define PHDR_SIZE_64 56

// ============================================================================
// Macros
// ============================================================================

#define PHDR_IS_LOADABLE(p) ((p)->p_type == PT_LOAD)
#define PHDR_IS_DYNAMIC(p) ((p)->p_type == PT_DYNAMIC)
#define PHDR_IS_READABLE(p) ((p)->p_flags & PF_R)
#define PHDR_IS_WRITABLE(p) ((p)->p_flags & PF_W)
#define PHDR_IS_EXECUTABLE(p) ((p)->p_flags & PF_X)

// ============================================================================
// Function declarations
// ============================================================================

int phdr_parse(const uint8_t *data, size_t size, phdr_t *out);

phdr_status_t phdr_parse_all(elf_buffer_t *buf, const elf_header_t *hdr, 
                             phdr_t **out_phdrs, size_t *out_count);

phdr_status_t phdr_validate(const phdr_t *phdr);

const char* phdr_type_to_string(uint32_t type);

void phdr_flags_to_string(uint32_t flags, char *out, size_t size);

// ============================================================================
// ASM optimized versions
// ============================================================================

extern int phdr_parse_fast(const uint8_t *data, size_t size, phdr_t *out);
extern uint32_t phdr_read_u32_le(const uint8_t *data);
extern uint32_t phdr_read_u32_be(const uint8_t *data);
extern uint64_t phdr_read_u64_le(const uint8_t *data);
extern uint64_t phdr_read_u64_be(const uint8_t *data);

#endif // PHDR_H
