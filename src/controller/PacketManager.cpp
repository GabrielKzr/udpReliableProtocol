#include "../include/PacketManager.hpp"
PacketManager::PacketManager(int port) : port(port) {
    actualMessage = nullptr;
    actualSystemId = 0;
}

Message_t PacketManager::buildMessage(std::string name, std::string message) {
    char name_s[20] = {0};  // Array de chars para 'name'
    uint8_t hash[16] = {0};     // Array de hash (16 bytes)

    uint8_t type = 2;  // Tipo de mensagem
    uint8_t id[4] = {0}; 
    intToLogicVectorLittleEndian(this->actualSystemId, id, 4);  // Converte ID para little endian

    strncpy(name_s, name.c_str(), sizeof(name_s) - 1); 

    const char* name_as_char = name_s;

    const uint8_t* payload = reinterpret_cast<const uint8_t*>(message.c_str());

    Message_t msg(type, id, name_as_char, 0, 0, hash, 0, payload);

    return msg; // Se necessário
}

bool PacketManager::sendMessage(Message_t packet, std::string ip, int sock) {

    if(actualMessage != nullptr && !actualMessage->acked) {
        std::cout << "Mensagem anterior não foi 'acked'. Não é possível enviar nova mensagem." << std::endl;
        return false; // Não pode enviar nova mensagem se a anterior não foi "acked"
    }

    if(sock == -1) {
        std::cerr << "Socket inválido." << std::endl;
        return false; // Socket inválido
    }

    Message* msg = new Message();
    msg->msg = &packet;
    msg->acked = false; // Inicializa como não "ackeado"
    this->actualMessage = msg;

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);  // Porta do servidor de escuta
    addr.sin_addr.s_addr = inet_addr(ip.c_str()); // IP do cliente

    int n = sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr));
    if (n < 0) {
        std::cerr << "Erro ao enviar pacote!" << std::endl;
        return false;
    }    

    // precisa validar um ack, senão pacote pode só ser perdido

    return true; // Retorna true se o envio for bem-sucedido
}

/*
bool PacketManager::verifyAck(int ack) {
    
if(ack == this->actualMessage->id) {
    
// se já tinha sido "ackeado", então retorna false (ignore repetitive acks)
if(isAcked()) {
    return false;
}

return true;
} 

return false;
}

bool PacketManager::isAcked() {
    return this->actualMessage->acked;
}

bool PacketManager::sendMessage(std::string message, int id) {
    
if(!isAcked()) {
    return false;
}

// lógica para envio de uma mensagem (talvez para enviar precise de outras informações)
// talvez o retorno dessa função não possa ser bool, mas acho que não é a melhor escolha mexer no retorno

return true;
}

void PacketManager::retransmitPacket() {
    
// lógica para a retransmissão de um pacote
// mesma coisa da lógica de envio, mas ainda não sei qual a melhor opção
}
*/


