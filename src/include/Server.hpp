#ifndef SERVER_HPP
#define SERVER_HPP

#pragma once

#include "PacketManager.hpp"
#include <mutex>
#include <atomic>
#include <thread>

/* // acho que talvez mais pra frente precise usar isso ao invés do pair no map de clients
struct clientInfo {
    std::string ip;
    int counter;
    int id;
};
*/

class Server {

    private:

                        // key = name ----> value = (ip, tempo)
        std::unordered_map<std::string, std::pair<std::string, int>> clients;
        
        int server_socket;
        int port;
    
        std::string name; // nome do processo na rede

        PacketManager packetManager;

        std::mutex sendMutex;

        bool sendKeepAlive();
        bool serverInit();

        void handleMessage(char* buffer, sockaddr_in* addr);

    public:

        Server(int port, std::string name);
        void serverStart();
        void serverClose();


};


#endif