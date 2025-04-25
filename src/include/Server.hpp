#ifndef SERVER_HPP
#define SERVER_HPP

#pragma once

#include "PacketManager.hpp"
#include <mutex>
#include <atomic>
#include <thread>
#include "Utils.hpp"


struct clientInfo {
    std::string ip;
    int tempo;
};

class Server {

    private:

                        // key = name ----> value = (ip, tempo)
        std::unordered_map<std::string, clientInfo> clients;
        
        int server_socket;
        int port;
    
        std::string name; // nome do processo na rede

        PacketManager packetManager;

        std::mutex sendMutex;

        bool sendKeepAlive();
        bool serverInit();

        void handleMessage(Message_t* buffer, sockaddr_in* addr, int receivedBytes);

    public:

        Server(int port, std::string name);
        void serverStart();
        void serverClose();


};


#endif