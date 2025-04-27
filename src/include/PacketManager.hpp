#ifndef PACKETMANAGER_HPP
#define PACKETMANAGER_HPP

#pragma once

#include <unordered_map>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <algorithm>

#include "MessageType.hpp"
#include "Utils.hpp"

struct Message {
    Message_t* msg;
    bool acked; 
};

/*
    O packet manager garante que os pacotes vão ser recebidos e que, caso não sejam recebidos, haja retransmissão

    * A função sendMessage não enviará outra mensagem se a mensagem atual (actualMessage) não houver sido "acked" (confirmada)
    * Caso um ack recebido seja diferente do ack da actualMessage, automaticamente será ignorado, visto que esta classe garante que só um pacote por vez será enviado
    * A condição de chamada da função de retransmissão ocorrerá apenas quando o timeout da resposta após o envio da mensagem exceder, 
      (isso será controlado por outra thread) e não por essa classe, está classe fornecerá apenas a API para uso externo

*/

class PacketManager {

    private:
    
        int port;
        Message* actualMessage;
        int actualSystemId;

        // verifica se mensagem atual já foi "acked"
        bool isAcked();

    public:

        PacketManager(int port);
        bool sendMessage(Message_t packet, std::string ip, int sock);
        bool verifyAck(int ack);  
        void retransmitPacket();

        Message_t buildMessage(std::string name, std::string message);
};


#endif