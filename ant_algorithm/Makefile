
EXE = ant

all: $(EXE)

$(EXE):	main.c
	gcc $^ -o $@ -g -Wall

run: $(EXE)
	./$<

clean:
	rm -f $(EXE)
