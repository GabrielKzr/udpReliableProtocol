#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>

class Console {

    private:

        std::string trim(const std::string& s);
        std::string toLower(const std::string& s);

    public:

        void cleanConsole();
        void printMenu();
        std::pair<std::string, std::pair<std::string, std::string>> handleInput();

};

#endif