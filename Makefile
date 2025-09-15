INCLUDES = -Iinclude
LINKER = -lsndfile
LIBRARY = -Llib
CC = gcc

all: autopan

sfpan: autopan.c breakpoints.c
#$(CC) autopan.c breakpoints.c -o autopan $(INCLUDES) $(LINKER)
	$(CC) autopan.c breakpoints.c -o sfpan $(INCLUDES) $(LIBRARY) $(LINKER)
# For macOS Apple M-series users, you need to comment out line #10 and uncomment line #10
# You must use a tab (click the tab key on your keyboard) for indent!!!


#run the executable file
run: autopan
	./autopan

run2: autopan
	./autopan Brahms.wav Brahms_sine.wav 1 1 0 sine

run3: autopan
	./autopan Brahms.wav Brahms_saw.wav 0.8 0.6 1.8 sawtooth

run4: autopan
	./autopan Brahms.wav Brahms_triangle.wav 1 2 0.5 triangle

run5: autopan
	./autopan Brahms.wav Brahms_square.wav 0.75 1 3.14 square

run6: autopan
	./autopan Brahms.wav Brahms_random.wav 0.8 0.2 0 random

# delete the executable file
clean: 
	rm autopan
	
