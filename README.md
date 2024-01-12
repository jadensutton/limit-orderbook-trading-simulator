This project was created as a learning exercise in IPC and concurrency on a Linux environment using C++

Overview

The program is composed of the Client and Server applications. The Client will continuously poll user input and send it to the server's message queue. 
The server will parse the message and, if valid, create a new order object and send it to the matching engine. The matching engine will attempt to
match the new order against the existing book. If there is no available match, the engine will add the order to the book. The server will return a result to
the client depending on the outcome of their request.

Design Decisions

I chose to represent the orderbook as a sorted, doubly linked list. The linked list data structure allows for O(1) insertion/removal, which is critical when considering
that an orderbook requires storing massive amounts of data. Additionally, the fact that the linked list is sorted allows for quick and efficient matching of incoming orders.
I also use a hash table to store a pointer for each order indexed by its order ID. This allows for O(1) lookup during order cancellation.

For IPC, I decided on a message queue. While I acknowledge that shared memory would have been much faster, this project was mainly done as a learning exercise in
Linux IPC and client/server architecture. I may update the project in the future to use shared memory.

I chose the following rules for the orderbook:
1. Orders are matched using a price-time priority (FIFO) matching algorithm
2. Clients are allowed to match with themselves

Feature roadmap
- Object pooling for order/node objects
- Notification system to update clients regarding order fills
- Use shared memory instead of a message queue for client/server requests
- Allow clients to modify their active orders

Usage

1. Compile Server.cpp and Client.cpp found in the src/ directory
2. Open one Server instance and one or more client instances on separate Linux terminals
3. Use the client terminals to send orders to the server using the following input format: {side (BUY/SELL)} {qty} {limit}
