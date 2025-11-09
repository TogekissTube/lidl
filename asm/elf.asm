; elf_parser_asm.asm - Lectura ELF optimizada en ASM x86-64
; Funciones de bajo nivel para lectura rápida de buffers

global elf_read_bulk
global elf_validate_magic_fast
global elf_buffer_memcpy_fast
global elf_buffer_seek_fast

section .data
    elf_magic db 0x7F, 'E', 'L', 'F'

section .text

; ============================================================================
; elf_read_bulk(uint8_t *dest, uint8_t *src, size_t count)
; rdi = dest, rsi = src, rdx = count
; Copia bloques de 8 bytes (64-bit) cuando sea posible
; ============================================================================
elf_read_bulk:
    push rbp
    mov rbp, rsp
    push rbx
    
    ; count >= 8?
    cmp rdx, 8
    jl .read_bulk_remainder
    
    ; Alinear a 16 bytes para mejor caché
    mov rax, rdi
    and rax, 0xF
    test rax, rax
    jz .read_bulk_aligned
    
    ; Alinear destino
    mov rcx, 16
    sub rcx, rax
    cmp rcx, rdx
    jg .read_bulk_remainder
    
.read_bulk_align_loop:
    mov al, byte [rsi]
    mov byte [rdi], al
    inc rsi
    inc rdi
    dec rcx
    dec rdx
    jnz .read_bulk_align_loop
    
.read_bulk_aligned:
    ; Copiar en bloques de 64 bytes (8x qwords)
    mov rcx, rdx
    shr rcx, 6              ; rcx = count / 64
    test rcx, rcx
    jz .read_bulk_qword_loop
    
.read_bulk_64_loop:
    mov rax, [rsi]
    mov [rdi], rax
    mov rax, [rsi + 8]
    mov [rdi + 8], rax
    mov rax, [rsi + 16]
    mov [rdi + 16], rax
    mov rax, [rsi + 24]
    mov [rdi + 24], rax
    mov rax, [rsi + 32]
    mov [rdi + 32], rax
    mov rax, [rsi + 40]
    mov [rdi + 40], rax
    mov rax, [rsi + 48]
    mov [rdi + 48], rax
    mov rax, [rsi + 56]
    mov [rdi + 56], rax
    
    add rsi, 64
    add rdi, 64
    dec rcx
    jnz .read_bulk_64_loop
    
    ; Resto después de bloques de 64
    and rdx, 0x3F
    
.read_bulk_qword_loop:
    cmp rdx, 8
    jl .read_bulk_remainder
    
    mov rax, [rsi]
    mov [rdi], rax
    add rsi, 8
    add rdi, 8
    sub rdx, 8
    jmp .read_bulk_qword_loop
    
.read_bulk_remainder:
    ; Copiar bytes restantes
    test rdx, rdx
    jz .read_bulk_done
    
.read_bulk_byte_loop:
    mov al, byte [rsi]
    mov byte [rdi], al
    inc rsi
    inc rdi
    dec rdx
    jnz .read_bulk_byte_loop
    
.read_bulk_done:
    pop rbx
    pop rbp
    ret

; ============================================================================
; elf_validate_magic_fast(uint8_t *data)
; rdi = data
; Retorna: 1 si es ELF válido, 0 si no
; ============================================================================
elf_validate_magic_fast:
    push rbp
    mov rbp, rsp
    
    ; Cargar 4 bytes de magic esperado
    lea rax, [rel elf_magic]
    mov edx, dword [rax]
    
    ; Cargar 4 bytes del buffer
    mov ecx, dword [rdi]
    
    ; Comparar
    cmp edx, ecx
    je .magic_valid
    
    xor eax, eax            ; retorna 0 (inválido)
    pop rbp
    ret
    
.magic_valid:
    mov eax, 1              ; retorna 1 (válido)
    pop rbp
    ret

; ============================================================================
; elf_buffer_memcpy_fast(elf_buffer_t *buf, uint8_t *src, size_t count)
; rdi = buf, rsi = src, rdx = count
; Copia data al buffer con expansión automática
; Retorna: 0 OK, -1 error
; ============================================================================
elf_buffer_memcpy_fast:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    
    ; Guardar parámetros
    mov r12, rdi            ; r12 = buf
    mov r13, rsi            ; r13 = src
    mov rbx, rdx            ; rbx = count
    
    ; Verificar capacidad: buf->size + count > buf->capacity
    mov rax, [rdi]          ; rax = buf->data (offset 0)
    mov rcx, [rdi + 8]      ; rcx = buf->size (offset 8)
    mov r8, [rdi + 16]      ; r8 = buf->capacity (offset 16)
    
    add rcx, rbx
    cmp rcx, r8
    jle .memcpy_fast_no_expand
    
    ; Necesita expansión - por ahora retorna error
    mov eax, -1
    jmp .memcpy_fast_end
    
.memcpy_fast_no_expand:
    ; Copiar datos
    mov rdi, [r12]          ; rdi = buf->data
    add rdi, [r12 + 8]      ; rdi += buf->size
    mov rsi, r13            ; rsi = src
    mov rdx, rbx            ; rdx = count
    
    ; Llamar elf_read_bulk
    call elf_read_bulk
    
    ; Actualizar buf->size
    mov rax, [r12 + 8]
    add rax, rbx
    mov [r12 + 8], rax
    
    xor eax, eax            ; retorna 0 (OK)
    
.memcpy_fast_end:
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ============================================================================
; elf_buffer_seek_fast(elf_buffer_t *buf, size_t offset)
; rdi = buf, rsi = offset
; Actualiza buf->pos sin validaciones
; ============================================================================
elf_buffer_seek_fast:
    push rbp
    mov rbp, rsp
    
    ; buf->pos (offset 24) = offset
    mov [rdi + 24], rsi
    
    pop rbp
    ret
