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

class Clock {

    private:

        std::unordered_map<std::string, clientInfo>& clients; // key = name ----> value = tempo
        std::atomic<bool> running;

        void _counter();

    public:

        std::mutex clockMutex;

        Clock(std::unordered_map<std::string, clientInfo>& clients);
        bool HandleNewClient(std::string name, clientInfo client);
        clientInfo* getClientInfo(std::string name);
        bool containsClient(std::string ip);
        void Start();
        void Stop();
        ~Clock();
};


#endif