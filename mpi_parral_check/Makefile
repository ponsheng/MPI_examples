
PROC = 8 
EXE= mpi_test 

$(EXE): main.c
	mpiicc -g -Wall -o $(EXE) main.c

run: $(EXE)
	mpiexec -n $(PROC) --hostfile ../mpd.hosts ./$(EXE)

boot:
	mpdboot --file=../mpd.hosts -n 4

trace:
	mpdtrace

clean:
	rm $(EXE)
