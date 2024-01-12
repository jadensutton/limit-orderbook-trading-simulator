#include <mutex>
#include <queue>
#include <unordered_map>
#include "Order.h"

#ifndef MatchingEngine_H
#define MatchingEngine_H

enum CancelOrderResponse {
	ORDER_CANCELLED,
	ORDER_NOT_FOUND,
	CLIENT_NOT_AUTHORIZED
};

struct OrderNode {
	Order* payload;
	OrderNode* next;
	OrderNode* prev;
};	// Struct to house the Order objects as part of a sorted, doubly linked list
	
class MatchingEngine {
	OrderNode *bids, *asks;
	std::mutex bidsLock, asksLock, bidQueueLock, askQueueLock;
	std::queue<Order*> bidQueue, askQueue;	// Queue to house Order objects before being matched or added to orderbook
	std::unordered_map<int, OrderNode*> orderNodeTable;	// Hash table to index order nodes by ID
	
	public:
		MatchingEngine();
		void pollBidQueue();
		void pollAskQueue();
		void placeOrder(Order* order);
		CancelOrderResponse cancelOrder(int clientId, int orderId);
		std::string getBidsSnapshot();
		std::string getAsksSnapshot();
		
	private:		
		MatchStatus matchBid(Order* order);
		void addBid(Order* order);
		MatchStatus matchAsk(Order* order);
		void addAsk(Order* order);
		MatchStatus fillOrder(Order* incomingOrder, OrderNode* existingOrderNode);
		void handleFill(Order* order);
		void handlePartialFill(Order* order, int qty);
		void addOrderNode(OrderNode* currNode, Order* order);
		void freeOrderNode(OrderNode* node);
};

#endif
