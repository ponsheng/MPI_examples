
PROC=2
EXE= smooth

$(EXE): main.cpp
	mpiicc -g -Wall -o $(EXE) main.cpp

run: $(EXE)
	mpiexec -n $(PROC) ./$(EXE)
	diff origin.bmp output2.bmp

boot:
	mpdboot -n 4

check: run
	diff origin.bmp output2.bmp

trace:
	mpdtrace

clean:
	rm $(EXE)
