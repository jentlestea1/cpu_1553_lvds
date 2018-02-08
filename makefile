vpath %.c ./1553B/
vpath %.c ./lvds/
vpath %.c ./task/
vpath %.c ./
vpath %.h ./1553B/
vpath %.h ./lvds/
vpath %.h ./task/
vpath %.h ./

CCFLAGS = -I ./1553B/ -I ./lvds -I ./task -I ./ -DDEBUG
ENDFLAGS = -std=c89 -pedantic -Wall -Wstrict-prototypes -Wno-missing-braces -O6 -lm -lpthread
CC = sparc-elf-gcc
OBJ = main.o bus1553.o bus1553base.o bus1553lib.o do_frame.o frame.o \
	  lvds.o task.o img_clouddetect_parallel.o

main : $(OBJ)
	$(CC) -o main $(OBJ) $(ENDFLAGS)
	mv ./*.o ./obj
$(OBJ): %.o: %.c
	$(CC) -c $(CCFLAGS) $< -o $@

.PHONY : clean
clean:
	rm main ./obj/*.o ./*.o
