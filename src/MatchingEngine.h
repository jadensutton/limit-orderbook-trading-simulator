#include <mutex>
#include <queue>
#include "Order.h"

#ifndef MatchingEngine_H
#define MatchingEngine_H

struct OrderNode {
	Order* payload;
	OrderNode* next;
};	// Struct to house the Order objects as part of a sorted linked list
	
class MatchingEngine {
	OrderNode *bids, *asks;
	std::mutex bidsLock, asksLock, bidQueueLock, askQueueLock;
	std::queue<Order*> bidQueue, askQueue;	// Queue to house Order objects before being matched or added to orderbook
	
	public:
		void pollBidQueue();
		void pollAskQueue();
		void placeOrder(Order* order);
		void cancelOrder(int orderId);
		std::string getBidsSnapshot();
		std::string getAsksSnapshot();
		
	private:		
		MatchStatus matchBid(Order* order);
		void addBid(Order* order);
		MatchStatus matchAsk(Order* order);
		void addAsk(Order* order);
		void handleFill(Order* order);
		void handlePartialFill(Order* order, int qty);
		void freeOrderNode(OrderNode* node);
		void notifyClient(int clientId, std::string msg);
};

#endif
