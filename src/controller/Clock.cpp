#include "Clock.hpp"

Clock::Clock(std::unordered_map<std::string, clientInfo> clients) : clients(clients) {
    // Construtor da classe Clock
}

void Clock::Start() {
    if(running) return; // Se já estiver rodando, não faz nada
    std::cout << "Iniciando o relógio." << std::endl;

    // Inicia a contagem em uma nova thread
    std::thread counterThread(&Clock::_counter, this);
    counterThread.detach(); // Desanexa a thread para que continue rodando em segundo plano
}

void Clock::Stop() {
    running = false; // Para o relógio
    std::cout << "Parando o relógio." << std::endl;
}

void Clock::_counter() {
    
    running = true; // Inicia o relógio

    while (running) {

        for (auto& client : clients) {
            std::lock_guard<std::mutex> lock(clockMutex); // Protege o acesso à lista de clientes
            // std::cout << "Contando para o cliente: " << client.first << " com IP: " << client.second.ip << std::endl;
            
            if(client.second.tempo == 10) {
                clients.erase(client.first);
                std::cout << "Cliente " << client.first << " removido por timeout." << std::endl;
            } else {
                client.second.tempo++;
            }
        }

        if(clients.empty()) {
            std::cout << "Nenhum cliente conectado." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Relógio parado." << std::endl;
}

bool Clock::HandleNewClient(std::string name, clientInfo client) {
    std::lock_guard<std::mutex> lock(clockMutex); // Protege o acesso à lista de clientes
    std::cout << "Relógio parado." << std::endl;
    clients.insert_or_assign(name, client);
    return true;
}

Clock::~Clock() {
    Stop(); // Para o relógio
    std::cout << "Destruindo o relógio." << std::endl;
}

