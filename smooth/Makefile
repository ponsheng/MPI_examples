
PROC=8
EXE= smooth

$(EXE): main.cpp
	mpiicc -g -Wall -o $(EXE) $<

run: $(EXE)
	mpiexec -n $(PROC) ./$(EXE)
	diff output1.bmp output2.bmp

boot:
	mpdboot -n 4

check: 
	diff origin.bmp output2.bmp

trace:
	mpdtrace

clean:
	rm $(EXE)
