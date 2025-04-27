#include "Clock.hpp"

Clock::Clock(std::unordered_map<std::string, clientInfo>& clients) : clients(clients) {

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

        {
            std::lock_guard<std::mutex> lock(clockMutex); // Protege o acesso à lista de clientes

            for (auto it = clients.begin(); it != clients.end(); /*…*/) {
                if (it->second.tempo >= 10) {
                    it = clients.erase(it);
                } else {
                    ++it->second.tempo;

                    // std::cout << "Cliente: " << it->first << ", Tempo: " << it->second.tempo << std::endl;

                    ++it;
                }
            }
            
            if(clients.empty()) {
                // std::cout << "Nenhum cliente conectado." << std::endl;
            }
        }
            
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Relógio parado." << std::endl;
}

bool Clock::HandleNewClient(std::string name, clientInfo client) {
    std::lock_guard<std::mutex> lock(clockMutex); // Protege o acesso à lista de clientes
    
    auto result = clients.insert_or_assign(name, client);

    /*
    for (const auto& [key, value] : clients) {
        std::cout << "Cliente: " << key << ", IP: " << value.ip << ", Tempo: " << value.tempo << std::endl;
    }    
    // Verifica o resultado da operação
    if (result.second) {
        std::cout << "Novo cliente inserido: " << name << " com IP: " << client.ip << std::endl;
    } else {
        std::cout << "Cliente existente atualizado: " << name << " com IP: " << client.ip << std::endl;
    }
    */

    return true;
}

clientInfo* Clock::getClientInfo(std::string name) {

    
    std::lock_guard<std::mutex> lock(clockMutex); // Protege o acesso à lista de clientes
    
    auto it = clients.find(name);
    if (it != clients.end()) {
        
        clientInfo* client = new clientInfo();
        
        client->ip = it->second.ip;
        client->tempo = it->second.tempo;

        return client; // Retorna o ponteiro para o clientInfo
    }
    
    return nullptr; // Retorna nullptr se não encontrar
}

Clock::~Clock() {
    Stop(); // Para o relógio
    std::cout << "Destruindo o relógio." << std::endl;
}

