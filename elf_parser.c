#include "elf_parser.h"
#include "elf_header.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define ELF_MAGIC "\x7FELF"
#define READ_CHUNK_SIZE 4096
#define INITIAL_CAPACITY 65536
#define MAX 1024 * 1024 * 1024

elf_buffer_t* elf_buffer_new(size_t capacity) {
    elf_buffer_t *buf = (elf_buffer_t *)malloc(sizeof(elf_buffer_t));
    if (!buf) return NULL;
    
    if (capacity == 0) capacity = INITIAL_CAPACITY;
    
    buf->data = (uint8_t *)malloc(capacity);
    if (!buf->data) {
        free(buf);
        return NULL;
    }
    
    buf->size = 0;
    buf->capacity = capacity;
    buf->pos = 0;
    
    return buf;
}

void elf_buffer_free(elf_buffer_t *buf) {
    if (buf) {
        if (buf->data) free(buf->data);
        free(buf);
    }
}

static elf_status_t elf_buffer_expand(elf_buffer_t *buf, size_t needed) {
    size_t new_capacity = buf->capacity;
    
    while (new_capacity < buf->size + needed) {
        if (new_capacity > MAX) { 
            return ELF_ERR_READ;
        }
        new_capacity *= 2;
    }
    
    uint8_t *new_data = (uint8_t *)realloc(buf->data, new_capacity);
    if (!new_data) {
        return ELF_ERR_ALLOC;
    }
    
    buf->data = new_data;
    buf->capacity = new_capacity;
    return ELF_OK;
}

// ============================================================================
// LECTURA DESDE ARCHIVOS
// ============================================================================

elf_status_t elf_read_file(const char *filepath, elf_buffer_t *buf) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        return ELF_ERR_OPEN;
    }
    
    elf_status_t status = elf_read_fd(fd, buf);
    close(fd);
    
    return status;
}

elf_status_t elf_read_fd(int fd, elf_buffer_t *buf) {
    uint8_t chunk[READ_CHUNK_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = read(fd, chunk, READ_CHUNK_SIZE)) > 0) {
        if (bytes_read < 0) {
            return ELF_ERR_READ;
        }
        
        if (buf->size + bytes_read > buf->capacity) {
            elf_status_t status = elf_buffer_expand(buf, bytes_read);
            if (status != ELF_OK) {
                return status;
            }
        }
        
        memcpy(buf->data + buf->size, chunk, bytes_read);
        buf->size += bytes_read;
    }
    
    if (bytes_read < 0) {
        return ELF_ERR_READ;
    }
    
    return ELF_OK;
}  


elf_status_t elf_read_stream(FILE *stream, elf_buffer_t *buf) {
    uint8_t chunk[READ_CHUNK_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(chunk, 1, READ_CHUNK_SIZE, stream)) > 0) {
        if (buf->size + bytes_read > buf->capacity) {
            elf_status_t status = elf_buffer_expand(buf, bytes_read);
            if (status != ELF_OK) {
                return status;
            }
        }
        
        memcpy(buf->data + buf->size, chunk, bytes_read);
        buf->size += bytes_read;
    }
    
    if (ferror(stream)) {
        return ELF_ERR_READ;
    }
    
    return ELF_OK;
}

// ============================================================================
// LECTURA DE BYTES
// ============================================================================

elf_status_t elf_read_byte(elf_buffer_t *buf, uint8_t *out) {
    if (buf->pos >= buf->size) {
        return ELF_ERR_EOF;
    }
    
    *out = buf->data[buf->pos];
    buf->pos++;
    
    return ELF_OK;
}

elf_status_t elf_read_bytes(elf_buffer_t *buf, uint8_t *out, size_t count) {
    if (buf->pos > buf->size + count ) {
        return ELF_ERR_EOF;
    }
    
    memcpy(out, buf->data + buf->pos, count);
    buf->pos += count;
    
    return ELF_OK;
}

elf_status_t elf_peek_byte(elf_buffer_t *buf, uint8_t *out) {
    if (buf->pos >= buf->size) {
        return ELF_ERR_EOF;
    }
    
    *out = buf->data[buf->pos];
    return ELF_OK;
}

// ============================================================================
// VALIDACIÓN
// ============================================================================

elf_status_t elf_validate_magic(elf_buffer_t *buf) {
    if (!buf || buf->size < 4) {
        return ELF_ERR_INVALID;
    }
    
    if (buf->data[0] != 0x7F || buf->data[1] != 'E' || 
        buf->data[2] != 'L' || buf->data[3] != 'F') {
        return ELF_ERR_INVALID;
    }
    
    return ELF_OK;
}

int elf_is_elf(elf_buffer_t *buf) {
    return elf_validate_magic(buf) == ELF_OK;
}

// ============================================================================
// UTILIDADES DE BUFFER
// ============================================================================

void elf_buffer_reset(elf_buffer_t *buf) {
    buf->pos = 0;
}

void elf_buffer_seek(elf_buffer_t *buf, size_t offset) {
    if (offset <= buf->size) {
        buf->pos = offset;
    }
}

size_t elf_buffer_tell(elf_buffer_t *buf) {
    return buf->pos;
}

size_t elf_buffer_remaining(elf_buffer_t *buf) {
    if (buf->pos >= buf->size) {
    return 0;
    }

    free(buf);

}