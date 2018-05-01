#pragma once
#include <unordered_map>
#include <string>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <locale>
#include <ext/vstring.h>
#include "Policies.h"
#include "gtest/gtest_prod.h"

namespace SimpleIndexer {

    //SimpleWordCount class stores the index (word/count pairs). 
  
    //The class itself is agnostic of locking. The locking is controlled by the first template parameter.
    //Intended usage of this class is via a std::unique_lock
    //  void foo(SimpleWordCount<SomePolicy>& wCount) {
    //     std::unique_lock<SimpleWordCount<SomePolicy>> lock(wCount);
    //     wCount.insert(beg,end);
    //  }
    // Doing so will ensure that calling code stays thread safe irrespective of the locking policy
    // 

#if __cplusplus >= 201103L
    using ssostring_t = std::string;
#else
    using ssostring_t = __gnu_cxx::__vstring;
#endif

    using stdhashmap_t = std::unordered_map<ssostring_t,size_t>;
            
    template<class LockingPolicy, class HashTable_t=stdhashmap_t>
    class SimpleWordCount : public LockingPolicy {
        FRIEND_TEST(SimpleIndexer, SimpleWordCountTest);
        FRIEND_TEST(SimpleIndexer, SimpleWordCountMergeTest);
    public:        
        using WordMap_t = HashTable_t;
        using iterator = typename WordMap_t::iterator;                
        using const_iterator = typename WordMap_t::const_iterator;                
        using key_type = typename WordMap_t::key_type;
        using mapped_type = typename WordMap_t::mapped_type;        
        
    private:        
        WordMap_t wordMap_;
        
    public:        
        template<typename InputIterator>
        void insert(InputIterator begin, InputIterator end) {
            key_type key;
            key.reserve(end-begin);
            std::transform(begin,end,std::back_inserter(key),
                                     [](const char& ch) { return std::toupper(ch,std::locale());});
            auto itr = wordMap_.find(key);
            if(itr != wordMap_.end())
                itr->second++;
            else
                wordMap_.emplace(std::move(key),1);
        }
        
        void clear() { wordMap_.clear(); }
        bool empty() const { return wordMap_.empty(); }
        size_t size() const { return wordMap_.size(); }
        
        iterator begin() { return wordMap_.begin();}
        iterator end()   { return wordMap_.end();}

        const_iterator begin() const { return wordMap_.cbegin();}
        const_iterator end()   const { return wordMap_.cend();}

        const_iterator cbegin() { return wordMap_.cbegin();}
        const_iterator cend()   { return wordMap_.cend();}
        
        template<class Rhs>
        SimpleWordCount& operator += (const Rhs& rhs) {
            for(const auto& wc : rhs) {
                if(!wc.first.empty()) {
                    auto itr = wordMap_.find(wc.first);
                    if(itr != wordMap_.end())
                        itr->second += wc.second;
                    else
                        wordMap_[wc.first] = wc.second;
                }
            }
            return *this;
        }
    };
    
}
