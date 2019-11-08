CFLAGS=-W -Wall -ansi
LDFLAGS=
INC=-I./include
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)
EXEC=main

all: $(EXEC)

main: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(INC) $(CFLAGS)


clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
