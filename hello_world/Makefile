
PROC = 8 
EXE= hello

all: $(EXE).c
	mpiicc -g -Wall -o $(EXE) $(EXE).c

run:
	mpiexec -n $(PROC) ./$(EXE)

boot:
	mpdboot -n 4

trace:
	mpdtrace
