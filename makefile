# makefile
test: snp.c
	gcc snp.c -o snp
clean: rm -f *.o *.~ snp *.gch
