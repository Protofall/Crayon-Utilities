
EXE = VmuLcdIconCreator
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
	./$(EXE) --input images/icon_VMU_LCD.png --output-binary vmu_lcd1.bin --output-png preview1.png --invert	#For displaying via DC controller
	./$(EXE) --input images/icon_VMU_LCD.png --output-binary vmu_lcd2.bin --output-png preview2.png			#For displaying in handheld mode
