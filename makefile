CC = i686-elf-gcc
AS = as
ASM = nasm
CONFIG = ./config
GCCPARAMS =  -O0 -I./include -fno-omit-frame-pointer -nostdlib -fno-pic -fno-builtin -fno-exceptions -ffreestanding -fno-leading-underscore -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long

ASPARAMS = --32
LDPARAMS = -m elf_i386 -T linker.ld -nostdlib 

OBJ_DIR = obj
SRC_DIRS = arch kernel drivers libk asm syscalls
OBJ_DIRS = $(addprefix $(OBJ_DIR)/,$(SRC_DIRS))

$(shell mkdir -p $(OBJ_DIRS))

# List of directories to be blacklisted
BLACKLIST_DIRS := elf
# Function to filter out blacklisted directories
define filter-out-dir
$(foreach dir,$(1),$(shell find . -type d -name '$(dir)'))
endef

# Update SRC_FILES_C, SRC_FILES_S, and SRC_FILES_ASM to exclude blacklisted directories
SRC_FILES_C := $(filter-out $(call filter-out-dir,$(BLACKLIST_DIRS)),$(shell find $(SRC_DIRS) -type f -name '*.c'))
SRC_FILES_S := $(filter-out $(call filter-out-dir,$(BLACKLIST_DIRS)),$(shell find $(SRC_DIRS) -type f -name '*.s'))
SRC_FILES_ASM := $(filter-out $(call filter-out-dir,$(BLACKLIST_DIRS)),$(shell find $(SRC_DIRS) -type f -name '*.asm'))

# Generate object file paths
OBJ_FILES_C := $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRC_FILES_C))
OBJ_FILES_S := $(patsubst %.s, $(OBJ_DIR)/%.o, $(SRC_FILES_S))
OBJ_FILES_ASM := $(patsubst %.asm, $(OBJ_DIR)/%.o, $(SRC_FILES_ASM))

# Compile C files
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -c -o $@ $<

# Assemble assembly files
$(OBJ_DIR)/%.o: %.s
	@mkdir -p $(@D)
	$(AS) $(ASPARAMS) -o $@ $<

# Assemble NASM files
$(OBJ_DIR)/%.o: %.asm
	@mkdir -p $(@D)
	$(ASM) -f elf32 -o $@ $<

# Link object files into binary
AthenX.bin: $(OBJ_FILES_C) $(OBJ_FILES_S) $(OBJ_FILES_ASM)
	ld $(LDPARAMS) -o $@ $^
	mkdir -p isodir/boot/grub
	cp AthenX.bin isodir/boot/AthenX.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o AthenX.iso isodir

# Run the OS in QEMU
run: AthenX.bin
	(cd libc  && make )
	(cd user && make)
	bash ./scripts/boot32.sh
	qemu-system-i386 AthenX.img -m 4G -serial stdio -no-reboot -no-shutdown

# Run the OS in QEMU without debugging output
runt:
	qemu-system-i386 AthenX.img -m 4G

# Clean up generated files
clean:
	rm -rf $(OUT_DIR)/*.bin $(OUT_DIR)/*.map $(OUT_DIR)/*.img $(OBJ_DIR)/* AthenX.iso AthenX.bin libc/libc.a
