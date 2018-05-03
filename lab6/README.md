# lab6-yujie-yao

**Compile:**

	make

switch ROBOT_ON option in  client.c between 0 and 1 can disable or enable automatical play. 

	#define  ROBOT_ON 0
	

**Run:**
***Client***:

	./tictactoeClient <port> <ip>

***Server***:

	./tictactoeServer <port> 

	
player 1 is the server, player 2 is client, player 1 moves first.
Max number of game supported can be changed by `GAME_NUM` macro in server.c, default is two.

