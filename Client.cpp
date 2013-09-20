// Standard Includes
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <thread>
#include <mutex>

// Program Includes
#include "Protocol.hpp"
#include "Encryption.hpp"

// ncurses (UI)
#include <ncurses.h>

// SFML (Networking)
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

using std::string;
using std::stringstream;
using std::vector;
using std::out_of_range;
using std::thread;
using std::mutex;

// UI variables
int screenY, screenX;
WINDOW* serverStatusWin;
WINDOW* messagesWin;
WINDOW* whosOnlineWin;
WINDOW* inputWin;
vector<string> messages;
vector<string> whosonline;

// Multithreading variables
thread handleIncomingThread;
thread onTimeThread;
mutex globalMutex;

// Network variables
char address[] = "localhost";
sf::Uint16 port = 21098;
char username[32];
sf::TcpSocket socket;

// UI functions
WINDOW* createWindow(int starty, int startx, int height, int width);
void destroyWindow(WINDOW* local_win);
void addMessage(string& msg);
void updateServerStatus();
void updateMessages();
void updateWhosOnline(const string& userlist);

// Network functions
void handleIncoming();
void onTime();

// Program
bool running = true;

int main(int argc, char* argv[]) {
	
	initscr();			// Initialise curses
	echo();				// Echo typed characters on screen
	
	getmaxyx(stdscr, screenY, screenX);		// Get console dimensions (25 x 80 for ubuntu)
	
	printw("Conker\nEncrypted Chat\n\n");
	
	// Get credentials
	
	/*
	
	printw("Address: ");
	refresh();
	getstr(address);
	
	printw("Port: ");
	refresh();
	scanw("%d", &port);
	
	*/
	
	printw("Choose a name: ");
	refresh();
	getstr(username);
	
	// Attempt to connect with above credientials
	printw("Connecting to server... ");
	refresh();
	sf::Socket::Status status = socket.connect(address, port);
	if (status != sf::Socket::Done) {
		printw("Failed.\n");
		refresh();
		exit(0);
	} else {
		printw("Success!\n");
		refresh();
	}
	
	// Send details to server to store information about client
	Substance s;
	s.code = NEW_CONNECTION;
	s.username = username;	
	sf::Packet packet;
	packet << s;
	if (socket.send(packet) != sf::Socket::Done) {
		printw("An error occured whilst sending register request\n");
		exit(0);
	}
	packet.clear();
	
	// Receieve confirmation that client is connected
	if (socket.receive(packet) != sf::Socket::Done) {
		printw("An error occured whilst receiving register confirmation\n");
		exit(0);
	}
	
	if (packet >> s) {
		if (s.code == CONNECTION_ACCEPTED) {
			printw("Successfully registered on the network\n");
			printw("\n");
			refresh();
		} else if (s.code == COMBO_INVALID) {
			printw("Name already taken\n");
			refresh();
			exit(0);
		} else {
			printw("An error occured whilst registering on the network\n");
			refresh();
			exit(0);
		}
		packet.clear();
	} else {
		printw("Error reading packet\n");
		refresh();
		exit(0);
	}
	
	sf::sleep(sf::milliseconds(500));
	
	// Create four windows
	
	serverStatusWin = createWindow(0, 0, 6, 80);
	messagesWin = createWindow(6, 0, 15, 60);
	whosOnlineWin = createWindow(6, 60, 15, 20);
	inputWin = createWindow(21, 0, 3, 80);
	
	/*
	 * Chat loop
	 */
	
	//handleIncomingThread = thread(handleIncoming);
	//onTimeThread = thread(onTime);
	
	//updateServerStatus();
	
	while (running) {
		sf::Packet outPacket;
		Substance outS;
		outS.username = username;
		outS.code = MESSAGE;
		
		mvwprintw(inputWin, 1, 1, "You: ");
		wrefresh(inputWin);
		char input[64];
		wgetstr(inputWin, input);
		
		if (strcmp(input, "") == 0) {
			//wclear(inputWin);
			//box(inputWin, 0, 0);
			continue;
		}
		
		outS.message = input;
		
		if (outS.message.compare("/quit") == 0) {
			running = false;
			break;
		}
		else {
			outPacket << outS;	
			if (socket.send(outPacket) != sf::Socket::Done) {
				wprintw(inputWin, "An error occured whilst sending\n");
				wrefresh(inputWin);
				running = false;
				break;
			}
			
			string msg = "You: ";
			msg += input;
			addMessage(msg);
			updateMessages();
			
			//wclear(inputWin);
			//box(inputWin, 0, 0);
		}
	}
	
	//handleIncomingThread.join();
	//onTimeThread.join();
	
	socket.disconnect();
	
	destroyWindow(serverStatusWin);
	destroyWindow(messagesWin);
	destroyWindow(whosOnlineWin);
	destroyWindow(inputWin);
	
	endwin();
	
	return 0;
}

void handleIncoming() {
	
	sf::SocketSelector selector;
	selector.add(socket);
	
	while (running) {
		sf::Packet inPacket;
		Substance inS;
		
		if (selector.wait(sf::milliseconds(1000))) {
			if (selector.isReady(socket)) {
				if (socket.receive(inPacket) != sf::Socket::Done) {
					printw("An error occured whilst receiving a message\n");
					refresh();
				} else {
					if (inPacket >> inS) {
						switch (inS.code) {
							case MESSAGE: {
								string msg = inS.username + ": " + inS.message;
								addMessage(msg);
								updateMessages();
								break;
							} case WHOSONLINE: {
								updateWhosOnline(inS.userlist);
								break;
							}
						}
					} else {
						printw("Error reading packet\n");
					}
				}
			}
		}
	}
}

void onTime() {
	while (running) {
		
		// message window updates
		updateMessages();
		
		// whos online window updates
		sf::Packet outPacket;
		Substance outS;
		outS.username = username;
		outS.code = WHOSONLINE;
		
		outPacket << outS;
		if (socket.send(outPacket) != sf::Socket::Done) {
			wprintw(inputWin, "An error occured whilst sending whos online request\n");
			wrefresh(inputWin);
		}
		sf::sleep(sf::milliseconds(5000));
	}
}

WINDOW* createWindow(int starty, int startx, int height, int width)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0, 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroyWindow(WINDOW* local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}

void addMessage(string& msg) {
	globalMutex.lock();
		messages.push_back(msg);
	globalMutex.unlock();
}

void updateServerStatus() {
	char title[] = "Conker";
	
	//wclear(serverStatusWin);
	mvwprintw(serverStatusWin, 1, floor((screenX - strlen(title)) / 2.0), title, address);
	mvwprintw(serverStatusWin, 2, 1, "Host: %s", address);
	mvwprintw(serverStatusWin, 3, 1, "Port: %d", port);
	mvwprintw(serverStatusWin, 4, 1, "Username: %s", username);
	//box(serverStatusWin, 0, 0);
	wrefresh(serverStatusWin);
}

void updateMessages() {
	//wclear(messagesWin);
	
	int noMessages = 13;
	for (int i = 0; i < noMessages; i++) {
		string msg = "";
		try {
			msg = messages.at(messages.size() - (noMessages - i));      // vector::at throws an out-of-range
		} catch (const std::out_of_range& oor) {
			//std::cerr << "Out of Range error: " << oor.what() << '\n';
		}
		mvwprintw(messagesWin, i + 1, 1, msg.c_str());
	}

	//box(messagesWin, 0, 0);
	wrefresh(messagesWin);
}

void updateWhosOnline(const string& userlist) {
	whosonline.clear();
	
	string currentUser;
    stringstream stream(userlist);
    while (getline(stream, currentUser, ','))
		if (currentUser.compare("") != 0)
			whosonline.push_back(currentUser);
			
	//wclear(whosOnlineWin);
	for (int i = 0; i < whosonline.size(); i++)
		mvwprintw(whosOnlineWin, i + 1, 1, whosonline.at(i).c_str());
	//box(whosOnlineWin, 0, 0);
	wrefresh(whosOnlineWin);
}
