#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "elf_header.h"

#define ELF_MAGIC "\x7FELF"
#define ELF_HEADER_64_SIZE 64

typedef enum {
    ELF_OK = 0,
    ELF_ERR_OPEN = 1,
    ELF_ERR_READ = 2,
    ELF_ERR_INVALID = 3,
    ELF_ERR_ALLOC = 4,
    ELF_ERR_EOF = 5
} elf_status_t;

typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
    size_t pos;
} elf_buffer_t;

// Buffer management
elf_buffer_t* elf_buffer_new(size_t capacity);
void elf_buffer_free(elf_buffer_t *buf);

// File reading
elf_status_t elf_read_file(const char *filepath, elf_buffer_t *buf);
elf_status_t elf_read_fd(int fd, elf_buffer_t *buf);
elf_status_t elf_read_stream(FILE *stream, elf_buffer_t *buf);

// Byte reading
elf_status_t elf_read_byte(elf_buffer_t *buf, uint8_t *out);
elf_status_t elf_read_bytes(elf_buffer_t *buf, uint8_t *out, size_t count);
elf_status_t elf_peek_byte(elf_buffer_t *buf, uint8_t *out);

// Validation
elf_status_t elf_validate_magic(elf_buffer_t *buf);
int elf_is_elf(elf_buffer_t *buf);

// Utilities
void elf_buffer_reset(elf_buffer_t *buf);
void elf_buffer_seek(elf_buffer_t *buf, size_t offset);
size_t elf_buffer_tell(elf_buffer_t *buf);
size_t elf_buffer_remaining(elf_buffer_t *buf);

// Header parsing
elf_status_t elf_parse_header(elf_buffer_t *buf, elf_header_t *header);

#endif
