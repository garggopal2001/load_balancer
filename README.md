
# Simplified Load Balancer System

## Table of Contents
- [Motivation and Purpose](#motivation-and-purpose)
- [Project Overview](#project-overview)
- [Requirements](#requirements)
- [How to Run](#how-to-run)
  - [Compilation](#compilation)
  - [Starting the Servers](#starting-the-servers)
  - [Starting the Load Balancer](#starting-the-load-balancer)
  - [Running the Clients](#running-the-clients)
- [Files in the Project](#files-in-the-project)
- [Explanation of the Implementation](#explanation-of-the-implementation)
  - [Client (client.c)](#client-clientc)
  - [Load Balancer (lb.c)](#load-balancer-lbc)
  - [Server (server.c)](#server-serverc)
- [Additional Notes](#additional-notes)
- [Potential Enhancements](#potential-enhancements)

## Motivation and Purpose
The purpose of this project is to provide a practical implementation of a simplified load balancer system. It serves as an educational tool to understand:
- **Networking Concepts:** How TCP/IP networking works at a basic level.
- **Socket Programming in C:** Establishing and managing socket connections between processes.
- **Inter-Process Communication:** Communication between different processes over a network.
- **Load Balancing Mechanisms:** How a load balancer distributes client requests to servers based on certain criteria (e.g., server load).

## Project Overview
This project simulates a simple load balancing system consisting of:

1. **Two Computation Servers (S1 and S2):**
   - Each server can send its current "load" (a random integer between 1 and 100) when requested.
   - Can provide the current date and time upon request.
   
2. **One Load Balancer (L):**
   - Periodically (every 5 seconds) requests the load from both servers.
   - Forwards client requests to the server with the least load.
   - Acts as a server to the clients and as a client to the servers.
   
3. **Multiple Clients:**
   - Connect to the load balancer to request the current date and time.

The load balancer collects load information from the servers and uses it to distribute incoming client requests efficiently, simulating a real-world load balancing scenario.

## Requirements
- **Operating System:** Linux or Unix-based system.
- **Compiler:** GCC (GNU Compiler Collection).
- **Knowledge Prerequisites:**
  - Basic understanding of C programming.
  - Familiarity with TCP/IP networking concepts.
  - Understanding of socket programming in C.

## How to Run

### Compilation
Open a terminal and navigate to the directory containing the source code files. Compile each of the programs using the following commands:

```bash
gcc server.c -o server
gcc lb.c -o lb
gcc client.c -o client
```

### Starting the Servers
You'll need to start two instances of the server program for S1 and S2. Open two separate terminals for this purpose.

- **Server S1:**
  ```bash
  ./server <S1_Port>
  ```
  Replace `<S1_Port>` with the port number for Server S1 (e.g., 8001).

- **Server S2:**
  ```bash
  ./server <S2_Port>
  ```
  Replace `<S2_Port>` with the port number for Server S2 (e.g., 8002).

**Example:**
```bash
# Terminal for Server S1
./server 8001

# Terminal for Server S2
./server 8002
```

### Starting the Load Balancer
Open another terminal to start the load balancer.

```bash
./lb <LB_Port> <S1_Port> <S2_Port>
```
- `<LB_Port>`: Port number for the Load Balancer (e.g., 9000).
- `<S1_Port>`: Port number where Server S1 is running.
- `<S2_Port>`: Port number where Server S2 is running.

**Example:**
```bash
./lb 9000 8001 8002
```

### Running the Clients
You can run multiple clients. Open new terminal(s) for each client.

```bash
./client <LB_IP> <LB_Port>
```
- `<LB_IP>`: IP address of the Load Balancer (usually 127.0.0.1 for localhost).
- `<LB_Port>`: Port number where the Load Balancer is running.

**Example:**
```bash
./client 127.0.0.1 9000
```

### Observing the Outputs
- **Load Balancer Terminal:** Shows polling status, load requests to servers, and forwarding of client requests.
- **Server Terminals:** Display loads being sent when requested by the load balancer.
- **Client Terminals:** Display the current system date and time received from the servers via the load balancer.

## Files in the Project
1. **server.c**: The server program used for both S1 and S2. Handles requests from the load balancer to send load or current date and time.
2. **lb.c**: The load balancer program. Interacts with both clients and servers. Distributes client requests to servers based on their current load.
3. **client.c**: The client program. Connects to the load balancer to request the current date and time.

## Explanation of the Implementation

### Client (client.c)
**Purpose:**
- Connects to the load balancer to request the current date and time.

**Functionality:**
- Creates a TCP socket and connects to the load balancer using the provided IP and port.
- Receives the date and time from the load balancer.
- Displays the received date and time.
- Closes the socket.

### Load Balancer (lb.c)
**Purpose:**
- Acts as an intermediary between clients and servers.
- Distributes client requests to the server with the least load.

**Functionality:**
- Listens for client connections on a specified port.
- Uses `poll()` with a 5-second timeout to check for incoming client connections.
- Every 5 seconds (if no client connects), requests the current load from both servers.
- Maintains the last known load values for each server.
- When a client connects, it forwards the request to the server with the least load.
- Receives the date and time from the selected server and sends it back to the client.

### Server (server.c)
**Purpose:**
- Responds to requests from the load balancer.

**Functionality:**
- Listens for connections on a specified port.
- Upon receiving a connection, reads the request:
  - If the request is "Send Load", generates a random load value between 1 and 100 and sends it back.
  - If the request is "Send Time", retrieves the current date and time and sends it back.
- Closes the connection after responding.

## Additional Notes
- **Custom Receive Function:** A custom `receive_data` function is used to handle incoming data in chunks to ensure complete messages are received.
- **Concurrency Handling:** The load balancer uses `fork()` to handle multiple client connections concurrently.
- **Random Load Generation:** Servers generate a random load using `rand()` seeded with the current time.
- **Networking Protocol:** All communication is done over TCP sockets.
- **Error Handling:** The code includes error checking after critical operations like socket creation, binding, connecting, and sending/receiving data.
- **Port Selection:** Ensure that the ports used are not occupied by other services or blocked by firewalls.

## Potential Enhancements
1. **Dynamic Server Discovery:** Implement a mechanism for the load balancer to discover servers dynamically.
2. **Load Calculation:** Replace the dummy random load with actual server load metrics (CPU usage, memory usage).
3. **Multiple Services:** Extend the servers to handle multiple types of client requests.
4. **Improved Concurrency:** Use threading instead of forking for better performance and resource management.
5. **Logging and Monitoring:** Implement detailed logging and monitoring for debugging and analysis.
