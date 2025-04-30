#ifndef MESSAGETYPE_HPP
#define MESSAGETYPE_HPP

#include <stdint.h>
#include <cstring>
#include <string>

// não deixa ter alinhamento de 4 bytes, ou seja, não deixa ter padding
#pragma pack(push, 1) // Alinha a estrutura em 1 byte
class Message_t {

    public:

        static const int MAX_PAYLOAD_SIZE = 1430; // Tamanho máximo do payload

        // ------ Header  ------

        uint8_t type; 
        uint8_t id[4];  
        char name[20];
        uint8_t length;
        uint8_t seq;
        uint8_t hash[16]; // hash MD5 de 128 bits
        uint16_t payloadSize; // novo campo
        uint8_t reason; 
        char ip[16]; // ip teoricamente não é necessário, só ta sendo passado por estar bugado no podman e eu não to com paciecia de arrumar
        // se for usar mesmo, o melhor é xorear o ip pelo menos, pra q não haja possíveis bloqueios por possuir ip no pacote
        
        // ------ Payload ------

        uint8_t payload[MAX_PAYLOAD_SIZE + 1]; // 1450 bytes de payload

        Message_t();    
        Message_t(uint8_t type, uint8_t id[4], const char* name, uint8_t length, uint8_t seq, uint8_t hash[16], uint8_t reason, const uint8_t* payload, const char* ip, uint16_t payloadSize);    
        std::string toString() const; // Converte a mensagem para string
};
#pragma pack(pop)

#endif