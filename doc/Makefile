.PHONY: all docuser docfull clean

all: docuser docfull

docuser:
	doxygen doxyfile-user

docfull:
	doxygen doxyfile-full
	make -C user/latex pdf

clean:
	rm -Rf html/ latex/

