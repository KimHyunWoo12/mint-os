all: ImageMaker.exe mov

ImageMaker.exe: ImageMaker.c
	gcc -o $@ $<

mov:
	mv ImageMaker.exe ../../

clean:
	rm -f ImageMaker.exe
