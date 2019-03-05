
EXE = VmuSfIconCreator
EXT =
CPPFLAGS = -std=gnu++11 -lpng -lm
CFLAGS = -lpng -lm

CPPFILES = $(wildcard *.cpp)
CFILES = $(wildcard *.c)
OBJS = $(CPPFILES:%.cpp=%.o) $(CFILES:%.c=%.o)

.PHONY: all clean run warn_test

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
	./VmuSfIconCreator --input-image textures/single_test1.png textures/single_test2.png textures/single_test3.png --output-image IMAGE1.BIN --output-palette PALLETTE1.BIN
	./VmuSfIconCreator --input-image textures/three_test.png --output-image IMAGE2.BIN --output-palette PALLETTE2.BIN
	./VmuSfIconCreator --input-image textures/lines.png --output-image IMAGE3.BIN --output-palette PALLETTE3.BIN	#Has 28 colours, colour count is reduced

warn_test:
	./VmuSfIconCreator --input-image textures/single_test1.png textures/three_test.png --output-image IMAGE_W.BIN --output-palette PALLETTE_W.BIN


