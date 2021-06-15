.PHONY: all docuser docfull clean

all: docuser docfull

docuser:
	doxygen doxyfile-user
	make -C user/latex pdf

docfull:
	doxygen doxyfile-full

clean:
	rm -Rf user/ full/

