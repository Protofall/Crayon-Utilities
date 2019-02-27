
EXE = VmuSfIconCreator
EXT =
CPPFLAGS = -std=gnu++11 -lpng
CFLAGS = -lpng

CPPFILES = $(wildcard *.cpp)
CFILES = $(wildcard *.c)
OBJS = $(CPPFILES:%.cpp=%.o) $(CFILES:%.c=%.o)

.PHONY: all clean run

all: clean $(EXE)

$(EXE): $(OBJS)
	gcc -o $(EXE) $(OBJS) -lpng

%.o: %.cpp
	g++ $(CPPFLAGS) -c $<

%.o: %.c
	echo $(OBJS)
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o $(EXE)$(EXT)
	rm -f *.bin
	rm -f *.BIN
	rm -f *.png

run:
	./VmuSfIconCreator --input-image textures/single_test.png textures/twelve_test.png --output-image IMAGE.BIN --output-palette PALLETTE.BIN


