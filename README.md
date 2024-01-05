This project was created as an exercise in IPC and multi-threading on a Linux environment using C++

Overview

The program is composed of the Client and Server applications. The Client will continuously poll user input and send it to the server's message queue. 
The server will parse the message and, if valid, create a new order object and send it to the matching engine. The matching engine will attempt to
match the new order against the existing book. If there is no available match, the engine will add the order to the book. The server will return a result to
the client depending on the outcome of their request.

Usage

1. Compile Server.cpp and Client.cpp found in the src/ directory
2. Open one Server instance and one or more client instances on separate Linux terminals
3. Use the client terminals to send orders to the server using the following input format: {side (BUY/SELL)} {qty} {limit}
