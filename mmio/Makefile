STUDENT_ID=2522979

all:
	gcc -g read_write.c -o read_write
	gcc -g memmap.c -o memmap

clean:
	rm -f *.o read_write memmap copy.ogg

test:
	./memmap sample.ogg copy.ogg
	diff sample.ogg copy.ogg

tar: clean
	make clean
	mkdir $(STUDENT_ID)-mmio-lab
	cp Makefile memmap.c read_write.c $(STUDENT_ID)-mmio-lab/
	tar cvzf $(STUDENT_ID)-mmio-lab.tar.gz $(STUDENT_ID)-mmio-lab
	rm -rf $(STUDENT_ID)-mmio-lab

