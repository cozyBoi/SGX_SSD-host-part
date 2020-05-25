APP = spm

spm: spm.cpp ctl_path_lib.o
	g++ -o spm.cpp ctl_path_lib.o

ctl_path_lib.o: ctl_path_lib.cpp
	g++ -c ctl_path_lib.cpp