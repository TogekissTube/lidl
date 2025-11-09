global phdr_parse_fast
global phdr_read_u32_le
global phdr_read_u32_be
global phdr_read_u64_le
global phdr_read_u64_be

section .text

; ============================================================================
; phdr_read_u32_le(const uint8_t *data) -> uint32_t
; rdi = data
; Retorna: eax = valor leído (little-endian)
; ============================================================================
phdr_read_u32_le:
    push rbp
    mov rbp, rsp
    mov eax, dword [rdi]
    pop rbp
    ret

; ============================================================================
; phdr_read_u32_be(const uint8_t *data) -> uint32_t
; rdi = data
; Retorna: eax = valor leído (big-endian)
; ============================================================================
phdr_read_u32_be:
    push rbp
    mov rbp, rsp
    mov eax, dword [rdi]
    bswap eax
    pop rbp
    ret

; ============================================================================
; phdr_read_u64_le(const uint8_t *data) -> uint64_t
; rdi = data
; Retorna: rax = valor leído (little-endian)
; ============================================================================
phdr_read_u64_le:
    push rbp
    mov rbp, rsp
    mov rax, qword [rdi]
    pop rbp
    ret

; ============================================================================
; phdr_read_u64_be(const uint8_t *data) -> uint64_t
; rdi = data
; Retorna: rax = valor leído (big-endian)
; ============================================================================
phdr_read_u64_be:
    push rbp
    mov rbp, rsp
    mov rax, qword [rdi]
    bswap rax
    pop rbp
    ret

; ============================================================================
; phdr_parse_fast(const uint8_t *data, size_t size, phdr_t *out)
;
; rdi = data (uint8_t *)
; rsi = size (size_t)
; rdx = out  (phdr_t *)
;
; Retorna: eax = status code
;   0 = PHDR_OK
;   1 = PHDR_ERR_SIZE
;   5 = PHDR_ERR_INVALID
;
; Estructura phdr_t (56 bytes):
;   offset 0:  p_type (uint32_t)
;   offset 4:  p_flags (uint32_t)
;   offset 8:  p_offset (uint64_t)
;   offset 16: p_vaddr (uint64_t)
;   offset 24: p_paddr (uint64_t)
;   offset 32: p_filesz (uint64_t)
;   offset 40: p_memsz (uint64_t)
;   offset 48: p_align (uint64_t)
; ============================================================================
phdr_parse_fast:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    
    mov r12, rdi                ; r12 = data
    mov r13, rsi                ; r13 = size
    mov rbx, rdx                ; rbx = out (phdr_t *)
    
    ; Validación: Tamaño mínimo (56 bytes)
    cmp r13, 56
    jl .phdr_parse_err_size
    
    ; Lectura: p_type (offset 0, 4 bytes)
    mov eax, dword [r12]
    mov dword [rbx], eax
    
    ; Lectura: p_flags (offset 4, 4 bytes)
    mov eax, dword [r12 + 4]
    mov dword [rbx + 4], eax
    
    ; Lectura: p_offset (offset 8, 8 bytes)
    mov rax, qword [r12 + 8]
    mov qword [rbx + 8], rax
    
    ; Lectura: p_vaddr (offset 16, 8 bytes)
    mov rax, qword [r12 + 16]
    mov qword [rbx + 16], rax
    
    ; Lectura: p_paddr (offset 24, 8 bytes)
    mov rax, qword [r12 + 24]
    mov qword [rbx + 24], rax
    
    ; Lectura: p_filesz (offset 32, 8 bytes)
    mov rax, qword [r12 + 32]
    mov qword [rbx + 32], rax
    
    ; Lectura: p_memsz (offset 40, 8 bytes)
    mov rax, qword [r12 + 40]
    mov qword [rbx + 40], rax
    
    ; Lectura: p_align (offset 48, 8 bytes)
    mov rax, qword [r12 + 48]
    mov qword [rbx + 48], rax
    
    ; Validación: p_filesz <= p_memsz
    mov rax, qword [rbx + 32]   ; rax = p_filesz
    mov rcx, qword [rbx + 40]   ; rcx = p_memsz
    
    cmp rax, rcx
    jg .phdr_parse_err_invalid
    
    ; Retorno: SUCCESS
    xor eax, eax
    jmp .phdr_parse_done
    
.phdr_parse_err_size:
    mov eax, 1
    jmp .phdr_parse_done
    
.phdr_parse_err_invalid:
    mov eax, 5
    
.phdr_parse_done:
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
