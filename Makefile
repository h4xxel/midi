# Project: midi

CC   = gcc
LIBS =  -L"/usr/lib"
INCS =  -I"/usr/include" 
BIN  = midi
DEBUGFLAGS = -g -D__DEBUG__
CFLAGS = $(INCS) -c
LDFLAGS = 
RM = rm -f

OBJFILES = pcspkr.o main.o midi.o timing.o
.PHONY: all clean

all: $(OBJFILES)
	@echo " [ CC ]	$(OBJFILES)"
	@$(CC) $(OBJFILES) -o $(BIN) $(LIBS)
	@echo " Done."
	@echo

clean:
	@echo ""
	@echo " [ RM ]	$(OBJFILES)"
	@echo " [ RM ]	$(BIN)"
	@$(RM) $(OBJFILES)
	@$(RM) $(BIN)
	@echo " Done."
	@echo

%.o: %.c %.h
	@echo " [ CC ]	$<"
	@$(CC) $(CFLAGS) -o $@ $< 
