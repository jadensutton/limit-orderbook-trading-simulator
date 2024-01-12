#include <unordered_map>
#include <string.h>
#include <stdlib.h>

#include "Parser.h"
#include "Order.h"

#define EXPECTED_TOKENS 4

/*
Parse a request string
@param request - Client request
@return request status and contents
*/
ParseResult Parser::parseOrder(char request[MAX_TEXT]) {
	struct ParseResult result;
	result.clientId = 0;
		
	OrderType orderType;
	int qty;
	int limit;
			
	int i = 0;
	bool validTokens = true;
	bool receivedClientId = false;
	char *token = strtok(request, " ");
	while (token != nullptr) {
		switch (i) {
			case 0:
				// Client
				result.clientId = atoi(token);
				break;
			case 1:
				// Request type
				if (strlen(token) == 3 && strncmp(token, "BUY", 3) == 0) {
					result.requestType = NEW_ORDER;
					orderType = LIMIT_BUY;
				} else if (strlen(token) == 4 && strncmp(token, "SELL", 4) == 0) {
					result.requestType = NEW_ORDER;
					orderType = LIMIT_SELL;
				} else if (strlen(token) == 6 && strncmp(token, "CANCEL", 6) == 0) {
					result.requestType = CANCEL_ORDER;
				} else {
					validTokens = false;
				}
				break;
			case 2:
				// Qty or Order ID
				if (result.requestType == NEW_ORDER) {
					qty = atoi(token);
				} else if (result.requestType == CANCEL_ORDER) {
					result.orderId = atoi(token);
				}
				break;
			case 3:
				// Limit
				// To improve performance, we represent limit prices using minor currency units represented as an integer. Must convert user provided double into integer
				limit = (int) 100 * atof(token);
				break;
		}
		
		i += 1;
		token = strtok(nullptr, " ");
	}
	
	// If request is invalid
	if (!validTokens) {
		result.error = 3;
	} else if (i > expectedTokens[result.requestType]) {
		result.error = 1;
	} else if (i < expectedTokens[result.requestType]) {
		result.error = 2;
	} else {
		if (result.requestType == NEW_ORDER) {
			result.error = 0;
			result.order = new Order(orderType, limit, qty, result.clientId);	// Create a new order object with provided information;
		}
	}
	
	return result;	
}

