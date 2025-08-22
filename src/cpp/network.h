#pragma once

#include <string>

class Network {
public:
    Network();
    ~Network();
    
    std::string fetch_url(const std::string& url);
    
private:
    std::string last_url;
};
