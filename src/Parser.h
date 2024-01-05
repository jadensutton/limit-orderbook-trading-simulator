#include "Server.h"
#include "Order.h"

#ifndef Parser_H
#define Parser_H

struct ParseResult {
	int error;	// 0 = parse successful, 1 = too many tokens, 2 = too few tokens, 3 = invalid token values
	int clientId;
	Order* order;
};

class Parser {
	public:
		ParseResult parseOrder(char request[MAX_TEXT]);

};
#endif
