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

        void _counter();

    public:

        std::mutex clockMutex;
        std::atomic<bool> running;

        Clock(std::unordered_map<std::string, clientInfo> clients);
        bool HandleNewClient(std::string name, clientInfo client);
        void Start();
        void Stop();
        ~Clock();
};


#endif