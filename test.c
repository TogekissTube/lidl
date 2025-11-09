#include "elf_parser.h"
#include "elf_header.h"
#include <stdio.h>
#include <string.h>

// Funciones helper de lectura
static uint16_t read_u16_le(const uint8_t *d) {
    return d[0] | (d[1] << 8);
}

static uint32_t read_u32_le(const uint8_t *d) {
    return d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
}

static uint64_t read_u64_le(const uint8_t *d) {
    return (uint64_t)d[0] | ((uint64_t)d[1] << 8) |
           ((uint64_t)d[2] << 16) | ((uint64_t)d[3] << 24) |
           ((uint64_t)d[4] << 32) | ((uint64_t)d[5] << 40) |
           ((uint64_t)d[6] << 48) | ((uint64_t)d[7] << 56);
}

static uint16_t read_u16_be(const uint8_t *d) {
    return (d[0] << 8) | d[1];
}

static uint32_t read_u32_be(const uint8_t *d) {
    return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3];
}

static uint64_t read_u64_be(const uint8_t *d) {
    return ((uint64_t)d[0] << 56) | ((uint64_t)d[1] << 48) |
           ((uint64_t)d[2] << 40) | ((uint64_t)d[3] << 32) |
           ((uint64_t)d[4] << 24) | ((uint64_t)d[5] << 16) |
           ((uint64_t)d[6] << 8) | (uint64_t)d[7];
}

// Versión C pura de elf_header_parse
static int elf_header_parse_c_only(const uint8_t *data, size_t size, elf_header_t *out) {
    if (!data || !out || size < 64) {
        return 7; // ELF_HDR_ERR_SIZE
    }

    // Validar magic
    if (data[0] != 0x7F || data[1] != 'E' || data[2] != 'L' || data[3] != 'F') {
        printf("[DEBUG] Magic check failed\n");
        printf("  data[0-3]: %02x %02x %02x %02x\n", 
               data[0], data[1], data[2], data[3]);
        return 1; // ELF_HDR_ERR_MAGIC
    }

    // Validar clase (debe ser 2 = 64-bit)
    if (data[4] != 2) {
        printf("[DEBUG] Class check failed: %d\n", data[4]);
        return 2; // ELF_HDR_ERR_CLASS
    }

    // Validar encoding (debe ser 1=LE o 2=BE)
    if (data[5] != 1 && data[5] != 2) {
        printf("[DEBUG] Encoding check failed: %d\n", data[5]);
        return 3; // ELF_HDR_ERR_ENCODING
    }

    // Validar version (debe ser 1)
    if (data[6] != 1) {
        printf("[DEBUG] Version check failed: %d\n", data[6]);
        return 4; // ELF_HDR_ERR_VERSION
    }

    printf("[+] All validations passed!\n");

    // Copiar e_ident
    memcpy(out->e_ident, data, 16);

    // Parsear según encoding
    if (data[5] == 1) { // Little-endian
        out->e_type = read_u16_le(data + 16);
        out->e_machine = read_u16_le(data + 18);
        out->e_version = read_u32_le(data + 20);
        out->e_entry = read_u64_le(data + 24);
        out->e_phoff = read_u64_le(data + 32);
        out->e_shoff = read_u64_le(data + 40);
        out->e_flags = read_u32_le(data + 48);
        out->e_ehsize = read_u16_le(data + 52);
        out->e_phentsize = read_u16_le(data + 54);
        out->e_phnum = read_u16_le(data + 56);
        out->e_shentsize = read_u16_le(data + 58);
        out->e_shnum = read_u16_le(data + 60);
        out->e_shstrndx = read_u16_le(data + 62);
    } else { // Big-endian
        out->e_type = read_u16_be(data + 16);
        out->e_machine = read_u16_be(data + 18);
        out->e_version = read_u32_be(data + 20);
        out->e_entry = read_u64_be(data + 24);
        out->e_phoff = read_u64_be(data + 32);
        out->e_shoff = read_u64_be(data + 40);
        out->e_flags = read_u32_be(data + 48);
        out->e_ehsize = read_u16_be(data + 52);
        out->e_phentsize = read_u16_be(data + 54);
        out->e_phnum = read_u16_be(data + 56);
        out->e_shentsize = read_u16_be(data + 58);
        out->e_shnum = read_u16_be(data + 60);
        out->e_shstrndx = read_u16_be(data + 62);
    }

    return 0; // ELF_HDR_OK
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    printf("[*] Testing C-only parser: %s\n\n", argv[1]);

    // Leer archivo
    elf_buffer_t *buf = elf_buffer_new(0);
    elf_read_file(argv[1], buf);
    printf("[+] File size: %zu bytes\n", buf->size);

    // Ver primeros 4 bytes
    printf("[+] First 4 bytes: %02x %02x %02x %02x\n",
           buf->data[0], buf->data[1], buf->data[2], buf->data[3]);

    // Parsear con C puro
    printf("\n[*] Parsing with C-only parser...\n");
    elf_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    
    int status = elf_header_parse_c_only(buf->data, buf->size, &hdr);

    if (status == 0) {
        printf("[+] Parse SUCCESS!\n\n");
        printf("Entry Point:    0x%lx\n", hdr.e_entry);
        printf("PHDR Offset:    0x%lx (%u entries)\n", hdr.e_phoff, hdr.e_phnum);
        printf("SHDR Offset:    0x%lx (%u entries)\n", hdr.e_shoff, hdr.e_shnum);
        printf("Machine:        %u\n", hdr.e_machine);
        printf("Type:           %u\n", hdr.e_type);
    } else {
        printf("[-] Parse FAILED with status: %d\n", status);
    }

    elf_buffer_free(buf);
    return status;
}
