#Program made by Protofall for anyone to use free of charge

CPPFLAGS = --std=gnu++11

CC = g++
PROG = VmuLcdIconCreator
EXT = 

CPPFILES = $(wildcard *.cpp)
OBJS = $(CPPFILES:%.cpp=%.o)

.PHONY: all clean

all: clean $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) --std=gnu++11

#stb_image.h requires c++11
%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm -f *.o $(PROG)$(EXT)
	rm -f *.bin

example:
	./$(PROG) "icon_VMU_LCD.png" "LCD.bin" "--invert"
