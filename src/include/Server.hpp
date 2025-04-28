#ifndef SERVER_HPP
#define SERVER_HPP

#pragma once

#include "PacketManager.hpp"
#include <mutex>
#include <atomic>
#include <thread>
#include <ifaddrs.h>

#include "Utils.hpp"
#include "Clock.hpp"
#include "Console.hpp"

class Server {

    private:

                        // key = name ----> value = (ip, tempo)
        std::unordered_map<std::string, clientInfo> clients;
        
        int server_socket;
        int port;
    
        std::string name; // nome do processo na rede
        std::string localIp;

        PacketManager* packetManager;
        Clock* clock;
        Console console;

        std::mutex sendMutex;

        bool sendKeepAlive();
        bool serverInit();

        void handleMessage(Message_t* buffer, sockaddr_in* addr, int receivedBytes);

        std::string getLocalIp();

    public:

        Server(int port, std::string name);
        void serverStart();
        void serverClose();
        ~Server();
};


#endif