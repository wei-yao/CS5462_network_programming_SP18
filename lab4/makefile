# the compiler: gcc for C program, define as g++ for C++
  CC = gcc

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -g -Wall -std=c99


  # the build target executable:

  all: lab 

  lab: tictactoe 
    
  tictactoe: 
	$(CC) $(CFLAGS) -o tictactoe  tictactoe.c client.c server.c main.c
	

  clean:
	rm tictactoe
