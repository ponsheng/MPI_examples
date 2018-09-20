PROC=1
MPI_SMOOTH=mpi_smooth
SMOOTH=smooth
INPUT=input.bmp

NSmooth=10

SRC= main.c qdbmp/qdbmp.c
flags= -I qdbmp

all: $(MPI_SMOOTH) $(SMOOTH)

$(MPI_SMOOTH): % : %.c $(SRC)
	mpicc $(flags) -o $@ $^ -DNSmooth=$(NSmooth)

$(SMOOTH): % : %.c $(SRC)
	mpicc $(flags) -o $@ $^ -DNSmooth=$(NSmooth)


output1.bmp: $(SMOOTH)
	mpiexec -n 1 ./$< $(INPUT) $@

output2.bmp: $(MPI_SMOOTH)
	mpiexec -n $(PROC) ./$< $(INPUT) $@

check: output1.bmp output2.bmp
	@diff  output1.bmp output2.bmp || (echo "Fail"; exit 1);
	@echo "OK" ;

boot:
	mpdboot -n 4

trace:
	mpdtrace

clean:
	rm -f  $(MPI_SMOOTH) $(SMOOTH) output1.bmp output2.bmp
