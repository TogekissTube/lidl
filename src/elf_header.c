#include <stdint.h>
#include <string.h>
#include "elf_parser.h"
#include "elf_header.h"

int elf_header_validate_magic(const uint8_t *data) {
    if (!data) return 0;
    return data[0] == 0x7F && data[1] == 'E' && data[2] == 'L' && data[3] == 'F';
}

int elf_header_get_class(const uint8_t *data) {
    if (!data) return -1;
    return (int)data[4];
}

int elf_header_get_encoding(const uint8_t *data) {
    if (!data) return -1;
    return (int)data[5];
}

int elf_header_get_version(const uint8_t *data) {
    if (!data) return -1;
    return (int)data[6];
}

int elf_header_is_64bit(const uint8_t *data) {
    return elf_header_get_class(data) == ELFCLASS64;
}

int elf_header_is_little_endian(const uint8_t *data) {
    return elf_header_get_encoding(data) == ELFDATA2LSB;
}

// ============================================================================
// CONVERSION ENDIANNESS (Fallback C)
// ============================================================================

static uint16_t read_u16_le(const uint8_t *data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint16_t read_u16_be(const uint8_t *data) {
    return ((uint16_t)data[0] << 8) | (uint16_t)data[1];
}

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
// PARSER PRINCIPAL
// ============================================================================

int elf_header_parse(const uint8_t *data, size_t size, elf_header_t *out) {
    if (!data || !out || size < ELF_HEADER_64_SIZE) {
        return ELF_HDR_ERR_SIZE;
    }

    if (data[0] != 0x7F || data[1] != 'E' || data[2] != 'L' || data[3] != 'F') {
        return ELF_HDR_ERR_MAGIC;
    }

    int class = elf_header_get_class(data);
    int encoding = elf_header_get_encoding(data);
    int version = elf_header_get_version(data);

    if (class != ELFCLASS64) {
        return ELF_HDR_ERR_CLASS;
    }

    if (version != EV_CURRENT) {
        return ELF_HDR_ERR_VERSION;
    }

    memcpy(out->e_ident, data, 16);

    uint16_t (*read_u16)(const uint8_t *) = (encoding == ELFDATA2LSB) ? read_u16_le : read_u16_be;
    uint32_t (*read_u32)(const uint8_t *) = (encoding == ELFDATA2LSB) ? read_u32_le : read_u32_be;
    uint64_t (*read_u64)(const uint8_t *) = (encoding == ELFDATA2LSB) ? read_u64_le : read_u64_be;

    out->e_type = read_u16(data + 16);
    out->e_machine = read_u16(data + 18);

    if (out->e_type != ET_REL && out->e_type != ET_EXEC && 
        out->e_type != ET_DYN && out->e_type != ET_CORE) {
        return ELF_HDR_ERR_TYPE;
    }

    out->e_version = read_u32(data + 20);
    out->e_entry = read_u64(data + 24);
    out->e_phoff = read_u64(data + 32);
    out->e_shoff = read_u64(data + 40);
    out->e_flags = read_u32(data + 48);
    out->e_ehsize = read_u16(data + 52);
    out->e_phentsize = read_u16(data + 54);
    out->e_phnum = read_u16(data + 56);
    out->e_shentsize = read_u16(data + 58);
    out->e_shnum = read_u16(data + 60);
    out->e_shstrndx = read_u16(data + 62);

    if (encoding != ELFDATA2LSB && encoding != ELFDATA2MSB) {
        return ELF_HDR_ERR_ENCODING;
    }

    return ELF_HDR_OK;
}

// ============================================================================
// WRAPPERS PARA ASM (FALLBACK A C)
// ============================================================================

__attribute__((weak)) int elf_header_parse_fast(const uint8_t *data, size_t size, elf_header_t *out) {
    return elf_header_parse(data, size, out);
}

__attribute__((weak)) uint64_t elf_read_u64_le(const uint8_t *data) {
    return read_u64_le(data);
}

__attribute__((weak)) uint64_t elf_read_u64_be(const uint8_t *data) {
    return read_u64_be(data);
}

__attribute__((weak)) uint32_t elf_read_u32_le(const uint8_t *data) {
    return read_u32_le(data);
}

__attribute__((weak)) uint32_t elf_read_u32_be(const uint8_t *data) {
    return read_u32_be(data);
}

__attribute__((weak)) uint16_t elf_read_u16_le(const uint8_t *data) {
    return read_u16_le(data);
}

__attribute__((weak)) uint16_t elf_read_u16_be(const uint8_t *data) {
    return read_u16_be(data);
}

// ============================================================================
// INTEGRACION: Funcion de alto nivel
// ============================================================================

elf_status_t elf_parse_header(elf_buffer_t *buf, elf_header_t *header) {
    if (!buf || !header || buf->size < 64) {
        return ELF_ERR_INVALID;
    }
    
    int status = elf_header_parse_fast(buf->data, buf->size, header);
    
    if (status != ELF_HDR_OK) {
        return ELF_ERR_INVALID;
    }
    
    return ELF_OK;
}