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
    std::cout << "1. Talk: <type> <name> <data>\n";
    std::cout << "2. File: <type> <name> <path>\n";
    std::cout << "3. Devices: <type>\n";
    std::cout << "4. Exit: <type>\n";
}


std::pair<std::string, std::pair<std::string, std::string>> Console::handleInput() {
    std::cin.clear();
    // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // limpa lixo anterior

    std::string linha;
    std::getline(std::cin, linha);

    std::istringstream iss(linha);
    std::string type, name, data;

    iss >> type >> name;
    std::getline(iss, data); // pega o resto da linha como mensagem
    data = trim(data);       // remove espaços nas bordas, se necessário

    type = toLower(type);

    if (type == "1")        type = "talk";
    else if (type == "2")   type = "file";
    else if (type == "3")   type = "devices";
    else if (type == "4")   type = "exit";

    if (type == "exit" || type == "devices") {
        return std::make_pair(type, std::make_pair("", ""));
    }

    if (data.size() > 1410) {
        std::cout << "Mensagem muito longa. O tamanho máximo é 1410 caracteres.\n";
        return std::make_pair("", std::make_pair("", ""));
    }

    if (type == "talk" || type == "file") {
        return std::make_pair(type, std::make_pair(name, data));
    } else {
        std::cout << "Opção inválida: '" << type << "'. Por favor escolha Talk, File ou Exit.\n";
        return std::make_pair("", std::make_pair("", ""));
    }
}