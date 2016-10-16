
EXE= smooth

$(EXE): main.cpp
	mpiicc -g -Wall -o $(EXE) main.cpp

run: $(EXE)
	mpiexec -n $(PROC) ./$(EXE)

boot:
	mpdboot -n 4

trace:
	mpdtrace

clean:
	rm $(EXE)
