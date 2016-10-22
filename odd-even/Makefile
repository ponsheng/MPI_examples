
PROC = 8 
EXE= odd-even

$(EXE): main.c
	mpiicc -g -Wall -o $(EXE) main.c

run: $(EXE)
	mpiexec -n $(PROC) ./$(EXE)

boot:
	mpdboot -n 4

trace:
	mpdtrace

clean:
	rm $(EXE)
