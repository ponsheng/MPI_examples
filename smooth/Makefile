
PROC=4
EXE= smooth
NSmooth=500

$(EXE): main.cpp
	mpic++ -g -Wall -o $(EXE) $< -DNSmooth=$(NSmooth)

origin: smooth.cpp
	mpic++ -g -Wall -o $@ $< -DNSmooth=$(NSmooth)

output1.bmp: origin
	mpiexec -n 1 ./origin

run: $(EXE) output1.bmp
	mpiexec -n $(PROC) ./$(EXE)
	@diff  output1.bmp output2.bmp || (echo -e "\033[31mFail\033[0m"; exit 1);
	@echo -e "\033[32mVerified OK!!\033[0m" ;

boot:
	mpdboot -n 4

check: 
	diff origin.bmp output2.bmp

trace:
	mpdtrace

clean:
	rm -f  $(EXE) origin output1.bmp output2.bmp
