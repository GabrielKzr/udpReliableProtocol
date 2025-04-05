#ifndef SERVER_HPP
#define SERVER_HPP

#pragma once

#include <unordered_map>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <algorithm>
#include <array>

namespace messageTypes {
    constexpr std::string_view heartbeat = "01\n";
    constexpr std::string_view talk = "02\n";
    constexpr std::string_view file = "03\n";
    constexpr std::string_view chunk = "04\n";
    constexpr std::string_view end = "05\n";
    constexpr std::string_view ack = "06\n";
    constexpr std::string_view nack = "07\n";

    constexpr std::array<std::string_view, 7> all = {
        heartbeat, talk, file, chunk, end, ack, nack
    };

    bool isValidMessageType(std::string_view msg);
}

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

        std::string name;

        bool sendKeepAlive();
        bool serverInit();

        void handleMessage(char* buffer, sockaddr_in* addr);

    public:

        Server(int port, std::string name);
        void serverStart();
        void serverClose();


};


#endif