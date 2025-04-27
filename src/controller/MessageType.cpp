#include "../include/MessageType.hpp"

Message_t::Message_t() {
    // Inicializa os campos da estrutura
    type = 0;
    std::memset(id, 0, sizeof(id));
    std::memset(name, 0, sizeof(name));
    length = 0;
    seq = 0;
    std::memset(hash, 0, sizeof(hash));
    reason = 0;
    std::memset(payload, 0, sizeof(payload));
}

Message_t::Message_t(uint8_t type, uint8_t id[4], const char* name, uint8_t length, uint8_t seq, uint8_t hash[16], uint8_t reason, const uint8_t* payload, const char* ip) {
    this->type = type;
    std::memcpy(this->id, id, sizeof(this->id));
    std::strncpy(this->name, name, sizeof(this->name) - 1);
    this->name[sizeof(this->name) - 1] = '\0'; // Garante que a string esteja terminada
    std::strncpy(this->ip, ip, sizeof(this->ip) - 1);
    this->ip[sizeof(this->ip) - 1] = '\0'; // Garante que a string esteja terminada
    this->length = length;
    this->seq = seq;
    std::memcpy(this->hash, hash, sizeof(this->hash));
    this->reason = reason;
    std::memcpy(this->payload, payload, sizeof(this->payload));
}