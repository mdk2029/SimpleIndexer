
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <thread>
#include <exception>
#include <chrono>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
#include "Utility.h"
#include "DirectoryTraverser.h"
#include "MMapWorkItem.h"
#include "IndexingWorkQueue.h"
#include "SimpleWordCount.h"
#include "PriorityQueue.h"
#include "Indexer.h"
#include "Policies.h"
#include "gtest/gtest.h"

using namespace boost::filesystem;
using namespace std;
using SimpleIndexer::Indexer;
using SimpleIndexer::SimpleWordCount;
using SimpleIndexer::FileFilter;
using SimpleIndexer::DirectoryTraverser;
using SimpleIndexer::IndexingWorkQueue;
using SimpleIndexer::IndexingWorkItem;
using SimpleIndexer::NopLockingPolicy;
using SimpleIndexer::MutexLockingPolicy;
using SimpleIndexer::PriorityQueue;

void die(const char* prgname) {
    std::cerr << "Valid invocations :" << std::endl;
    std::cerr << "(1) " << prgname << " --tests" << std::endl;    
    std::cerr << "(2) " << prgname << " -t <N> </path/to/directory> [--dump-full-index] [--all-files]" << std::endl;
    exit(EXIT_FAILURE);
}

template<class Container>
void report(const Container& cont, bool dumpIndex, size_t topCount) {
    using std::cout;
    if(dumpIndex) {
        for(const auto& wordCount : cont) {
            cout << wordCount.first << '\t' << wordCount.second << endl;
        }
    }
    else {
        using Pair_t = std::pair<typename Container::key_type, typename Container::mapped_type>;
        PriorityQueue<Pair_t> pq(topCount,std::begin(cont),std::end(cont));
        while(!pq.empty()) {
            Pair_t next;
            pq.pop(next);
            std::cout << next.first << '\t' << next.second << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    
    bool test = false;
    bool help = false;
    size_t numIndexingThreads = 0;
    string startDir = ".";
    size_t count = 10;
    bool dumpIndex = false;
    bool allFiles = false;
    
    Utilities::parseArgs(argc,argv,help,test,numIndexingThreads,startDir,count,dumpIndex,allFiles);
    if(help) {
        die(argv[0]);
    }

    if(test) {
        testing::InitGoogleTest(&argc, argv);
        int val = RUN_ALL_TESTS();
        return val ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    
    std::clog << "Parameters: " << std::endl
            << "start-dir            = " << startDir << std::endl
            << "num-indexing-threads = " << numIndexingThreads << std::endl
            << "count                = " << count << std::endl
            << "dump-full-index      = " << std::boolalpha << dumpIndex << std::endl
            << "all-files            = " << std::boolalpha << allFiles << std::endl;    
    
    SimpleIndexer::FileFilter filter{ allFiles ? "(.)*" : "((.|\n)*\\.txt)" };
    DirectoryTraverser dTraverser{path{startDir},filter};
    Indexer indexer;        

    if(numIndexingThreads == 0) {
        
        //Singlethreaded operation. Used mostly for testing

        SimpleWordCount<NopLockingPolicy> words;

        path fPath; size_t fSize;
        while(dTraverser.next(fPath,fSize)) {
            SimpleIndexer::IndexingWorkItem wItem{fPath,fSize};
            indexer.process(wItem,words);
        }
        
        report(words,dumpIndex,count);
    }
    
    else if(numIndexingThreads > 0) {
        
        //This is the queue between the dir-traversing thread and the indexing threads
        IndexingWorkQueue workQueue;
        
        //Each thread that the indexer launches will read from workQueue and will write to a SimpleWordCount
        //Please see README for design details. The threads will be operating mostly on
        //local containers and only occasionally aggregate data into the shared container

        SimpleWordCount<MutexLockingPolicy> words;
        
        std::clog << "Launching " << numIndexingThreads << " indexing threads." << std::endl;
        for(size_t i = 0; i< numIndexingThreads; i++) {
            indexer.asyncIndex(workQueue,words);
        }
        
        //Now launch the dir-traversing part of the system. i.e. the part that will enqueue WorkItems into
        //the workqueue
        std::clog << "Launching dir-traverser." << std::endl;
        dTraverser.asyncTraverse(workQueue);
        
        //Wait for the traverser to finish
        dTraverser.waitTillAsyncTraverseDone();
        std::clog << "Done traversing." << std::endl;

        //At this point, we are done with the dir crawling and all the indexing threads are building the index 
        //Enqueue sentinel values to get the indexing threads to quit
        for(size_t i = 0; i < indexer.batchSize()*numIndexingThreads; i++) {
            workQueue.put(IndexingWorkItem{});
        }

        //Now wait for the indexing threads to finish
        indexer.waitTillDone();

        //Report either full index or top count words
        report(words,dumpIndex,count);
    }
    
    return EXIT_SUCCESS;
}

