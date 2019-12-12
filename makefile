SHELL = /bin/bash
LDFLAGS=
INC = -I include
BUILDDIR = build
DESTDIR ?= ${SDKTARGETSYSROOT}

include make/git.mk

# Needed because bitbake overrides CFLAGS
TARGET_CFLAGS += -DVERSION=\"$(VERSION)\"

all: eviewitf

.PHONY: eviewitf
eviewitf: $(BUILDDIR)/src/main.o libewiewitf
	$(CC) $< -o $(BUILDDIR)/$@ -l$@ -L$(BUILDDIR) 

.PHONY: libewiewitf
libewiewitf: $(BUILDDIR)/src/mfis_communication.o $(BUILDDIR)/src/eviewitf.o $(BUILDDIR)/src/eviewitf_ssd.o
	@mkdir -p $(BUILDDIR)
	$(AR) rcs $(BUILDDIR)/libeviewitf.a $^

$(BUILDDIR)/%.o : %.c
	@mkdir -p $(@D)
	$(CC) $(TARGET_CFLAGS) $(CFLAGS) -c $< $(INC) -o $@

.PHONY:	clean
clean:
	@rm -rf $(BUILDDIR)

.PHONY: install
install: eviewitf
	mkdir -p $(DESTDIR)/usr/bin/
	cp $(BUILDDIR)/eviewitf $(DESTDIR)/usr/bin/eviewitf
	mkdir -p $(DESTDIR)/usr/lib/
	cp $(BUILDDIR)/libeviewitf.a $(DESTDIR)/usr/lib/libeviewitf.a
	mkdir -p $(DESTDIR)/usr/include/
	cp include/eviewitf.h $(DESTDIR)/usr/include/eviewitf.h

CLANG_FORMAT_DIRS = include src

.PHONY: clangformat
clangformat:
	@find $(CLANG_FORMAT_DIRS) -iname *.h -o -iname *.c | xargs clang-format --style=file -i

.PHONY: clangcheck
clangcheck:
	@find $(CLANG_FORMAT_DIRS) -iname *.h -o -iname *.c -exec cat {} \; \
		| diff -u <(find $(CLANG_FORMAT_DIRS) -iname *.h -o -iname *.c -exec \
		clang-format --style=file {} \;) -