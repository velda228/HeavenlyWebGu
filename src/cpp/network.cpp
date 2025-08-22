#include "network.h"

Network::Network() {
}

Network::~Network() {
}

std::string Network::fetch_url(const std::string& url) {
    last_url = url;
    // TODO: Реализовать HTTP запросы
    return "Mock content for: " + url;
}
