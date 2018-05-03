# lab4-yujie-yao

**Compile:**

	make

switch ROBOT_ON option in server.c and client.c between 0 and 1 can disable or enable automatical play. Server autoplay is enabled by default.

	#define  ROBOT_ON 0
**Run:**

***Player 1***:

	./tictactoe <port> <play number: 1>

***Player 2***:

	./tictactoe <port> <play number: 2> <ip address>

	
player 1 is the server, player 2 is client, player 2 moves first.

