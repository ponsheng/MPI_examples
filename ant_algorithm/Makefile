EXE = ant

CFLAGS = -Wall -g -std=gnu99 -fopenmp

all: $(EXE)

$(EXE):	main.c
	gcc $^ -o $@  $(CFLAGS)

run: $(EXE)
	./$<

clean:
	rm -f $(EXE)
