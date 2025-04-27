#include "../include/Console.hpp"

std::string Console::trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\n\r");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\n\r");
    return s.substr(b, e - b + 1);
}

std::string Console::toLower(const std::string& s) {
    std::string o = s;
    std::transform(o.begin(), o.end(), o.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return o;
}

void Console::cleanConsole() {
    // Limpa o console
    std::cout << "\033[2J\033[1;1H"; // ANSI escape code para limpar o console
}

void Console::printMenu() {
    std::cout << "Menu:\n";
    std::cout << "1. Talk: <type> <data> <name>\n";
    std::cout << "2. File: <type> <path> <name>\n";
    std::cout << "3. Exit: <type>\n";
}

std::pair<std::string, std::pair<std::string, std::string>> Console::handleInput() {

    std::string type;
    std::string data;
    std::string name;

    std::cout << "Digite o tipo de mensagem: " << std::endl;
    std::cin >> std::ws; 

    std::cin >> type;

    type = toLower(trim(type));

    if (type == "1")        type = "talk";
    else if (type == "2")   type = "file";
    else if (type == "3")   type = "exit";

    if (type == "exit") {
        return std::make_pair("exit", std::make_pair("", ""));
    }

    std::cin >> data;
    std::cin >> name;

    // Remove espaços nas bordas e converte pra minúsculas

    if (type == "talk") {
        if(data.size() > 1430) {
            std::cout << "Mensagem muito longa. O tamanho máximo é 1430 caracteres.\n";
            return std::make_pair("", std::make_pair("", ""));
        }

        return std::make_pair(type, std::make_pair(name, data));
    }
    else if (type == "file") {

    }
    else {
        std::cout << "Opção inválida: '" << type << "'. Por favor escolha Talk, File ou Sair.\n";
    }
    
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Limpa o buffer de entrada   

    return std::make_pair("", std::make_pair("", ""));
}