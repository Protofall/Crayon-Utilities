
EXT =
CPPFLAGS = -std=gnu++11 -lpng

CC = g++
EXE = DtexToRGBA8888

CPPFILES = $(wildcard *.cpp)
OBJS = $(CPPFILES:%.cpp=%.o)

.PHONY: all clean run dtex_examples

all: clean dtex_examples $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) -std=gnu++11 -lpng

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm -f *.o $(EXE)$(EXT)
	rm -f textures/*.png
	rm -f textures/*.dtex
	rm -f textures/*.dtex.pal
	rm -f *.bin

run:
	./DtexToRGBA8888 textures/cow_RGB565.dtex --binary lol.bin --png textures/cow_RGB565.png
	# ./DtexToRGBA8888 textures/cow_RGB565_m.dtex --png textures/cow_RGB565_m.png
	./DtexToRGBA8888 textures/cow_YUV422.dtex --png textures/cow_YUV422.png
	./DtexToRGBA8888 textures/cow_YUV422_c.dtex --png textures/cow_YUV422_c.png
	./DtexToRGBA8888 textures/cow_YUV422_s.dtex --png textures/cow_YUV422_s.png

	./DtexToRGBA8888 textures/dwarfy_ARGB1555.dtex --png textures/dwarfy_ARGB1555.png
	./DtexToRGBA8888 textures/dwarfy_ARGB1555_c.dtex --png textures/dwarfy_ARGB1555_c.png	#New untested compressed option
	./DtexToRGBA8888 textures/dwarfy_RGB565.dtex --png textures/dwarfy_RGB565.png
	./DtexToRGBA8888 textures/dwarfy_ARGB4444.dtex --png textures/dwarfy_ARGB4444.png
	./DtexToRGBA8888 textures/dwarfy_YUV422.dtex --png textures/dwarfy_YUV422.png

	./DtexToRGBA8888 textures/insta_RGB565.dtex --png textures/insta_RGB565.png
	./DtexToRGBA8888 textures/insta_PAL4BPP.dtex --png textures/insta_PAL4BPP.png
	./DtexToRGBA8888 textures/insta_PAL8BPP.dtex --png textures/insta_PAL8BPP.png
	./DtexToRGBA8888 textures/insta_PAL4BPP_c.dtex --png textures/insta_PAL4BPP_c.png
	./DtexToRGBA8888 textures/insta_PAL8BPP_c.dtex --png textures/insta_PAL8BPP_c.png

	./DtexToRGBA8888 textures/lines_PAL8BPP.dtex --png textures/lines_PAL8BPP.png

	./DtexToRGBA8888 textures/simpleRect_RGB565.dtex --png textures/simpleRect_RGB565.png
	./DtexToRGBA8888 textures/simpleRect_PAL4BPP.dtex --png textures/simpleRect_PAL4BPP.png

	./DtexToRGBA8888 textures/strideTest_RGB565.dtex --png textures/strideTest_RGB565.png

dtex_examples:
	texconv -i "textures/source/cow.png" -o "textures/cow_RGB565.dtex" -f "RGB565"
	texconv -i "textures/source/cow.png" -o "textures/cow_RGB565_m.dtex" -f "RGB565" -mipmap
	texconv -i "textures/source/cow.png" -o "textures/cow_YUV422.dtex" -f "YUV422"
	texconv -i "textures/source/cow.png" -o "textures/cow_YUV422_c.dtex" -f "YUV422" -compress	#Despite the Sega docs saying you can't compress YUV422, you can
	texconv -i "textures/source/cow.png" -o "textures/cow_YUV422_s.dtex" -f "YUV422" -stride 4	#128/32 = 4

	texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_ARGB1555.dtex" -f "ARGB1555"
	texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_ARGB1555_c.dtex" -f "ARGB1555" -compress
	texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_RGB565.dtex" -f "RGB565"
	texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_ARGB4444.dtex" -f "ARGB4444"
	texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_YUV422.dtex" -f "YUV422"

	texconv -i "textures/source/insta.png" -o "textures/insta_RGB565.dtex" -f "RGB565"
	texconv -i "textures/source/insta.png" -o "textures/insta_RGB565_c.dtex" -f "RGB565" -compress
	texconv -i "textures/source/insta.png" -o "textures/insta_PAL4BPP.dtex" -f "PAL4BPP"
	texconv -i "textures/source/insta.png" -o "textures/insta_PAL8BPP.dtex" -f "PAL8BPP"
	texconv -i "textures/source/insta.png" -o "textures/insta_PAL4BPP_c.dtex" -f "PAL4BPP" -compress	#Compression works slightly differently for 4 and 8bpp
	texconv -i "textures/source/insta.png" -o "textures/insta_PAL8BPP_c.dtex" -f "PAL8BPP" -compress

	texconv -i "textures/source/lines.png" -o "textures/lines_PAL8BPP.dtex" -f "PAL8BPP"

	texconv -i "textures/source/simpleRect.png" -o "textures/simpleRect_RGB565.dtex" -f "RGB565"	#Showing non-square textures work
	texconv -i "textures/source/simpleRect.png" -o "textures/simpleRect_PAL4BPP.dtex" -f "PAL4BPP"

	texconv -i "textures/source/strideTest.png" -o "textures/strideTest_RGB565.dtex" -f "RGB565" -stride 3	#96/32 = 3. Note stride only applies to width (Height of 96 doesn't work)
