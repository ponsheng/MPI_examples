CC = c++

EXEC = smooth
EXEC_ORIG = smooth_orig

OUTPUT = output.bmp
OUTPUT_ORIG = output_origin.bmp

CFLAGS := -DNSmooth=10

all:$(EXEC_ORIG) $(EXEC)

$(EXEC): main.cpp
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

$(EXEC_ORIG): $(EXEC_ORIG).cpp
	$(CC) $^ -o $@ $(CFLAGS)

$(OUTPUT): $(EXEC)
	./$(EXEC)

$(OUTPUT_ORIG): $(EXEC_ORIG)
	./$(EXEC_ORIG)

check: $(OUTPUT) $(OUTPUT_ORIG)
	@diff  $(OUTPUT) $(OUTPUT_ORIG) || (echo -e "\033[31mFail\033[0m"; exit 1);
	@echo -e "\033[32mVerified OK!!\033[0m" ;

clean:
	rm -f $(EXEC) $(EXEC_ORIG) $(OUTPUT) $(OUTPUT_ORIG)

