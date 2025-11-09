#include "phdr.h"
#include "elf_header.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// ENDIANNESS HELPERS
// ============================================================================

static uint32_t read_u32_le(const uint8_t *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

static uint32_t read_u32_be(const uint8_t *data) {
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) | (uint32_t)data[3];
}

static uint64_t read_u64_le(const uint8_t *data) {
    return (uint64_t)data[0] | ((uint64_t)data[1] << 8) |
           ((uint64_t)data[2] << 16) | ((uint64_t)data[3] << 24) |
           ((uint64_t)data[4] << 32) | ((uint64_t)data[5] << 40) |
           ((uint64_t)data[6] << 48) | ((uint64_t)data[7] << 56);
}

static uint64_t read_u64_be(const uint8_t *data) {
    return ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) |
           ((uint64_t)data[2] << 40) | ((uint64_t)data[3] << 32) |
           ((uint64_t)data[4] << 24) | ((uint64_t)data[5] << 16) |
           ((uint64_t)data[6] << 8) | (uint64_t)data[7];
}

// ============================================================================
// SINGLE PROGRAM HEADER PARSER
// ============================================================================

int phdr_parse(const uint8_t *data, size_t size, phdr_t *out) {
    if (!data || !out || size < PHDR_SIZE_64) {
        return PHDR_ERR_SIZE;
    }

    uint32_t (*read_u32)(const uint8_t *) = read_u32_le;
    uint64_t (*read_u64)(const uint8_t *) = read_u64_le;

    out->p_type = read_u32(data + 0);
    out->p_flags = read_u32(data + 4);
    out->p_offset = read_u64(data + 8);
    out->p_vaddr = read_u64(data + 16);
    out->p_paddr = read_u64(data + 24);
    out->p_filesz = read_u64(data + 32);
    out->p_memsz = read_u64(data + 40);
    out->p_align = read_u64(data + 48);

    return PHDR_OK;
}

// ============================================================================
// VALIDATE PROGRAM HEADER
// ============================================================================

phdr_status_t phdr_validate(const phdr_t *phdr) {
    if (!phdr) {
        return PHDR_ERR_INVALID;
    }

    if (phdr->p_filesz > phdr->p_memsz) {
        return PHDR_ERR_SIZE;
    }

    if (phdr->p_type > PT_HIPROC) {
        return PHDR_ERR_TYPE;
    }

    return PHDR_OK;
}

// ============================================================================
// PARSE ALL PROGRAM HEADERS
// ============================================================================

phdr_status_t phdr_parse_all(elf_buffer_t *buf, const elf_header_t *hdr,
                             phdr_t **out_phdrs, size_t *out_count) {
    if (!buf || !buf->data || !hdr || !out_phdrs || !out_count) {
        return PHDR_ERR_INVALID;
    }

    size_t phdr_count = hdr->e_phnum;
    size_t phdr_offset = hdr->e_phoff;
    size_t phdr_entsize = hdr->e_phentsize;

    if (phdr_offset > buf->size) {
        return PHDR_ERR_OFFSET;
    }

    if (phdr_count > (buf->size / phdr_entsize)) {
        return PHDR_ERR_OFFSET;
    }

    if (phdr_offset + (phdr_count * phdr_entsize) > buf->size) {
        return PHDR_ERR_OFFSET;
    }

    if (phdr_entsize != PHDR_SIZE_64) {
        return PHDR_ERR_SIZE;
    }

    phdr_t *phdrs = (phdr_t *)malloc(phdr_count * sizeof(phdr_t));
    if (!phdrs) {
        return PHDR_ERR_INVALID;
    }

    for (size_t i = 0; i < phdr_count; i++) {
        size_t offset = phdr_offset + (i * phdr_entsize);
        
        if (offset + PHDR_SIZE_64 > buf->size) {
            free(phdrs);
            return PHDR_ERR_OFFSET;
        }
        
        int status = phdr_parse(buf->data + offset, buf->size - offset, &phdrs[i]);
        if (status != PHDR_OK) {
            free(phdrs);
            return (phdr_status_t)status;
        }

        phdr_status_t val_status = phdr_validate(&phdrs[i]);
        if (val_status != PHDR_OK) {
            free(phdrs);
            return val_status;
        }
    }

    *out_phdrs = phdrs;
    *out_count = phdr_count;

    return PHDR_OK;
}

// ============================================================================
// TYPE AND FLAGS TO STRING
// ============================================================================

const char* phdr_type_to_string(uint32_t type) {
    switch (type) {
        case PT_NULL: return "PT_NULL";
        case PT_LOAD: return "PT_LOAD";
        case PT_DYNAMIC: return "PT_DYNAMIC";
        case PT_INTERP: return "PT_INTERP";
        case PT_NOTE: return "PT_NOTE";
        case PT_SHLIB: return "PT_SHLIB";
        case PT_PHDR: return "PT_PHDR";
        case PT_TLS: return "PT_TLS";
        case PT_GNU_EH_FRAME: return "PT_GNU_EH_FRAME";
        case PT_GNU_STACK: return "PT_GNU_STACK";
        case PT_GNU_RELRO: return "PT_GNU_RELRO";
        default: return "PT_UNKNOWN";
    }
}

void phdr_flags_to_string(uint32_t flags, char *out, size_t size) {
    if (!out || size < 4) return;
    
    out[0] = (flags & PF_R) ? 'R' : '-';
    out[1] = (flags & PF_W) ? 'W' : '-';
    out[2] = (flags & PF_X) ? 'X' : '-';
    out[3] = '\0';
}

// ============================================================================
// ASM WEAK SYMBOLS (Fallback to C)
// ============================================================================

__attribute__((weak)) int phdr_parse_fast(const uint8_t *data, size_t size, phdr_t *out) {
    return phdr_parse(data, size, out);
}

__attribute__((weak)) uint32_t phdr_read_u32_le(const uint8_t *data) {
    return read_u32_le(data);
}

__attribute__((weak)) uint32_t phdr_read_u32_be(const uint8_t *data) {
    return read_u32_be(data);
}

__attribute__((weak)) uint64_t phdr_read_u64_le(const uint8_t *data) {
    return read_u64_le(data);
}

__attribute__((weak)) uint64_t phdr_read_u64_be(const uint8_t *data) {
    return read_u64_be(data);
}
