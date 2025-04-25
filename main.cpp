#include <iostream>
#include <string>
#include <vector>

int asciiStringToInt(const char* str) {

    std::vector<char> vec(str, str+4);

    if (vec.size() != 4) {
        throw std::invalid_argument("A string deve ter exatamente 4 caracteres.");
    }

    // Obter o valor de cada byte na string e formar o inteiro
    int result = 0;
    for (size_t i = 0; i < 4; ++i) {
        result |= static_cast<unsigned char>(vec[i]) << (8 * (3 - i));  // Desloca os bytes para a posição correta
    }
    
    return result;
}

int main() {
    const char* str = "\0\0\n";  // Exemplo de string com 4 caracteres ASCII
    try {
        int value = asciiStringToInt(str);
        std::cout << "Valor convertido: " << value << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Erro: " << e.what() << std::endl;
    }

    return 0;
}
