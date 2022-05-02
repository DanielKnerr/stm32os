# die genutzten Compiler der "Arm GNU Toolchain"
# siehe https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain
CC = tools/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc
AR = tools/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-ar

# Flags
COMMON_FLAGS = -Iinclude -nostartfiles -Wall -Wextra -Wno-main -mcpu=cortex-m4 -mthumb -fno-exceptions -ggdb
CFLAGS = $(COMMON_FLAGS) -c
LFLAGS = $(COMMON_FLAGS) -Tlink/link.ld
ARFLAGS = -rcs

# ".a" weil es eine static library ist (".a" = "archive")
# siehe https://stackoverflow.com/a/6561587/17278981
STATIC_LIB_NAME = libOS.a

# Name der zu generierenden ELF-Datei
EXEC_NAME = os.elf

# Ornder
SRCDIR = src
OUTPUTDIR = out
USERDIR = user
KERNEL_OBJDIR = obj/kernel
USER_OBJDIR = obj/user

# Sammelt alle .c und .S Dateien vom Kernel und vom User
src_files = $(wildcard $(SRCDIR)/*.c)
src_files += $(wildcard $(SRCDIR)/*/*.c)
src_files += $(wildcard $(SRCDIR)/*.S)
src_files += $(wildcard $(SRCDIR)/*/*.S)

user_files = $(wildcard $(USERDIR)/*.c)
user_files += $(wildcard $(USERDIR)/*/*.c)
user_files += $(wildcard $(USERDIR)/*.S)
user_files += $(wildcard $(USERDIR)/*/*.S)

# Macht aus den Quelldateien im Format *.c eine Objektdatei im Format *.c.o
KERNEL_OBJECTS=$(patsubst $(SRCDIR)/%, $(KERNEL_OBJDIR)/%.o, $(src_files))
USER_OBJECTS=$(patsubst $(USERDIR)/%, $(USER_OBJDIR)/%.o, $(user_files))

# kompiliert alle Quelldateien des Kernels in eine statische Bibliothek
$(OUTPUTDIR)/$(STATIC_LIB_NAME): $(KERNEL_OBJECTS)
	@mkdir -p $(OUTPUTDIR)
	$(AR) $(ARFLAGS) $(OUTPUTDIR)/$(STATIC_LIB_NAME) $(KERNEL_OBJECTS)

$(KERNEL_OBJDIR)/%.o : $(SRCDIR)/%
	@mkdir -p $(@D)
	$(CC)  -o $@ $(CFLAGS) $<

# kompiliert alle Quelldatein des Users
compileUser: $(USER_OBJECTS)

$(USER_OBJDIR)/%.o : $(USERDIR)/%
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $<

# das Erzeugen der ELF-Datei aus der statischen Bibliothek des Kernels funktioniert nicht...
# deshalb direkt alle Objektdateien auswählen
compile: $(OUTPUTDIR)/$(STATIC_LIB_NAME) $(USER_OBJECTS)
#	$(CC) $(LFLAGS) $(OUTPUTDIR)/$(STATIC_LIB_NAME) $(USER_OBJECTS) -o $(OUTPUTDIR)/$(EXEC_NAME)
	$(CC) $(LFLAGS) $(KERNEL_OBJECTS) $(USER_OBJECTS) -o $(OUTPUTDIR)/$(EXEC_NAME)

.PHONY: clean

clean:
	rm -rf $(KERNEL_OBJDIR) $(USER_OBJDIR)
	rm -rf $(OUTPUTDIR)/*