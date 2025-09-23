# ARM64 OS Makefile
# Save as: ~/OS_proj/Makefile

# Cross-compiler settings
CC = aarch64-elf-gcc
LD = aarch64-elf-ld
OBJCOPY = aarch64-elf-objcopy

# Directories
SRCDIR = src
BUILDDIR = build

# Compiler flags
CFLAGS = -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -O2
CFLAGS += -mgeneral-regs-only -MMD -MP

# Linker flags
LDFLAGS = --nostdlib

# Source files
ASM_SOURCES = $(wildcard $(SRCDIR)/*.s)
C_SOURCES = $(wildcard $(SRCDIR)/*.c)

# Object files
ASM_OBJECTS = $(ASM_SOURCES:$(SRCDIR)/%.s=$(BUILDDIR)/%.o)
C_OBJECTS = $(C_SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Target kernel
KERNEL_ELF = $(BUILDDIR)/kernel.elf
KERNEL_IMG = $(BUILDDIR)/kernel8.img

.PHONY: all clean run debug

all: $(KERNEL_IMG)

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile assembly files
$(BUILDDIR)/%.o: $(SRCDIR)/%.s | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel ELF
$(KERNEL_ELF): $(OBJECTS) $(SRCDIR)/kernel.ld
	$(LD) $(LDFLAGS) -T $(SRCDIR)/kernel.ld $(OBJECTS) -o $@

# Convert ELF to binary image
$(KERNEL_IMG): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "Kernel built successfully: $@"
	@ls -la $@

# Run in QEMU
run: $(KERNEL_IMG)
	qemu-system-aarch64 -M virt -cpu cortex-a72 -m 256M \
		-kernel $(KERNEL_IMG) -nographic

# Run in QEMU with debugging
debug: $(KERNEL_IMG)
	qemu-system-aarch64 -M virt -cpu cortex-a72 -m 256M \
		-kernel $(KERNEL_IMG) -nographic -s -S

# Clean build files
clean:
	rm -rf $(BUILDDIR)

# Show help
help:
	@echo "Available targets:"
	@echo "  all    - Build the kernel (default)"
	@echo "  run    - Build and run in QEMU"
	@echo "  debug  - Build and run with GDB debugging"
	@echo "  clean  - Remove build files"
	@echo "  help   - Show this help"