
EXE = VmuEyeCatchCreator
EXT =
CPPFLAGS = -std=gnu++11 -lpng -lm
CFLAGS = -lpng -lm

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
	./VmuEyeCatchCreator --input-image textures/simple1.png --output-binary IMAGE1.BIN --mode 1
	./VmuEyeCatchCreator --input-image textures/lines.png --output-binary IMAGE3.BIN --mode 3 --preview proc_lines.png	#Has 28 colours, colour count is reduced



