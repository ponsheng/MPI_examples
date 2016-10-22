
PROC=8
EXE= smooth
NSmooth=1

$(EXE): main.cpp
	mpiicc -g -Wall -o $(EXE) $< -DNSmooth=$(NSmooth)

origin: smooth.cpp
	mpiicc -g -Wall -o $@ $< -DNSmooth=$(NSmooth)
	mpiexec -n 1 ./origin

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
