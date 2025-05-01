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

Message_t::Message_t(uint8_t type, uint8_t id[4], const char* name, uint8_t length, uint8_t seq, uint8_t hash[16], uint8_t reason, const uint8_t* payload, const char* ip, uint16_t payloadSize, const char* fileName) {
    this->type = type;
    std::memcpy(this->id, id, sizeof(this->id));
    std::strncpy(this->name, name, sizeof(this->name) - 1);
    this->name[sizeof(this->name) - 1] = '\0'; // Garante que a string esteja terminada
    std::strncpy(this->fileName, fileName, sizeof(this->fileName) - 1);
    this->fileName[sizeof(this->fileName) - 1] = '\0'; // Garante que a string esteja terminada
    std::strncpy(this->ip, ip, sizeof(this->ip) - 1);
    this->ip[sizeof(this->ip) - 1] = '\0'; // Garante que a string esteja terminada
    this->length = length;
    this->seq = seq;
    std::memcpy(this->hash, hash, sizeof(this->hash));
    this->reason = reason;
    std::memcpy(this->payload, payload, sizeof(this->payload));
    this->payloadSize = payloadSize;
}

std::string Message_t::toString() const {
    std::string result;
    result += "Type: " + std::to_string(type) + "\n";
    result += "ID: " + std::to_string(id[0]) + std::to_string(id[1]) + std::to_string(id[2]) + std::to_string(id[3]) + "\n";
    result += "Name: " + std::string(name) + "\n";
    result += "Length: " + std::to_string(length) + "\n";
    result += "Seq: " + std::to_string(seq) + "\n";
    result += "Hash: ";
    for (int i = 0; i < 16; ++i) {
        result += std::to_string(hash[i]);
    }
    result += "\nReason: " + std::to_string(reason) + "\n";
    result += "IP: " + std::string(ip) + "\n";
    result += "Payload: " + std::string(payload, payload + MAX_PAYLOAD_SIZE) + "\n";
    return result;
}