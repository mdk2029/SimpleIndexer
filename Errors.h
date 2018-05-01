#pragma once

#include <stdexcept>

namespace SimpleIndexer {

    class WorkItemConstructionError : public std::runtime_error {
    public :
        WorkItemConstructionError(const std::string& msg) : std::runtime_error(msg) {}
    };
    
}
