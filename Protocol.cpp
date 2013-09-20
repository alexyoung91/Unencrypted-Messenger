#include "Protocol.hpp"

sf::Packet& operator <<(sf::Packet& packet, const Substance& s) {
    return packet << s.code << s.username << s.password << s.message << s.userlist;
}

sf::Packet& operator >>(sf::Packet& packet, Substance& s) {
    return packet >> s.code >> s.username >> s.password >> s.message >> s.userlist;
}
