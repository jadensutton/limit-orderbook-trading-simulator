#include "Order.h"


Order::Order(OrderType type, int limit, int qty, int clientId) {
	this->id = idCounter++;
	this->type = type;
	this->limit = limit;
	this->qty = qty;
	this->clientId = clientId;
	this->status = NO_FILL;
}

int Order::getId() {
	return id;
}

OrderType Order::getType() {
	return type;
}

int Order::getLimit() {
	return limit;
}

int Order::getQty() {
	return qty;
}

int Order::getClientId() {
	return clientId;
}

MatchStatus Order::getStatus() {
	return status;
}

void Order::setId(int id) {
	this->id = id;
}

void Order::setType(OrderType type) {
	this->type = type;
}

void Order::setLimit(int limit) {
	this->limit = limit;
}

void Order::setQty(int qty) {
	this->qty = qty;
}

void Order::setClientId(int clientId) {
	this->clientId = clientId;
}

void Order::setStatus(MatchStatus status) {
	this->status = status;
}
