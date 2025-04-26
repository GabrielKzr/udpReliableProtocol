#ifndef UTILS_HPP
#define UTILS_HPP
    
#pragma once

#include <exception>
#include <string>

struct clientInfo {
    std::string ip;
    int tempo;
};

int asciiStringToInt(char* str);

#endif