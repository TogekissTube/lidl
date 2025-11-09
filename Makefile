CC = gcc
NASM = nasm
CFLAGS = -Wall -Wextra -O2 -fPIC -m64 -std=c99 -g
NASM_FLAGS = -f elf64 -g -F dwarf
LDFLAGS = -m64

# Source files
C_SOURCES = src/elf_parser.c src/elf_header.c src/phdr.c
ASM_SOURCES = asm/elf.asm asm/header.asm asm/phdr.asm
TEST_SOURCE = tests/test.c

# Object files
C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
TEST_OBJECT = tests/test.o
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# Output
TEST_EXEC = test

# ============================================================================
# Default target
# ============================================================================

all: $(TEST_EXEC)

# ============================================================================
# Build test executable
# ============================================================================

$(TEST_EXEC): $(OBJECTS) $(TEST_OBJECT)
	@echo "[LD] Linking $@..."
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "[+] Created: $@"

# ============================================================================
# C compilation rules
# ============================================================================

src/%.o: src/%.c
	@echo "[CC] Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

tests/%.o: tests/%.c
	@echo "[CC] Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# ============================================================================
# ASM compilation rules
# ============================================================================

asm/%.o: asm/%.asm
	@echo "[NASM] Assembling $<..."
	$(NASM) $(NASM_FLAGS) -o $@ $<

# ============================================================================
# Testing targets
# ============================================================================

test_test: $(TEST_EXEC)
	@echo "[+] Running test..."
	./$(TEST_EXEC) /bin/ls
	@echo "[+] Test completed"

test_valgrind: $(TEST_EXEC)
	@echo "[+] Running with Valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_EXEC) /bin/ls

test_multiple: $(TEST_EXEC)
	@echo "[+] Testing multiple binaries..."
	./$(TEST_EXEC) /bin/ls
	./$(TEST_EXEC) /bin/cat
	./$(TEST_EXEC) /usr/bin/python3 2>/dev/null || echo "Python3 not found"

# ============================================================================
# Cleaning targets
# ============================================================================

clean:
	@echo "[*] Cleaning object files..."
	rm -f $(C_OBJECTS) $(ASM_OBJECTS) $(TEST_OBJECT)
	@echo "[+] Cleaned"

clean_all: clean
	@echo "[*] Full clean..."
	rm -f $(TEST_EXEC)
	@echo "[+] Full clean done"

distclean: clean_all
	@echo "[*] Distribution clean..."
	rm -f *.o *.so *.a
	@echo "[+] Distribution clean done"

# ============================================================================
# Info targets
# ============================================================================

info:
	@echo "=== x86-64 Dynamic Linker - Build Info ==="
	@echo "Test executable: $(TEST_EXEC)"
	@echo "C sources: $(C_SOURCES)"
	@echo "ASM sources: $(ASM_SOURCES)"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build test executable"
	@echo "  make test_test         - Run basic test"
	@echo "  make test_valgrind - Run with memory checking"
	@echo "  make test_multiple - Test multiple binaries"
	@echo "  make clean        - Clean object files"
	@echo "  make clean_all    - Clean everything"
	@echo "  make info         - Show this help"

# ============================================================================
# Phony targets
# ============================================================================

.PHONY: all test_test test_valgrind test_multiple clean clean_all distclean info
