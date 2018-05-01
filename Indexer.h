#pragma once

#include "ExtractWords.h"
#include "MMapWorkItem.h"
#include "IndexingWorkQueue.h"
#include "SimpleWordCount.h"
#include <unordered_map>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <vector>
#include <thread>
#include <boost/optional.hpp>

namespace SimpleIndexer {

    class Indexer {
        
    public:

        //This function processes one workItem and aggregates results into cont
        //Caller is required to set things up such that cont is either exclusive per thread
        //or should be a thread safe container or be locked before getting here
        template<class OutputContainer>
        void process(IndexingWorkItem& wItem, OutputContainer& cont) {
            if(wItem)
                extractWords(wItem.begin(),wItem.end(),cont);
        }

        //Similar as above, except takes a file path instead of a MMappedWorkItem
        //Useful mostly for debugging
        template<class OutputContainer>        
        void process(boost::filesystem::path fPath, OutputContainer& cont) {
            try {
                
                std::ifstream iFile(fPath.string());
                std::string line;
                while(std::getline(iFile,line)) {
                    extractWords(std::begin(line), std::end(line),cont); 
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << "Skipping " << fPath.string() << " because of exception : " << e.what() << std::endl;
            }
        }
        
        //Each time this function is called, it will launch a new indexing thread.
        //It is not thread safe and is meant to be called from the main thread when 
        //the program is setting up all the components
        template<typename OutputContainer>
        void asyncIndex(IndexingWorkQueue& inQ, OutputContainer& cont) {

            eptrs_.push_back(std::exception_ptr());
            auto& eptr = eptrs_.back();

            //Each thread will get a different budget to minimize them wanting to drain their local 
            //results into the global container at the same time. The first thread will try and get 
            //a random prime between [10000,20000], the second thread will try and get a random prime 
            //between [20000,30000] and so on
            
            size_t bStart = threads_.size() + 1;
            boost::optional<unsigned> localBudget{Utilities::getRandom(bStart*10000,(bStart+1)*10000)};
            //boost::optional<unsigned> localBudget = boost::none;     
            
            static size_t id = 0;
            std::string name = "Indexing Thread " + std::to_string(id++);
            
            auto tfunc = [this,&inQ,&cont,&eptr,localBudget,name](){ _index(inQ,cont,eptr,localBudget,name); };
            threads_.emplace_back(tfunc);
        }
        
        size_t numThreads() const {
            return threads_.size();
        }
        
        void waitTillDone() {
            //First join all threads;            
            std::for_each(threads_.begin(), threads_.end(), [](std::thread& thr){ if(thr.joinable()) thr.join();});
            
            //Now re-throw the first exception if any. Beware that subsequent exceptions are lost
            std::for_each(eptrs_.begin(), eptrs_.end(), [](std::exception_ptr& eptr){ if(eptr) std::rethrow_exception(eptr); });
        }
        
        ~Indexer() {
            waitTillDone();  //Beware - If a thread exits via an exception, it will get rethrown here
        }
        
        size_t batchSize() const  {
            //Totally random value. Batching is good because it reduces contention on the shared queue. 
            //But coming up with a batch size is not easy. The queue should ideally do some dynamic load 
            //balancing to handout "equal" chunks of work to all the threads;
            return 32;
        }
            
    private:        
        
        std::vector<std::thread> threads_;
        std::list<std::exception_ptr> eptrs_; //Launched threads will have a reference to an eptr. So do not use vector to avoid invalidation-due-to-resizing issues
        

        //This is the function invoked by the indexing threads. It will forever get a batch of items
        //from the workQueue and process the items.
        //
        //The loop terminates if we get an empty workItem.
        //
        //If a thread exits via an exception, we will catch it and store it in eptr to 
        //allow it to be re-thrown elsewhere
        //
        //Please see README for design. Each thread will operate on a local container 
        //and periodically flush to the global container only when the global container is not contended
        
        template<class OutputContainer>        
        void _index(IndexingWorkQueue& inQ, OutputContainer& cont, std::exception_ptr& eptr, boost::optional<unsigned> localBudget, const std::string& name) noexcept {

            SimpleWordCount<NopLockingPolicy> localWordCount_;
            size_t lockAttempts = 0, lockFails = 0;            
            
            try {
                
                bool done = false;
                while(!done) {
                    std::vector<IndexingWorkItem> wItems;
                    wItems.reserve(batchSize());
                    inQ.take(std::back_inserter(wItems),batchSize());

                    //Try to minimize page faults                    
                    for(auto& wItem : wItems) {
                        if(wItem)
                            wItem.willNeed();
                    }

                    for(auto& wItem : wItems) {
                        if(!wItem) {
                            done = true; //Empty WorkItem is a signal that all is done and we should quit
                            break;
                        }
                        else {
                            process(wItem,localWordCount_);
                        }
                    }
                    
                    //If localWordCount_ is big enough and cont is currently not contended
                    if(localBudget && localWordCount_.size() > *localBudget) {
                        ++lockAttempts;
                        std::unique_lock<OutputContainer> ul(cont,std::defer_lock);
                        if(ul.try_lock()) {
                            cont += localWordCount_;
                            ul.unlock();  //Unlock explicitly so that we can clear localWordCount_ outside the mutex
                            localWordCount_.clear();
                        }
                        else
                            ++lockFails;
                    }
                }
                        
                //Done.Make a blocking lock on global cont to drain localWordCount_ and guard std::clog while printing
                std::unique_lock<OutputContainer> ul(cont);                
                if(!localWordCount_.empty()) cont += localWordCount_;                    
                printStats(std::clog, name, std::this_thread::get_id(),localBudget,lockAttempts,lockFails,!localWordCount_.empty());            
            }
            catch(...) {
                eptr = std::current_exception();
            }
        }
        

        //A helper function to log some stats when a thread is done        
        template<class Ostream>        
        void printStats(Ostream& os, const std::string& name, std::thread::id id, boost::optional<unsigned> localBudget,size_t lockAttempts,size_t lockFails,bool neededBlockingLock) {
            os << name << " [" << id << "]" << " ran with budget : " << (localBudget ? *localBudget : 0) << " ; had [ " << lockAttempts << " total / " << lockFails << " failed ] nonblocking lock attempts ; ";
                
            if(!neededBlockingLock)
                os << " and did not need a blocking lock at the end." << std::endl;
            else
                os << " and needed a blocking lock at the end." << std::endl;                    
        }
    };
}

