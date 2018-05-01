#include "Utility.h"
#include <boost/program_options.hpp>
#include <stdexcept>
#include <cstring>
#include <random>

namespace Utilities {
    
    void parseArgs(int argc, char** argv, bool& help, bool& tests, size_t& numWorkerThreads, std::string& directory, size_t& count, bool& dumpFullIndex, bool& allFiles) {
        
        namespace po = boost::program_options;
        
        po::options_description desc("Allowed options");
        desc.add_options()
        ("help", "produce help message")
        ("tests","run tests")
        ("threads,t", po::value<size_t>(&numWorkerThreads)->default_value(0),"num worker threads")
        ("root-dir,d", po::value<std::string>(&directory),"starting dir")
        ("count,c", po::value<size_t>(&count)->default_value(10), "num words to output")
        ("dump-full-index,d","dump full index instead of top 10")
        ("all-files,a","Look at all accessible files and not just *.txt files")   
        ;
        
        po::positional_options_description p;
        p.add("root-dir", 1);
        
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);
        
        if(vm.count("help"))
            help = true;
        if(vm.count("tests"))
            tests = true;
        if(vm.count("dump-full-index"))
            dumpFullIndex = true;
        if(vm.count("all-files"))
            allFiles = true;
    }
    
    int getRandom(size_t beg, size_t end) {
        
        static const size_t seed = ::getpid() * ::time(nullptr);
        static std::default_random_engine dre(seed);
        
        std::uniform_int_distribution<int> ud(beg,end);
        size_t start = ud(dre);
        for(; start <= end; start++) {
            bool isPrime = true;
            for(size_t d = 2; d <= start/2; d++) {
                if( start%d == 0) { 
                    isPrime = false; 
                    break; 
                }
            }
            if(isPrime) return start;
        }

        return ud(dre);
    }
    
}


