SHELL = /bin/bash
LDFLAGS=
INC = -I include
BUILDDIR = build

include make/git.mk

all: libewiewitf

libewiewitf: $(BUILDDIR)/src/mfis_communication.o $(BUILDDIR)/src/eviewitf.o
	@mkdir -p $(BUILDDIR)
	@echo "Version: $(VERSION)" > $(BUILDDIR)/version.txt
	ar rcs $(BUILDDIR)/libeviewitf.a $(BUILDDIR)/version.txt $<

$(BUILDDIR)/%.o : %.c
	@mkdir -p $(@D)
	$(CC) -c $< $(INC) -o $@

.PHONY:	clean
clean:
	@rm -rf $(BUILDDIR)

CLANG_FORMAT_DIRS = include src

.PHONY: clangformat
clangformat:
	@find $(CLANG_FORMAT_DIRS) -iname *.h -o -iname *.c | xargs clang-format --style=file -i

.PHONY: clangcheck
clangcheck:
	@find $(CLANG_FORMAT_DIRS) -iname *.h -o -iname *.c -exec cat {} \; \
		| diff -u <(find $(CLANG_FORMAT_DIRS) -iname *.h -o -iname *.c -exec \
		clang-format --style=file {} \;) -