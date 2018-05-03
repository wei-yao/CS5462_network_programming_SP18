# the compiler: gcc for C program, define as g++ for C++
  CC = gcc

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -g -Wall -std=c99


  # the build target executable:

  all: lab3 

  lab3: server client
    
  client: client.c	
	$(CC) $(CFLAGS) -o client tictactoe.c  client.c
	
  server: server.c	
	$(CC) $(CFLAGS) -o server tictactoe.c server.c

  clean:
	$(RM) $(TARGET)
	rm client server
