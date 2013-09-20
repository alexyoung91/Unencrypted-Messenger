CC = g++
CFLAGS = -std=c++11 -pthread
LIBS = -lsfml-network -lsfml-system -lncurses

all:
	${CC} ${CFLAGS} -o Server Server.cpp Protocol.cpp Encryption.cpp -I"../SFML2/include" -L"../SFML2/lib" -DDEBUG ${LIBS}
	${CC} ${CFLAGS} -o Client Client.cpp Protocol.cpp Encryption.cpp -I"../SFML2/include" -L"../SFML2/lib" -DDEBUG ${LIBS}


# DEPENDENCIES:
#
#	sfml (possible to get static libraries for system and networking?)
#	ncurses (windows alternative?)
