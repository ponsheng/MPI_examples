
PROC = 3

EXE= odd-even

$(EXE): main.c
	mpiicc -g  -std=c99 -Wall -o $(EXE) main.c

run: $(EXE)
	mpiexec -n $(PROC) ./$(EXE)

boot:
	mpdboot --file=../mpd.hosts -n 4

trace:
	mpdtrace

clean:
	rm $(EXE)
