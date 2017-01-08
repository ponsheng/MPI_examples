EXE = count_sort
EXE_ORIG = count_sort_orig
EXE_QSORT = qsort

$(EXE): main.c count_sort.c
	gcc -std=c99 -Wall -fopenmp $^ -o $@

$(EXE_ORIG): main.c origin.c
	gcc -std=c99 $^ -o $@

$(EXE_QSORT): main.c qsort.c
	gcc -std=c99 $^ -o $@

clean:
	rm -f $(EXE) $(EXE_ORIG) $(EXE_QSORT)
