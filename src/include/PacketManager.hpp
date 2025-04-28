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
#include <chrono>
#include <mutex>

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

        std::timed_mutex ackManagerMutex; 

        // verifica se mensagem atual já foi "acked"
        bool isAcked();


    public:

        PacketManager(int port);
        bool sendMessage(Message_t packet, std::string ip, int sock, std::mutex& mtx);
        void sendMessageWithoutAck(Message_t packet, std::string ip, int sock, std::mutex& mtx);
        bool verifyAck(uint8_t* ack);  
        void retransmitPacket(int sock, std::mutex& mtx, Message_t packet, struct sockaddr_in addr);
        void handleNack(uint8_t reason);

        Message_t buildTalkMessage(std::string message, std::string localIp);
        Message_t buildNackMessage(uint8_t* id, uint8_t reason, std::string localIp);
        Message_t buildAckMessage(uint8_t* id, std::string localIp);
};


#endif