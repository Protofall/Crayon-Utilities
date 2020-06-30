
EXE = DreamcastEyecatcherTool
EXT =
CPPFLAGS = -std=gnu++11 -lpng -lm
CFLAGS = -lpng -lm -g
#Add a -g to the flags to get line number info for valgrind

CPPFILES = $(wildcard *.cpp)
CFILES = $(wildcard *.c)
OBJS = $(CPPFILES:%.cpp=%.o) $(CFILES:%.c=%.o)

.PHONY: all clean run

all: clean $(EXE)

$(EXE): $(OBJS)
	gcc -o $(EXE) $(OBJS) $(CFLAGS) 

%.o: %.cpp
	g++ $(CPPFLAGS) -c $<

%.o: %.c
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o $(EXE)$(EXT)
	rm -f *.bin
	rm -f *.BIN
	rm -f *.png

run:
	./$(EXE) --input-image textures/simple1.png --output-binary eyecatch1.bin --mode 1
	./$(EXE) --input-image textures/simple1.png --output-binary eyecatch2.bin --mode 2
	./$(EXE) --input-image textures/simple1.png --output-binary eyecatch3.bin --mode 3
	./$(EXE) --input-image textures/lines.png --output-binary eyecatch_lines2.bin --mode 2 --preview proc_lines2.png
	./$(EXE) --input-image textures/lines.png --output-binary eyecatch_lines_3.bin --mode 3 --preview proc_lines3.png	#Has 28 colours, colour count is reduced



