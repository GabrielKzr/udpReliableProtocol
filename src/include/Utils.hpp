#ifndef UTILS_HPP
#define UTILS_HPP
    
#pragma once

#include <exception>
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

struct clientInfo {
    std::string ip;
    int tempo;
    int port;
};

int asciiStringToInt(char* str);
void intToLogicVectorLittleEndian(int value, uint8_t* output, int outputSize); 
int uint8_to_int(uint8_t bytes[4]);
std::string readFileToString(const std::string& filename);


#endif