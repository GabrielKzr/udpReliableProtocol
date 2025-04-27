#include "../include/Utils.hpp"

// !!!!!!!!!!!!!! pode dar exception
int asciiStringToInt(char* str){
    int result = 0;
    // Para cada byte da string, deslocamos e combinamos os valores
    for (size_t i = 0; i < 4; ++i) {
        result |= static_cast<unsigned char>(str[i]) << (8 * (3 - i));  // Deslocamento correto dos bytes
    }

    return result;
}

void intToLogicVectorLittleEndian(int value, uint8_t* output, int outputSize) {

    if(outputSize < 4) {
        throw std::length_error("Output size is less than 4 bytes");
    }
    for (int i = 0; i < 4; ++i) {
        output[i] = (value >> (i * 8)) & 0xFF;
    }
}
