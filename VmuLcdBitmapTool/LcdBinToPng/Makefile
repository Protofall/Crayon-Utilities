
EXE = LcdBinToPng
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
	./$(EXE) --input bins/a.bin --output b.png  --mode 0
