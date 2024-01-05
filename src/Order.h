#ifndef Order_H
#define Order_H
enum OrderType {
	LIMIT_BUY,
	LIMIT_SELL,
	/*Market orders not current supported
	MARKET_BUY,
	MARKET_SELL*/
};

enum MatchStatus {
	FULL_FILL,
	PARTIAL_FILL,
	NO_FILL,
};

class Order
{	
	static int idCounter;
	int id;	
	OrderType type;
	int limit;
	int qty;
	int clientId;
	MatchStatus status;
	
	public:
		Order(OrderType type, int limit, int qty, int clientId);
		int getId();
		OrderType getType();
		int getLimit();
		int getQty();
		int getClientId();
		MatchStatus getStatus();
		void setId(int id);
		void setType(OrderType type);
		void setLimit(int limit);
		void setQty(int qty);
		void setClientId(int clientId);
		void setStatus(MatchStatus status);
};

#endif
