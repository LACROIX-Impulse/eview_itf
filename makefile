LDFLAGS=
INC = -I include
BUILDDIR = build

all:
	$(CC) -c src/mfis_api.c $(INC) -o src/mfis_api.o 
	$(CC) -c src/mfis_driver_communication.c $(INC) -o src/mfis_driver_communication.o
	mkdir -p $(BUILDDIR)
	ar rcs $(BUILDDIR)/libmfis.a src/mfis_driver_communication.o src/mfis_api.o

.PHONY:	clean
clean:
	@find . -name "*.o" -exec rm -f '{}' \;
	@rm -rf $(BUILDDIR)
