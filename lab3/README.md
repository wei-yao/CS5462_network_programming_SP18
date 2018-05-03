**Description**

Play a tic-tac-toe game between server( play 1) and client ( player2 ).

**Compile:**

	make
	
switch ROBOT_ON option in server.c and client.c between 0 and 1 can disable or enable automatical play.

	#define  ROBOT_ON 0

**Run:**

	./server \<port>

	./client <server-ip> <port>
