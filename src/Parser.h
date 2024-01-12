#include <unordered_map>
#include "Server.h"
#include "Order.h"

#ifndef Parser_H
#define Parser_H

enum RequestType {
	NEW_ORDER,
	CANCEL_ORDER
};

struct ParseResult {
	int error;	// 0 = parse successful, 1 = too many tokens, 2 = too few tokens, 3 = invalid token values
	RequestType requestType;
	int clientId;
	int orderId;
	Order* order;
};

class Parser {
	std::unordered_map<RequestType, int> expectedTokens = {
		{NEW_ORDER, 4},
		{CANCEL_ORDER, 3}
	};
	
	public:
		ParseResult parseOrder(char request[MAX_TEXT]);

};
#endif
