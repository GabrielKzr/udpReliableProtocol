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