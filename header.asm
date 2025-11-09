global elf_header_parse_fast
global elf_read_u64_le
global elf_read_u64_be
global elf_read_u32_le
global elf_read_u32_be
global elf_read_u16_le
global elf_read_u16_be

section .data
    elf_magic db 0x7F, 'E', 'L', 'F'

section .text

; ============================================================================
; elf_read_u16_le(const uint8_t *data) -> uint16_t
; rdi = data
; Retorna: ax = valor leído (little-endian)
; 
; Lectura de 16-bit en formato little-endian manualmente
; data[0] | (data[1] << 8)
; ============================================================================
elf_read_u16_le:
    push rbp
    mov rbp, rsp
    
    ; al = data[0]
    movzx eax, byte [rdi]
    
    ; cl = data[1], cl <<= 8
    movzx ecx, byte [rdi + 1]
    shl ecx, 8
    
    ; eax = data[0] | (data[1] << 8)
    or eax, ecx
    
    ; Mantener solo 16 bits en ax
    and eax, 0xFFFF
    
    pop rbp
    ret

; ============================================================================
; elf_read_u16_be(const uint8_t *data) -> uint16_t
; rdi = data
; Retorna: ax = valor leído (big-endian)
;
; Lectura de 16-bit en formato big-endian
; (data[0] << 8) | data[1]
; ============================================================================
elf_read_u16_be:
    push rbp
    mov rbp, rsp
    
    ; al = data[0], al <<= 8
    movzx eax, byte [rdi]
    shl eax, 8
    
    ; cl = data[1]
    movzx ecx, byte [rdi + 1]
    
    ; eax = (data[0] << 8) | data[1]
    or eax, ecx
    
    ; Mantener solo 16 bits en ax
    and eax, 0xFFFF
    
    pop rbp
    ret

; ============================================================================
; elf_read_u32_le(const uint8_t *data) -> uint32_t
; rdi = data
; Retorna: eax = valor leído (little-endian)
;
; En x86-64 little-endian nativo, lectura directa es más rápida
; ============================================================================
elf_read_u32_le:
    push rbp
    mov rbp, rsp
    
    ; Lectura directa de 32-bit (asume alineación y LE)
    mov eax, dword [rdi]
    
    pop rbp
    ret

; ============================================================================
; elf_read_u32_be(const uint8_t *data) -> uint32_t
; rdi = data
; Retorna: eax = valor leído (big-endian)
;
; Lectura + intercambio de bytes (bswap = 1 ciclo)
; ============================================================================
elf_read_u32_be:
    push rbp
    mov rbp, rsp
    
    ; Lectura de 32-bit
    mov eax, dword [rdi]
    
    ; Intercambiar bytes: AABBCCDD -> DDCCBBAA
    bswap eax
    
    pop rbp
    ret

; ============================================================================
; elf_read_u64_le(const uint8_t *data) -> uint64_t
; rdi = data
; Retorna: rax = valor leído (little-endian)
;
; Lectura directa de 64-bit (asume LE nativo)
; ============================================================================
elf_read_u64_le:
    push rbp
    mov rbp, rsp
    
    ; Lectura directa de 64-bit
    mov rax, qword [rdi]
    
    pop rbp
    ret

; ============================================================================
; elf_read_u64_be(const uint8_t *data) -> uint64_t
; rdi = data
; Retorna: rax = valor leído (big-endian)
;
; Lectura + intercambio de bytes (bswap en 64-bit)
; ============================================================================
elf_read_u64_be:
    push rbp
    mov rbp, rsp
    
    ; Lectura de 64-bit
    mov rax, qword [rdi]
    
    ; Intercambiar bytes: AABBCCDD_EEFFGGHH -> HHGGGFF_EEDDCCBBAA
    bswap rax
    
    pop rbp
    ret

; ============================================================================
; elf_header_parse_fast(const uint8_t *data, size_t size, elf_header_t *out)
; 
; rdi = data (uint8_t *)
; rsi = size (size_t)
; rdx = out  (elf_header_t *)
;
; Retorna: eax = status code
;   0 = ELF_HDR_OK
;   1 = ELF_HDR_ERR_MAGIC
;   2 = ELF_HDR_ERR_CLASS
;   3 = ELF_HDR_ERR_ENCODING
;   4 = ELF_HDR_ERR_VERSION
;   7 = ELF_HDR_ERR_SIZE
;
; Parser rápido de cabecera ELF 64-bit con validaciones
; ============================================================================
elf_header_parse_fast:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    
    ; Guardar parámetros en registros preservados
    mov r12, rdi                ; r12 = data
    mov r13, rsi                ; r13 = size
    mov r14, rdx                ; r14 = out
    
    ; ========================================================================
    ; VALIDACIÓN 1: Tamaño mínimo (64 bytes para ELF64)
    ; ========================================================================
    cmp r13, 64
    jl .parse_fast_err_size
    
    ; ========================================================================
    ; VALIDACIÓN 2: Magic \x7FELF (comparación byte a byte)
    ; ========================================================================
    mov al, byte [r12]          ; al = data[0]
    cmp al, 0x7F
    jne .parse_fast_err_magic
    
    mov al, byte [r12 + 1]      ; al = data[1]
    cmp al, 'E'
    jne .parse_fast_err_magic
    
    mov al, byte [r12 + 2]      ; al = data[2]
    cmp al, 'L'
    jne .parse_fast_err_magic
    
    mov al, byte [r12 + 3]      ; al = data[3]
    cmp al, 'F'
    jne .parse_fast_err_magic
    
    ; ========================================================================
    ; VALIDACIÓN 3: Clase (byte 4) - debe ser 2 (ELFCLASS64)
    ; ========================================================================
    cmp byte [r12 + 4], 2
    jne .parse_fast_err_class
    
    ; ========================================================================
    ; VALIDACIÓN 4: Encoding (byte 5) - debe ser 1 (LE) o 2 (BE)
    ; ========================================================================
    mov al, [r12 + 5]
    cmp al, 1
    je .parse_fast_encoding_ok
    cmp al, 2
    jne .parse_fast_err_encoding
    
.parse_fast_encoding_ok:
    ; ========================================================================
    ; VALIDACIÓN 5: Version (byte 6) - debe ser 1
    ; ========================================================================
    cmp byte [r12 + 6], 1
    jne .parse_fast_err_version
    
    ; ========================================================================
    ; PASO 1: Copiar e_ident (16 bytes)
    ; ========================================================================
    mov rsi, r12                ; rsi = data
    mov rdi, r14                ; rdi = out
    mov rcx, 16
    rep movsb
    
    ; ========================================================================
    ; PASO 2: Determinar endianness y parsear campos
    ; ========================================================================
    mov al, [r12 + 5]           ; al = encoding byte
    cmp al, 1                   ; 1 = ELFDATA2LSB (Little-Endian)
    je .parse_fast_little_endian
    
    ; ====== BIG-ENDIAN ======
    ; Para BE necesitamos bswap en cada lectura
    
    ; e_type (offset 16, 2 bytes)
    mov ax, [r12 + 16]
    bswap eax
    shr eax, 16
    mov [r14 + 16], ax
    
    ; e_machine (offset 18, 2 bytes)
    mov ax, [r12 + 18]
    bswap eax
    shr eax, 16
    mov [r14 + 18], ax
    
    ; e_version (offset 20, 4 bytes)
    mov eax, [r12 + 20]
    bswap eax
    mov [r14 + 20], eax
    
    ; e_entry (offset 24, 8 bytes)
    mov rax, [r12 + 24]
    bswap rax
    mov [r14 + 24], rax
    
    ; e_phoff (offset 32, 8 bytes)
    mov rax, [r12 + 32]
    bswap rax
    mov [r14 + 32], rax
    
    ; e_shoff (offset 40, 8 bytes)
    mov rax, [r12 + 40]
    bswap rax
    mov [r14 + 40], rax
    
    ; e_flags (offset 48, 4 bytes)
    mov eax, [r12 + 48]
    bswap eax
    mov [r14 + 48], eax
    
    ; e_ehsize (offset 52, 2 bytes)
    mov ax, [r12 + 52]
    bswap eax
    shr eax, 16
    mov [r14 + 52], ax
    
    ; e_phentsize (offset 54, 2 bytes)
    mov ax, [r12 + 54]
    bswap eax
    shr eax, 16
    mov [r14 + 54], ax
    
    ; e_phnum (offset 56, 2 bytes)
    mov ax, [r12 + 56]
    bswap eax
    shr eax, 16
    mov [r14 + 56], ax
    
    ; e_shentsize (offset 58, 2 bytes)
    mov ax, [r12 + 58]
    bswap eax
    shr eax, 16
    mov [r14 + 58], ax
    
    ; e_shnum (offset 60, 2 bytes)
    mov ax, [r12 + 60]
    bswap eax
    shr eax, 16
    mov [r14 + 60], ax
    
    ; e_shstrndx (offset 62, 2 bytes)
    mov ax, [r12 + 62]
    bswap eax
    shr eax, 16
    mov [r14 + 62], ax
    
    jmp .parse_fast_ok
    
    ; ====== LITTLE-ENDIAN ======
.parse_fast_little_endian:
    ; Lectura directa (no necesita bswap)
    
    ; e_type (offset 16, 2 bytes)
    mov ax, [r12 + 16]
    mov [r14 + 16], ax
    
    ; e_machine (offset 18, 2 bytes)
    mov ax, [r12 + 18]
    mov [r14 + 18], ax
    
    ; e_version (offset 20, 4 bytes)
    mov eax, [r12 + 20]
    mov [r14 + 20], eax
    
    ; e_entry (offset 24, 8 bytes)
    mov rax, [r12 + 24]
    mov [r14 + 24], rax
    
    ; e_phoff (offset 32, 8 bytes)
    mov rax, [r12 + 32]
    mov [r14 + 32], rax
    
    ; e_shoff (offset 40, 8 bytes)
    mov rax, [r12 + 40]
    mov [r14 + 40], rax
    
    ; e_flags (offset 48, 4 bytes)
    mov eax, [r12 + 48]
    mov [r14 + 48], eax
    
    ; e_ehsize (offset 52, 2 bytes)
    mov ax, [r12 + 52]
    mov [r14 + 52], ax
    
    ; e_phentsize (offset 54, 2 bytes)
    mov ax, [r12 + 54]
    mov [r14 + 54], ax
    
    ; e_phnum (offset 56, 2 bytes)
    mov ax, [r12 + 56]
    mov [r14 + 56], ax
    
    ; e_shentsize (offset 58, 2 bytes)
    mov ax, [r12 + 58]
    mov [r14 + 58], ax
    
    ; e_shnum (offset 60, 2 bytes)
    mov ax, [r12 + 60]
    mov [r14 + 60], ax
    
    ; e_shstrndx (offset 62, 2 bytes)
    mov ax, [r12 + 62]
    mov [r14 + 62], ax
    
.parse_fast_ok:
    ; ========================================================================
    ; RETORNO: SUCCESS
    ; ========================================================================
    xor eax, eax                ; retorna 0 (ELF_HDR_OK)
    jmp .parse_fast_done
    
.parse_fast_err_magic:
    mov eax, 1                  ; ELF_HDR_ERR_MAGIC
    jmp .parse_fast_done
    
.parse_fast_err_class:
    mov eax, 2                  ; ELF_HDR_ERR_CLASS
    jmp .parse_fast_done
    
.parse_fast_err_encoding:
    mov eax, 3                  ; ELF_HDR_ERR_ENCODING
    jmp .parse_fast_done
    
.parse_fast_err_version:
    mov eax, 4                  ; ELF_HDR_ERR_VERSION
    jmp .parse_fast_done
    
.parse_fast_err_size:
    mov eax, 7                  ; ELF_HDR_ERR_SIZE
    
.parse_fast_done:
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret