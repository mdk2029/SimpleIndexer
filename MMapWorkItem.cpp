#include "MMapWorkItem.h"
#include "Errors.h"
#include <memory>
#include <string>
#include <boost/scope_exit.hpp>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using namespace boost::filesystem;

namespace SimpleIndexer {
    
    static constexpr size_t PAGE_SIZE = 4096;
    
    void MMapWorkItem::_init(int fd, size_t startOffset, size_t endOffset) {

        void* b = ::mmap(nullptr, (endOffset-startOffset), PROT_READ, MAP_PRIVATE|MAP_NORESERVE,
                  fd, startOffset);
        
        if(b == MAP_FAILED){
            std::unique_ptr<char []> buf(new char[256]);
            throw WorkItemConstructionError(string("MMapWorkItem construction failed : ") + strerror_r(errno, buf.get(),256));
        }
        
        beg_ = static_cast<char*>(b);
        end_ = beg_ + (endOffset-startOffset);
    }
    
    MMapWorkItem::MMapWorkItem(int fd, size_t startOffset, size_t endOffset) {
        if(startOffset % PAGE_SIZE)
            throw WorkItemConstructionError("MMapWorkItem must begin on page boundary");
        if(endOffset <= startOffset)
            throw WorkItemConstructionError("Invalid range in MMapWorkItem : " + std::to_string(startOffset) + " - " + std::to_string(endOffset));
        
        _init(fd,startOffset,endOffset);
    }
    
    MMapWorkItem::MMapWorkItem(path fPath, size_t fSize) {

        if(!fSize)
            throw WorkItemConstructionError("MMapWorkItem requires nonzero fSize");
        
        int fd = ::open(fPath.string().c_str(),O_RDONLY);
        if(fd == -1) {
            std::unique_ptr<char []> buf(new char[256]);
            char* e = strerror_r(errno,buf.get(),256);
            throw WorkItemConstructionError("Skipping " + fPath.string() + " because open failed with error : " + e);
        }

        BOOST_SCOPE_EXIT_ALL(=){
            ::close(fd);
        };
        
        _init(fd,0,fSize);
    }
    
    MMapWorkItem::MMapWorkItem(MMapWorkItem&& rhs) : beg_(nullptr), end_(nullptr) {
        std::swap(beg_,rhs.beg_);
        std::swap(end_,rhs.end_);
    }
    
    MMapWorkItem& MMapWorkItem::operator=(MMapWorkItem&& rhs) {

        //To avoid surprises, we will not follow the usual paradigm of swapping with rhs 
        //but instead unmap self here itself if needed        
        if(beg_)
            ::munmap(beg_, (end_-beg_));
        
        beg_ = end_ = nullptr;
        std::swap(beg_,rhs.beg_);
        std::swap(end_,rhs.end_);
        
        return *this;
    }
    
    MMapWorkItem::~MMapWorkItem()
    {
        if(beg_)
            ::munmap(beg_,end_-beg_);
    }
}
