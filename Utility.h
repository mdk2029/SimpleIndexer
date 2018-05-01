#pragma once
#include <string>
#include <cstdint>
#include <locale>

//These are utility functions that would normally be available in the existing codebase

namespace Utilities {
    
    void parseArgs(int argc, char** argv, bool& help, bool& tests, size_t& numWorkerThreads, std::string& directory, size_t& count, bool& dumpIndex, bool& allFiles);

    //Will try to return a random prime between [start,end] or else just a random number in [start,end]    
    int getRandom(size_t start, size_t end);
}
