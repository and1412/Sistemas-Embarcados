SRCDIR=src
BINDIR=bin
INCDIR=include
OBJDIR=obj

CC=gcc
CFLAGS= -W -Wall -I $(INCDIR)

TARGET=experimento1

_HEADER=experimento1.h
_OBJ=experimento1.o main_experimento1.o

HEADER=$(patsubst %,$(INCDIR)/%,$(_HEADER))
OBJ=$(patsubst %,$(OBJDIR)/%,$(_OBJ))


################# TARGETS #####################

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADER)
	$(CC) $(CFLAGS) -c -o $@ $< -lpthread


$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ -lpthread

help: 
	$(info )
	$(info <<<<Attention:>>>> )
	$(info --------Compile the program using the command: make --------)
	$(info --------After the installation, run the program with the command: ./bin/experimento1 <number> --------)
	$(info )
	
clean:
	rm -f $(OBJDIR)/*
	rm -f $(BINDIR)/*
	rm -f $(SRCDIR)/*~
	rm -f $(INCDIR)/*~
