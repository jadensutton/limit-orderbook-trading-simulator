#include <iostream>
#include <queue>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "MatchingEngine.h"
#include "Order.h"

/*
Continuously poll the bid queue and service orders in FIFO fashion
*/
void MatchingEngine::pollBidQueue() {
	Order* nextOrder;	// Pointer for the next order in queue
	while (1) {
		if (!bidQueue.empty()) {
			bidQueueLock.lock();
			nextOrder = bidQueue.front();
			bidQueue.pop();
			bidQueueLock.unlock();
			if (matchBid(nextOrder) != FULL_FILL)
				addBid(nextOrder);
		}
	}
}

/*
Continuously poll the ask queue and service orders in FIFO fashion
*/
void MatchingEngine::pollAskQueue() {
	Order* nextOrder;	// Pointer for the next order in queue
	while (1) {
		if (!askQueue.empty()) {
			askQueueLock.lock();
			nextOrder = askQueue.front();
			askQueue.pop();
			askQueueLock.unlock();
			if (matchAsk(nextOrder) != FULL_FILL)
				addAsk(nextOrder);
		}
	}
}

/*
Place a new order
@param order - Order object
*/
void MatchingEngine::placeOrder(Order* order) {
	std::cout << "[MATCHING ENGINE] " << order->getType() << " order received (ID=" << order->getId() << ")" << std::endl;
	if (order->getType() == LIMIT_BUY) {
		bidQueueLock.lock();
		bidQueue.push(order);
		bidQueueLock.unlock();
	} else if (order->getType() == LIMIT_SELL) {
		askQueueLock.lock();
		askQueue.push(order);
		askQueueLock.unlock();
	}
}

/*
Cancel an active order
@param orderId - ID of order
*/
void MatchingEngine::cancelOrder(int orderId) {
	// Not yet implemented
}
		
/*
Attempt to match a new bid order to a pre-existing ask in the orderbook
@param order - Pointer to Order object to attempt to match
@return true if order successfully matched, false otherwise
*/
MatchStatus MatchingEngine::matchBid(Order* order) {
	asksLock.lock();
	if (asks->next && order->getLimit() < asks->next->payload->getLimit()) {	// Return false if order is below lowest ask
		asksLock.unlock();
		return NO_FILL;
	}

	struct OrderNode* currNode = asks;
	while (currNode->next && order->getLimit() >= currNode->next->payload->getLimit()) {
		if (order->getQty() < currNode->next->payload->getQty()) {
			handlePartialFill(currNode->next->payload, order->getQty());
			handleFill(order);
			asksLock.unlock();
			return FULL_FILL;
		} else if (order->getQty() > currNode->next->payload->getQty()) {
			handlePartialFill(order, currNode->next->payload->getQty());
			handleFill(currNode->next->payload);
			freeOrderNode(currNode->next);
		} else {
			handleFill(order);
			handleFill(currNode->next->payload);
			freeOrderNode(currNode->next);
			asksLock.unlock();
			return FULL_FILL;
		}
		
		currNode = currNode->next;
	}
	
	asksLock.unlock();
	return PARTIAL_FILL;
}

/*
Add bid order to the orderbook
@param order - Pointer to Order object to add to book
*/
void MatchingEngine::addBid(Order* order) {
	bidsLock.lock();
	struct OrderNode* currNode = bids;
	while (currNode->next && currNode->next->payload->getLimit() >= order->getLimit()) {
		currNode = currNode->next;
	}
	
	struct OrderNode* orderNode = (struct OrderNode*) malloc(sizeof(struct OrderNode));
	
	orderNode->payload = order;
	orderNode->next = currNode->next;
	currNode->next = orderNode;
	bidsLock.unlock();
	
	std::cout << "[MATCHING ENGINE] Order added to bid book (ID=" << order->getId() << ")" << std::endl;
}

/*
Add ask order to the orderbook
@param order - Pointer to Order object to add to book
*/
MatchStatus MatchingEngine::matchAsk(Order* order) {
	bidsLock.lock();
	if (bids->next && order->getLimit() > bids->next->payload->getLimit()) {	// Return false if order is above highest bid
		bidsLock.unlock();
		return NO_FILL;
	}

	struct OrderNode* currNode = bids;
	while (currNode->next && order->getLimit() <= currNode->next->payload->getLimit()) {
		if (order->getQty() < currNode->next->payload->getQty()) {
			handlePartialFill(currNode->next->payload, order->getQty());
			handleFill(order);
			bidsLock.unlock();
			return FULL_FILL;
		} else if (order->getQty() > currNode->next->payload->getQty()) {
			handlePartialFill(order, currNode->next->payload->getQty());
			handleFill(currNode->next->payload);
			currNode->next = currNode->next->next;
		} else {
			handleFill(order);
			handleFill(currNode->next->payload);
			currNode->next = currNode->next->next;
			bidsLock.unlock();
			return FULL_FILL;
		}
		
		currNode = currNode->next;
	}
	
	bidsLock.unlock();
	return PARTIAL_FILL;
}

/*
Add ask order to the orderbook
@param order -  Pointer to Order object to add to book
*/
void MatchingEngine::addAsk(Order* order) {
	asksLock.lock();
	struct OrderNode* currNode = asks;
	while (currNode->next && currNode->next->payload->getLimit() <= order->getLimit()) {
		currNode = currNode->next;
	}
	
	struct OrderNode* orderNode = (struct OrderNode*) malloc(sizeof(struct OrderNode));
	
	orderNode->payload = order;
	orderNode->next = currNode->next;
	currNode->next = orderNode;
	asksLock.unlock();
	
	std::cout << "[MATCHING ENGINE] Order added to ask book (ID=" << order->getId() << ")" << std::endl;
}

void MatchingEngine::handleFill(Order* order) {
	notifyClient(order->getClientId(), "FILL");
	free(order);
}

void MatchingEngine::handlePartialFill(Order* order, int qty) {
	order->setStatus(PARTIAL_FILL);
	notifyClient(order->getClientId(), "PARTIAL FILL");
	order->setQty(order->getQty() - qty);
}

void MatchingEngine::freeOrderNode(OrderNode* node) {
	struct OrderNode* freeNode = node->next;
	node->next = freeNode->next;
	free(freeNode);
}

/*
Send a notification message to a client
@param clientId - The id of the client
@param msg - The message
*/
void MatchingEngine::notifyClient(int clientId, std::string msg) {
	// Not yet implemented
	//int fd = open("./tmp/fifo%s", O_WRONLY);
}
