EXE = ant
CFLAGS =  -g -std=gnu99 -openmp
PROC = 4

ifeq ($(city),17)
INPUT_FILE = "\"cities/gr17_d.txt\""
N  = 17
else 

ifeq ($(city),26)
INPUT_FILE = "\"cities/fri26_d.txt\""
N  = 26
else

ifeq ($(city),42)
INPUT_FILE = "\"cities/dantzig42_d.txt\""
N  = 42                                                                   
else

ifeq ($(city),48)
INPUT_FILE = "\"cities/att48_d.txt\""
N  = 48
else
INPUT_FILE = "\"cities/fri26_d.txt\""
N  = 26
endif
endif
endif
endif

all:	
	@echo "Use file: $(INPUT_FILE)"
	mpiicc main.c -o $(EXE)  $(CFLAGS) -DINPUT_FILE=$(INPUT_FILE) -DN=$(N)

run:
	mpiexec -n $(PROC)  ./$(EXE)

boot:
	mpdboot --file=../mpd.hosts -n 4

clean:
	rm -f $(EXE)
