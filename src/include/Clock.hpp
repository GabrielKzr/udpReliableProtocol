#ifndef CLOCK_HPP
#define CLOCK_HPP

#pragma once

#include <unordered_map>
#include "Utils.hpp"
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <openssl/md5.h>
#include <string>
#include <vector>
#include "MessageType.hpp"
#include <sstream>

class Clock {

    private:

        int fileCounter = 1;

        std::unordered_map<std::string, clientInfo>& clients; // key = name ----> value = tempo
        std::unordered_map<std::string, std::vector<Message_t>> filesManagement;
        std::vector<std::pair<std::string, int>> talkTempReceivedPackets;

        std::atomic<bool> running;

        void _createFile(std::vector<Message_t> packets);
        bool _handleCompleted(std::string name);
        void _counter();

    public:

        std::mutex clockMutex;

        int addClientFile(Message_t packet);

        Clock(std::unordered_map<std::string, clientInfo>& clients);
        bool HandleTalkMessage(Message_t packet);
        bool HandleNewClient(std::string name, clientInfo client);
        clientInfo* getClientInfo(std::string name);
        bool containsClient(std::string ip);
        void Start();
        void Stop();
        void clientsToString();
        ~Clock();
};


#endif