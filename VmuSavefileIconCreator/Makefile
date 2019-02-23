
EXE = VmuSavefileIconCreator
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
	rm -f textures/*.png
	rm -f textures/*.dtex
	rm -f textures/*.dtex.pal
	rm -f *.bin

run:
	echo WIP

