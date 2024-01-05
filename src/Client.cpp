#include <iostream>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>

#include "Server.h"

int main() {
	int id = getpid();	// Use PID to represent the client ID
	int serverMsgId;
	struct MsgSt msg;
	
	serverMsgId = msgget((key_t)SERVER_MQUEUE, 0666 | IPC_CREAT);
	if (serverMsgId == -1) {
		std::cout << "Failed to get server message queue ID" << std::endl;
		return 1;
	}
	
	char input[32];
	while (1) {
		// Get user input
		std::cout << ">>> ";
		std::cin.getline(input, 32);
		
		// Set message fields
		msg.type = 1;
		sprintf(msg.text, "%i %s", id, input);
		
		// Write to message queue
		if (msgsnd(serverMsgId, (void *)&msg, MAX_TEXT, 0) == -1) {
			std::cout << "Failed to send order to server" << std::endl;
			return 1;
		}
		
		// Read from message queue
		if (msgrcv(serverMsgId, (void *)&msg, MAX_TEXT, id, 0) == -1) {
			std::cout << "Failed to receive message" << std::endl;
			return 1;
		}
		
		std::cout << "Received response from server: " << msg.text << std::endl;
	}
}
