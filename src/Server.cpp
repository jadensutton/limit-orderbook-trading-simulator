#include <iostream>
#include <string.h>
#include <thread>
#include <sys/msg.h>

#include "Server.h"
#include "MatchingEngine.h"
#include "Order.h"
#include "Parser.h"

int Order::idCounter = 0;
	
int main() {
	MatchingEngine matchingEngine;
	Parser parser;
	
	int serverMsgId;
	int i;
	int res;
	struct MsgSt msg;
	char *token;
	
	serverMsgId = msgget((key_t)SERVER_MQUEUE, 0666 | IPC_CREAT);
	if (serverMsgId == -1) {
		std::cout << "Failed to get server message queue ID" << std::endl;
		return 1;
	}
	
	// Create threads to continuously poll bid and ask queues
	std::thread bidsThread(&MatchingEngine::pollBidQueue, &matchingEngine);
	std::thread asksThread(&MatchingEngine::pollAskQueue, &matchingEngine);
	
	while (1) {
		// Read from message queue
		if (msgrcv(serverMsgId, (void *)&msg, MAX_TEXT, 1, 0) == -1) {
			std::cout << "Failed to receive message" << std::endl;
			return 1;
		}
		
		std::cout << "[SERVER] Received request from client: " << msg.text << std::endl;
		
		// Parse request and check for error
		struct ParseResult parseResult = parser.parseOrder(msg.text);
		if (parseResult.error) {
			std::cout << "[SERVER] Error parsing request" << std::endl;
			// Send response to client if clientId was successfully parsed
			if (parseResult.clientId) {
				msg.type = parseResult.clientId;
				strcpy(msg.text, "Error parsing request");
				
				if (msgsnd(serverMsgId, (void *)&msg, MAX_TEXT, 0) == -1) {
					std::cout << "Failed to send order to server" << std::endl;
					return 1;
				}	
			}
			continue;
		} else if (parseResult.requestType == NEW_ORDER) {
			std::cout << "[SERVER] Adding order to queue" << std::endl;
			matchingEngine.placeOrder(parseResult.order);	// Send order to matching engine
			std::sprintf(msg.text, "Order Received (Order ID = %d)", parseResult.order->getId());
		} else if (parseResult.requestType == CANCEL_ORDER) {
			std::cout << "[SERVER] Cancelling order" << std::endl;
			CancelOrderResponse res = matchingEngine.cancelOrder(parseResult.clientId, parseResult.orderId);
			if (res == ORDER_CANCELLED) {
				std::sprintf(msg.text, "Order Cancelled (Order ID = %d)", parseResult.orderId);
			} else if (res == ORDER_NOT_FOUND) {
				std::sprintf(msg.text, "No order found with ID=%d", parseResult.orderId);
			} else if (res == CLIENT_NOT_AUTHORIZED) {
				std::sprintf(msg.text, "You do not have authorization to cancel order with ID=%d", parseResult.orderId);
			}
		}
		
		
		std::cout << "[SERVER] Responding to client (Client ID=" << parseResult.clientId << ")" << std::endl;
		// Send response to client
		msg.type = parseResult.clientId;
		if (msgsnd(serverMsgId, (void *)&msg, MAX_TEXT, 0) == -1) {
			std::cout << "Failed to send order to server" << std::endl;
			return 1;
		}
	}
}
