LDFLAGS=
INC = -I include

all:
	$(CC) -c src/mfis_api.c $(INC) -o src/mfis_api.o 
	$(CC) -c src/mfis_driver_communication.c $(INC) -o src/mfis_driver_communication.o

	ar rcs libmfis.a src/mfis_driver_communication.o src/mfis_api.o

.PHONY:	clean
clean:
	@find . -name "*.o" -exec rm -f '{}' \;
	@rm -rf libmfis.a
