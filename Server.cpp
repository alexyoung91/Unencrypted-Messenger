// Standard Includes
#include <iostream>
#include <string>
#include <cstring>
#include <list>

// SFML Includes
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

// Program Includes
#include "Protocol.hpp"
#include "Encryption.hpp"

using std::cout;
using std::cerr;
using std::cin;
using std::endl;
using std::string;
using std::list;

// Structure to hold client information
struct Client {
	std::string username;
	sf::TcpSocket* socket;
};

// Network
const sf::Uint16 PORT = 21098;		// port
sf::TcpListener listener;			// listener socket used to accept new clients
list<Client> clients;				// list to store client information
list<sf::TcpSocket*> sockets;		// list to store sockets
sf::SocketSelector selector;		// socket multiplexer

// Function prototypes
void clearSubstance(Substance* ps);
bool checkCredentials(const Substance& s);
void quit();

// Printing functions
inline void printInfo(string str) { cout << "INFO: " << str << endl; }
inline void printError(string str) { cerr << "ERROR: " << str << endl; }
inline void printConnections() { cout << "INFO: sockets: " << sockets.size() << ", clients: " << clients.size() << endl; }
inline void printReceived(const Substance& s) { cout << "RECEIVED: {" << s.code << ", " << s.username << ", " << s.password << ", " << s.message << ", " << s.userlist << "}" << endl; }
inline void printSending(const Substance& s) { cout << "SENDING: {" << s.code << ", " << s.username << ", " << s.password << ", " << s.message << ", " << s.userlist << "}" << endl; }

int main() {
	// bind the listener to a port
	if (listener.listen(PORT) != sf::Socket::Done) {
		//printError("Could not listen on port " + PORT);
		quit();
	} else {
		//string str = "";
		//str += PORT;
		//printInfo("Listening on port " + PORT);
	}
	
	selector.add(listener);
	
	while (true) { 
		if (selector.wait()) {
			
			if (selector.isReady(listener)) {
				sf::TcpSocket* socket = new sf::TcpSocket;
				if (listener.accept(*socket) == sf::Socket::Done) {
					sockets.push_back(socket);
					selector.add(*socket);
					//printInfo("Client connected");
				} else {
					delete socket;
					//printError("Failed to accept new client");
				}
				
			} else {
				
				for (list<sf::TcpSocket*>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
					sf::TcpSocket& socket = **it;
					if (selector.isReady(socket)) {
						sf::Packet packet;
						if (socket.receive(packet) == sf::Socket::Done) {
							Substance s;							
							if (packet >> s) {
								packet.clear();
								printReceived(s);
								switch (s.code) {
									case NEW_CONNECTION: {
										if (checkCredentials(s)) {
											Client newClient;
											newClient.username = s.username;
											newClient.socket = *it;
											clients.push_back(newClient);
											s.code = CONNECTION_ACCEPTED;
											//printInfo("New user online: " + s.username);
										} else {
											s.code = COMBO_INVALID;
										}
										
										//printSending(s);
										packet << s;
										if (socket.send(packet) != sf::Socket::Done) {
											//printError("Sending connection status code failed");
											quit();
										}

										//printConnections();
									} case MESSAGE: {
										//cout << "Received message from \"" << s.username << "\": " << s.message << endl;
										// Broadcast message to other connected users
										// this will require looping through the sockets
										//printSending(s);
										packet << s;
										for (list<sf::TcpSocket*>::iterator iter = sockets.begin(); iter != sockets.end(); ++iter) {
											sf::TcpSocket& broadcastSocket = **iter;
											if (*iter != *it) {
												if (broadcastSocket.send(packet) != sf::Socket::Done) {
													//printError("Broadcasting message failed");
													quit();
												}
											}
										}
									} case WHOSONLINE: {
										// get list of users separated by a comma
										for (list<Client>::iterator clit = clients.begin(); clit != clients.end(); ++clit) {
											s.userlist += (*clit).username + ",";
										}
										
										//printSending(s);
										packet << s;
										if (socket.send(packet) != sf::Socket::Done) {
											//printError("Sending whos online response failed");
											quit();
										}
									}
								}
							} else
								cout << "ERROR READING PACKET" << endl;
						} else if (socket.receive(packet) == sf::Socket::Disconnected) {
							// remove client from clients list
							string username = "undefined";
                            for (list<Client>::iterator iter = clients.begin(); iter != clients.end(); ++iter) {
								Client& queryClient = *iter;								
								if (queryClient.socket == *it) {
									username = queryClient.username;
									clients.erase(iter);
									iter--;
								}
							}
							
							// discard the socket
							selector.remove(socket);
							socket.disconnect();
							delete(&socket);
                            sockets.erase(it);
                            it--;
							
                            //printInfo("Client disconnected: " + username);
						}
					}
				}
			}
		}
		// could set a timeout on the wait and then send out heartbeats
		// 	to connected sockets - if they don't reply, remove them from the list?
	}
	
	return 0;
}

void clearSubstance(Substance* ps) {
	memset(ps, 0, sizeof(Substance));
}

bool checkCredentials(const Substance& s) {
	for (list<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if ((*it).username.compare(s.username) == 0)
			return false;
	}
	return true;
}

void quit() {
	cout << "Quitting..." << endl;
	exit(0);
}
