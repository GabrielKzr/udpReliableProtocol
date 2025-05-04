#include "../include/Utils.hpp"

void intToLogicVectorLittleEndian(int value, uint8_t* output, int outputSize) {

    if(outputSize < 4) {
        throw std::length_error("Output size is less than 4 bytes");
    }
    for (int i = 0; i < 4; ++i) {
        output[i] = (value >> (i * 8)) & 0xFF;
    }
}

int uint8_to_int(uint8_t bytes[4]) {
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        result |= static_cast<int>(bytes[i]) << (8 * i); // Deslocamento para little-endian
    }
    return result;
}

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // lê o conteúdo inteiro para o buffer

    return buffer.str(); // retorna como string
}