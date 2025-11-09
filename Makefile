# Makefile - Parte 2: ELF Header Parser (.so library)

CC = gcc
NASM = nasm
CFLAGS = -Wall -Wextra -O0 -fPIC -m64 -std=c99 -g 
NASM_FLAGS = -f elf64 -g -F dwarf
LDFLAGS = -shared -m64 

# Source files
C_SOURCES = elf_parser.c elf_header.c
ASM_SOURCES = elf.asm header.asm
OBJECTS = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.asm=.o)

# Output library
SONAME = ld-x86_64.so
VERSION = 0.1
SO_FULL = $(SONAME).$(VERSION)
SO_LINK = $(SONAME).0

# Test executable
TEST_EXEC = test
TEST_SOURCE = test.c
TEST_CFLAGS = -Wall -Wextra -O2 -m64 -std=c99  -g

# ============================================================================
# Main targets
# ============================================================================

all: $(SO_FULL) $(SO_LINK) $(TEST_EXEC)

# Build shared library
$(SO_FULL): $(OBJECTS)
	@echo "[LD] Linking shared library..."
	$(CC) $(LDFLAGS) -Wl,-soname,$(SO_LINK) -o $@ $^
	@echo "[+] Created: $@"

$(SO_LINK): $(SO_FULL)
	ln -sf $(SO_FULL) $(SO_LINK)
	@echo "[+] Created symlink: $@"

$(SONAME): $(SO_LINK)
	ln -sf $(SO_LINK) $(SONAME)
	@echo "[+] Created symlink: $@"

# Build test executable (static link)
$(TEST_EXEC): $(TEST_SOURCE) $(C_SOURCES:.c=.o) $(ASM_SOURCES:.asm=.o)
	@echo "[CC] Building test executable..."
	$(CC) $(TEST_CFLAGS) -o $@ $^
	@echo "[+] Created: $@"

# C object files
%.o: %.c
	@echo "[CC] Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# ASM object files
%.o: %.asm
	@echo "[NASM] Assembling $<..."
	$(NASM) $(NASM_FLAGS) -o $@ $<

# ============================================================================
# Utility targets
# ============================================================================

test_t: $(TEST_EXEC)
	@echo "[+] Running test..."
	LD_LIBRARY_PATH=. ./$(TEST_EXEC) /usr/bin/ls
	@echo "[+] Test completed"

test_other: $(TEST_EXEC)
	@echo "[+] Running test on other binaries..."
	LD_LIBRARY_PATH=. ./$(TEST_EXEC) /bin/bash
	LD_LIBRARY_PATH=. ./$(TEST_EXEC) /usr/bin/gcc

install: $(SO_FULL) $(SO_LINK)
	@echo "[*] Installing..."
	sudo cp $(SO_FULL) /usr/local/lib/
	sudo cp $(SO_LINK) /usr/local/lib/
	sudo cp elf_parser.h elf_header.h /usr/local/include/
	sudo ldconfig
	@echo "[+] Installed to /usr/local/lib/"

uninstall:
	@echo "[*] Uninstalling..."
	sudo rm -f /usr/local/lib/$(SO_FULL) /usr/local/lib/$(SO_LINK) /usr/local/lib/$(SONAME)
	sudo rm -f /usr/local/include/elf_parser.h /usr/local/include/elf_header.h
	sudo ldconfig
	@echo "[+] Uninstalled"

clean:
	@echo "[*] Cleaning..."
	rm -f $(OBJECTS) $(SO_FULL) $(SO_LINK) $(SONAME) $(TEST_EXEC)
	rm test
	@echo "[+] Cleaned"

clean_all: clean
	@echo "[*] Full clean..."
	rm -f *.o *.so* *.a
	@echo "[+] Full clean done"

debug: CFLAGS += -g -DDEBUG
debug: TEST_CFLAGS += -g -DDEBUG
debug: clean all
	@echo "[+] Debug build complete"

release: CFLAGS += -DNDEBUG -O3
release: clean all
	@echo "[+] Release build complete"

info:
	@echo "=== ELF Parser - Part 2 Build Info ==="
	@echo "Library: $(SO_FULL) (v$(VERSION))"
	@echo "Test: $(TEST_EXEC)"
	@echo "Targets: test, install, uninstall, clean"
	@echo "========================================"

.PHONY: all test test_other install uninstall clean clean_all debug release info
