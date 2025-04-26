#include "../include/PacketManager.hpp"
PacketManager::PacketManager() {
    actualMessage = nullptr;
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


