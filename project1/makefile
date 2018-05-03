# the compiler: gcc for C program, define as g++ for C++
  CC = gcc

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -g -Wall -std=c99 -lpthread


  # the build target executable:

  all: lab 

  lab: tictactoe 
    
  tictactoe: 
	$(CC) $(CFLAGS) -o tictactoeClient  coreFunction.c tictactoe.c client.c
	$(CC) $(CFLAGS) -o tictactoeServer  coreFunction.c tictactoe.c server.c


  clean:
	rm tictactoeClient
	rm tictactoeServer
