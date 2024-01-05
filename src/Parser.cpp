#include <string.h>
#include <stdlib.h>

#include "Parser.h"
#include "Order.h"

#define NULL nullptr
#define EXPECTED_TOKENS 4

Parser::Parser() {
	id = 0;
}

/*
Parse a request string and return an Order object
*/
ParseResult Parser::parseOrder(char request[MAX_TEXT]) {
	struct ParseResult result;
	result.clientId = 0;
		
	int clientId;
	OrderType type;
	int qty;
	int limit;
			
	int i = 0;
	bool validTokens = true;
	bool receivedClientId = false;
	char *token = strtok(request, " ");
	while (token != NULL) {
		switch (i) {
			case 0:
				// Client
				clientId = atoi(token);
				result.clientId = clientId;
				break;
			case 1:
				// Order type
				if (strlen(token) == 3 && strncmp(token, "BUY", 3) == 0) {
					type = OrderType::LIMIT_BUY;
				} else if (strlen(token) == 4 && strncmp(token, "SELL", 4) == 0) {
					type = OrderType::LIMIT_SELL;
				} else {
					validTokens = false;
				}
				break;
			case 2:
				// Qty
				qty = atoi(token);
				break;
			case 3:
				// Limit
				// To improve performance, we represent limit prices using minor currency units represented as an integer. Must convert user provided double into integer
				limit = (int) 100 * atof(token);
				break;
		}
		
		i += 1;
		token = strtok(NULL, " ");
	}
	
	// If request is invalid
	if (i > EXPECTED_TOKENS) {
		result.error = 1;
	} else if (i < EXPECTED_TOKENS) {
		result.error = 2;
	} else if (!validTokens) {
		result.error = 3;
	} else {
		result.error = 0;
		result.order = new Order(id, type, limit, qty, clientId);	// Create a new order object with provided information;
	}
	
	return result;	
}

