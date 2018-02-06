vpath %.c ./1553B/
vpath %.c ./lvds/
vpath %.c ./task/
vpath %.c ./
vpath %.h ./1553B/
vpath %.h ./lvds/
vpath %.h ./task/
vpath %.h ./

CCFLAGS = -I ./1553B/ -I ./lvds -I ./task -I ./ -DDEBUG

CC = sparc-elf-gcc
OBJ = main.o bus1553.o bus1553base.o bus1553lib.o do_frame.o frame.o \
	  lvds.o task.o

main : $(OBJ)
	$(CC) -o main $(OBJ)
	mv ./*.o ./obj
$(OBJ): %.o: %.c
	$(CC) -c $(CCFLAGS) $< -o $@

.PHONY : clean
clean:
	rm main ./obj/*.o ./*.o
