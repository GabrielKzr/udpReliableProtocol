#ifndef UTILS_HPP
#define UTILS_HPP
    
#pragma once

#include <exception>
#include <string>
#include <stdint.h>
#include <stdexcept>

struct clientInfo {
    std::string ip;
    int tempo;
};

int asciiStringToInt(char* str);
void intToLogicVectorLittleEndian(int value, uint8_t* output, int outputSize); 
int uint8_to_int(uint8_t bytes[4]);

#endif