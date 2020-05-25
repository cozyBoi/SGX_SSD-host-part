CC = g++

spm: spm.cpp ctl_path_lib.o
	$(CC) -o spm spm.cpp ctl_path_lib.o

ctl_path_lib.o: ctl_path_lib.cpp
	g++ -c ctl_path_lib.cpp

clean:
	rm -rf spm ctl_path_lib.o