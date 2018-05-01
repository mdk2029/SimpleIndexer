#pragma once

#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <cstdint>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace SimpleIndexer {
    
    class MMapWorkItem {

        char* beg_{nullptr};
        char* end_{nullptr};
        
        void _init(int fd, size_t startOffset, size_t endOffset);
        
    public:

        MMapWorkItem() {}
        MMapWorkItem(int fd, size_t startOffset, size_t endOffset);
        MMapWorkItem(boost::filesystem::path, size_t fSize);        
        

        //MMapWorkItem will have move-only semantics to avoid possibly munmapping twice
        MMapWorkItem(const MMapWorkItem&) = delete;
        MMapWorkItem& operator=(const MMapWorkItem&) = delete;        
        
        MMapWorkItem(MMapWorkItem&& rhs);
        MMapWorkItem& operator=(MMapWorkItem&& rhs);
        
        ~MMapWorkItem();
        
        explicit operator bool() const {
            return beg_ != nullptr;
        }
        
        void willNeed() {
            ::posix_madvise(beg_,end_-beg_,POSIX_MADV_WILLNEED);
        }
        
        char* begin() { return beg_; }
        char* end() { return end_; }        
    };
    
}
