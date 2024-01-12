#include <iostream>
#include <queue>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "MatchingEngine.h"
#include "Order.h"

MatchingEngine::MatchingEngine() {
	bids = new OrderNode;
	asks = new OrderNode;
}

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
	if (order->getType() == LIMIT_BUY) {
		printf("[MATCHING ENGINE] Buy order received (ID=%d, Limit=%d, Qty=%d)\n", order->getId(), order->getLimit(), order->getQty());
		bidQueueLock.lock();
		bidQueue.push(order);
		bidQueueLock.unlock();
	} else if (order->getType() == LIMIT_SELL) {
		printf("[MATCHING ENGINE] Sell order received (ID=%d, Limit=%d, Qty=%d)\n", order->getId(), order->getLimit(), order->getQty());
		askQueueLock.lock();
		askQueue.push(order);
		askQueueLock.unlock();
	}
}

/*
Cancel an active order
@param clientId - ID of client requesting to cancel order
@param orderId - ID of order
@return result of cancellation request
*/
CancelOrderResponse MatchingEngine::cancelOrder(int clientId, int orderId) {
	auto it = orderNodeTable.find(orderId);
	if (it == orderNodeTable.end()) {
		// Order node not present in hash table
		return ORDER_NOT_FOUND;
	}
	
	OrderNode* orderNode = it->second;
	
	if (orderNode->payload->getClientId() != clientId) {
		// Client is attempting to cancel an order they did not create
		return CLIENT_NOT_AUTHORIZED;
	}
	
	std::mutex& lock = orderNode->payload->getType() == LIMIT_BUY ? bidsLock : asksLock;
	lock.lock();
	free(orderNode->payload);
	freeOrderNode(orderNode);
	lock.unlock();
	return ORDER_CANCELLED;
}
		
/*
Attempt to match a new bid order to a pre-existing ask in the orderbook
@param order - Pointer to Order object to attempt to match
@return status of order match
*/
MatchStatus MatchingEngine::matchBid(Order* order) {
	asksLock.lock();
	if (asks->next && order->getLimit() < asks->next->payload->getLimit()) {	// Return false if order is below lowest ask
		asksLock.unlock();
		return NO_FILL;
	}

	struct OrderNode* currNode = asks->next;
	struct OrderNode* nextNode;
	while (currNode && order->getLimit() >= currNode->payload->getLimit()) {
		nextNode = currNode->next;
		if (fillOrder(order, currNode) == FULL_FILL) {
			asksLock.unlock();
			return FULL_FILL;
		}	
		currNode = nextNode;
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

	addOrderNode(currNode, order);
	bidsLock.unlock();
	
	printf("[MATCHING ENGINE] Order added to bid book (ID=%d)\n", order->getId());
}

/*
Attempt to match a new ask order to a pre-existing bid in the orderbook
@param order - Pointer to Order object to attempt to match
@return status of order match
*/
MatchStatus MatchingEngine::matchAsk(Order* order) {
	bidsLock.lock();
	if (bids->next && order->getLimit() > bids->next->payload->getLimit()) {	// Return false if order is above highest bid
		bidsLock.unlock();
		return NO_FILL;
	}

	struct OrderNode* currNode = bids->next;
	struct OrderNode* nextNode;
	while (currNode && order->getLimit() <= currNode->payload->getLimit()) {
		nextNode = currNode->next;
		if (fillOrder(order, currNode) == FULL_FILL) {
			bidsLock.unlock();
			return FULL_FILL;
		}	
		currNode = nextNode;
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
	
	addOrderNode(currNode, order);
	asksLock.unlock();
	
	printf("[MATCHING ENGINE] Order added to ask book (ID=%d)\n", order->getId());
}

MatchStatus MatchingEngine::fillOrder(Order* incomingOrder, OrderNode* existingOrderNode) {
	if (incomingOrder->getQty() < existingOrderNode->payload->getQty()) {
		handlePartialFill(existingOrderNode->payload, incomingOrder->getQty());
		handleFill(incomingOrder);
		return FULL_FILL;
	} else if (incomingOrder->getQty() > existingOrderNode->payload->getQty()) {
		handlePartialFill(incomingOrder, existingOrderNode->payload->getQty());
		handleFill(existingOrderNode->payload);
		freeOrderNode(existingOrderNode);
		return PARTIAL_FILL;
	}
	
	handleFill(incomingOrder);
	handleFill(existingOrderNode->payload);
	freeOrderNode(existingOrderNode);
	return FULL_FILL;
}

void MatchingEngine::handleFill(Order* order) {
	notifyClient(order->getClientId(), "FILL");
	printf("[MATCHING ENGINE] Order filled (ID=%d)\n", order->getId());
	free(order);
}

void MatchingEngine::handlePartialFill(Order* order, int qty) {
	int oldQty = order->getQty();
	order->setStatus(PARTIAL_FILL);
	notifyClient(order->getClientId(), "PARTIAL FILL");
	order->setQty(order->getQty() - qty);
	printf("[MATCHING ENGINE] Order partially filled (ID=%d, Old Qty=%d, New Qty=%d)\n", order->getId(), oldQty, order->getQty());
}

void MatchingEngine::addOrderNode(OrderNode* currNode, Order* order) {
	struct OrderNode* orderNode = (struct OrderNode*) malloc(sizeof(struct OrderNode));	// Dynamically allocate memory for new node
	
	orderNode->payload = order;
	orderNode->next = currNode->next;
	if (currNode->next) {
		currNode->next->prev = orderNode;
	}
	currNode->next = orderNode;
	orderNode->prev = currNode;
	
	orderNodeTable[order->getId()] = orderNode;	// Add order node to hash table for O(1) lookup
}

void MatchingEngine::freeOrderNode(OrderNode* node) {
	node->prev->next = node->next;
	if (node->next) {
		node->next->prev = node->prev;
	}
	free(node);
}

/*
Send a notification message to a client
@param clientId - The id of the client
@param msg - The message
*/
void MatchingEngine::notifyClient(int clientId, std::string msg) {
	// Not yet implemented
}
