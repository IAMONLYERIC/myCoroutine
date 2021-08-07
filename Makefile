# @file  Makefile
# @author gzy
# @version 0.1
# @brief   :A asymmetric coroutine library for C++

all:
	g++  myCoroutine.cpp  -g -c
	g++  main.cpp -g -o main myCoroutine.o
clean:
	rm -f myCoroutine.o main
