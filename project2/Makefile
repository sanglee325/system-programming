CC = gcc
CFLAGS = -Wall -g

SOURCE = 20171635.c command.c  input.c	assemble.c
TARGET = 20171635.out
OBJ = $(SOURCE:.c=.o)

$(TARGET) : $(OBJ) 20171635.h
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

debug : $(TARGET)
	gdb $(TARGET)

clean : 
	rm $(OBJ)

fcheck:
	./freeChecker.sh ./$(TARGET)
