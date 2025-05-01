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
                    std::cout << "Cliente Desconectado: " << it->first << ", Tempo: " << it->second.tempo << std::endl;
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
    */
    if (result.second) {
        std::cout << "Novo cliente inserido: " << name << " com IP: " << client.ip << std::endl;
    } 
    /*
    else {
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

bool Clock::containsClient(std::string ip) {
    std::lock_guard<std::mutex> lock(clockMutex); // Protege o acesso à lista de clientes
    
    for (const auto& client : clients) {
        if (client.second.ip == ip) {
            return true; // Retorna true se encontrar o cliente
        }
    }
    
    return false; // Retorna false se não encontrar
}

bool Clock::addClientFile(Message_t packet) {

    /* // não bloqueia, mesmo não estando na lista de clientes
    if(clients.find(packet.name) == clients.end()) {
        return false;
    }
    */

    auto it = filesManagement.find(packet.name);

    if(it != filesManagement.end()) {

        for(auto& packets : it->second) { // tratamento de pacotes duplicados
            if(packets.seq == packet.seq) {
                std::cout << "Pacote já recebido: " << packet.seq << std::endl;
                return false; // Pacote já recebido
            }
        }

        it->second.push_back(packet);
    } else {
        filesManagement.insert(std::make_pair(packet.name, std::vector<Message_t>{packet}));
    }

    if(filesManagement[packet.name].size() == packet.length) {
        if(!_handleCompleted(packet.name))
            return false;
    }


    return true;
}

bool Clock::_handleCompleted(std::string name) {

    std::vector<Message_t> packets = filesManagement[name];
    std::stringstream ss;
    std::string data;
    std::vector<uint8_t> digest(MD5_DIGEST_LENGTH); // MD5_DIGEST_LENGTH = 16 bytes

    for (size_t i = 0; i < packets.size(); i++) {
        for (size_t j = 0; j < packets.size() - i - 1; j++) {
            if (packets[j].seq > packets[j + 1].seq) {
                std::swap(packets[j], packets[j + 1]);
            }
        }
    }

    for(size_t j = 0; j < packets.size(); j++){
        ss << packets[j].payload;
    }

    data = ss.str();
    
    MD5(reinterpret_cast<const uint8_t*>(data.c_str()), data.size(), digest.data());

    for(size_t j = 0; j < 16; j++){
        if (packets[packets.size()-1].hash[j] != digest[j]){
            
            return false;
        }
    }

    // std::cout << "PAçoca\n";

    _createFile(packets);


    filesManagement.erase(name);

    return true;
}

void Clock::_createFile(std::vector<Message_t> packets) {
    std::string filename = "./exampleFiles/receivedFiles/" + std::string(packets[0].fileName);

    std::ofstream outfile(filename); // modo texto

    if (!outfile.is_open()) {
        std::cerr << "Erro ao criar o arquivo " << filename << std::endl;
        return;
    }

    for (size_t i = 0; i < packets.size(); ++i) {
        const auto& packet = packets[i];

        std::string data(reinterpret_cast<const char*>(packet.payload), packet.payloadSize);

        outfile << data;
    }

    outfile.close();
    std::cout << "Arquivo criado: " << filename << std::endl;
}

void Clock::clientsToString() {
    std::lock_guard<std::mutex> lock(clockMutex); // Protege o acesso à lista de clientes
    
    if(clients.empty()) {
        std::cout << "Nenhum cliente conectado." << std::endl;
        return;
    }

    for (const auto& client : clients) {
        std::cout << "Cliente: " << client.first << ", IP: " << client.second.ip << ", Tempo: " << client.second.tempo << ", Port: " << client.second.port << std::endl;
    }
}

bool Clock::HandleTalkMessage(Message_t packet) {    
    
    for (auto it = talkTempReceivedPackets.begin(); it != talkTempReceivedPackets.end(); ++it) {
        if (std::memcmp(it->id, packet.id, 4) == 0 &&
            std::memcmp(it->ip, packet.ip, 16) == 0) {
            // std::cout << "Mensagem já recebida: " << packet.payload << std::endl;
            return false; // Mensagem já recebida
        }
    }
    
    // limpa a primeira mensagem temporária se o tamanho for maior que 1024
    if(talkTempReceivedPackets.size() >= 1024) {
        talkTempReceivedPackets.erase(talkTempReceivedPackets.begin());
    }

    talkTempReceivedPackets.push_back(packet);

    return true;
}

Clock::~Clock() {
    Stop(); // Para o relógio
    std::cout << "Destruindo o relógio." << std::endl;
}

