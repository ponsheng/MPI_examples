
PROC = 16
EXE= circuit
COMM = 1 #1  SERIAL 2 TREE

$(EXE): main.c
	mpiicc -g -Wall -DCOMM=$(COMM) -o $(EXE) main.c

run: $(EXE)
	mpiexec -n $(PROC) ./$(EXE)

boot:
	mpdboot -n 4

trace:
	mpdtrace

clean:
	rm $(EXE)
