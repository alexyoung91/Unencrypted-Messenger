#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

// Standard Includes
#include <string>

// SFML Includes
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

// Substance Codes
const sf::Uint16	NEW_CONNECTION 			=		1;
const sf::Uint16	CONNECTION_ACCEPTED		=		2;
const sf::Uint16	COMBO_INVALID			=		3;
const sf::Uint16	MESSAGE					=		4;
const sf::Uint16	WHOSONLINE				=		5;

struct Substance {
	sf::Uint16 code;
	std::string username;
	std::string password;
	std::string message;
	std::string userlist;
};

sf::Packet& operator <<(sf::Packet& packet, const Substance& s);
sf::Packet& operator >>(sf::Packet& packet, Substance& s);

#endif
