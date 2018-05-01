#pragma once
#include <queue>
#include <stack>
#include <stdexcept>

namespace SimpleIndexer {

    template<class WordFreqPair>    
    struct FreqComparator {
        bool operator()(const WordFreqPair& lhs, const WordFreqPair& rhs) const {
            return lhs.second > rhs.second;
        }
    };

    //This class uses a priority queue extract the top N most frequent words
    //from range [begin,end)

    template<class T>    
    class PriorityQueue {
        
        using PQType = std::priority_queue<T,std::vector<T>,FreqComparator<T>>;
        
        size_t num_{10};        
        PQType pQueue_;
        std::stack<T> pStack_;
        
        public:
            
            using value_type = typename PQType::value_type;
            
            template<class Iterator>
            PriorityQueue(size_t num,Iterator begin, Iterator end) : num_(num) {
                for(; begin != end; begin++) {
                    if(pQueue_.size() < num_) 
                        pQueue_.push(*begin);
                    else {
                        if(begin->second > pQueue_.top().second) {
                            pQueue_.pop();
                            pQueue_.push(*begin);
                        }
                    }
                }
                
                //We have the top num items, but they will be extracted in reverse
                //order. So push them into a stack to get them to be extracted 
                //in the right order
                while(!pQueue_.empty()) {
                    value_type next = pQueue_.top();
                    pQueue_.pop();
                    pStack_.push(next);
                }
            }
            
            bool empty() const {
                return pStack_.empty();
            }
            
            void pop(value_type& dest) {
                if(empty()) throw std::runtime_error("Queue already empty");
                dest = pStack_.top();
                pStack_.pop();
            }
    };
    
    
    
    
}
