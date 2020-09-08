SHELL = /bin/bash
LDFLAGS=
INC = -I include
BUILDDIR = build
DESTDIR ?= ${SDKTARGETSYSROOT}
TARGETIP ?= 192.168.0.82
CFLAGS+= -Wall -Wextra

include make/git.mk

# Needed because bitbake overrides CFLAGS
TARGET_CFLAGS += -DVERSION=\"$(VERSION)\"

all: eviewitf

.PHONY: eviewitf
eviewitf: $(BUILDDIR)/src/main.o libewiewitf
	$(CC) $(CFLAGS) $< -o $(BUILDDIR)/$@ -l$@ -lrt -lpthread -L$(BUILDDIR)

LIBDEPS = $(BUILDDIR)/src/eviewitf.o
LIBDEPS += $(BUILDDIR)/src/eviewitf_app.o
LIBDEPS += $(BUILDDIR)/src/eviewitf_blender.o
LIBDEPS += $(BUILDDIR)/src/eviewitf_camera.o
LIBDEPS += $(BUILDDIR)/src/eviewitf_camera_seek.o
LIBDEPS += $(BUILDDIR)/src/eviewitf_device.o
LIBDEPS += $(BUILDDIR)/src/eviewitf_ssd.o
LIBDEPS += $(BUILDDIR)/src/eviewitf_streamer.o
LIBDEPS += $(BUILDDIR)/src/mfis_communication.o

.PHONY: libewiewitf
libewiewitf: $(LIBDEPS)
	@mkdir -p $(BUILDDIR)
	$(AR) rcs $(BUILDDIR)/libeviewitf.a $^

$(BUILDDIR)/%.o : %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TARGET_CFLAGS) -c $< $(INC) -o $@

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

.PHONY: deploy
deploy: install
	scp build/eviewitf root@$(TARGETIP):/usr/bin/

.PHONY: version
version:
	@echo $(VERSION)

.PHONY: ipk
ipk: eviewitf
	scripts/build_ipk.sh $(VERSION)

CLANG_FORMAT_DIRS = include src

.PHONY: clangformat
clangformat:
	@find $(CLANG_FORMAT_DIRS) -iname *.h -o -iname *.c | xargs clang-format --style=file -i

.PHONY: clangcheck
clangcheck:
	@find $(CLANG_FORMAT_DIRS) -iname *.h -o -iname *.c -exec cat {} \; \
		| diff -u <(find $(CLANG_FORMAT_DIRS) -iname *.h -o -iname *.c -exec \
		clang-format --style=file {} \;) -

.PHONY: docclean
docclean:
	$(MAKE) -C $(PWD)/doc clean

.PHONY: doc
doc: docclean
	$(MAKE) -C $(PWD)/doc all

.PHONY: package
package: doc ipk
	scripts/build_package.sh $(VERSION)
