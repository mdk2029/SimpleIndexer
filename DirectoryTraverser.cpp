#include "DirectoryTraverser.h"
#include "Errors.h"
#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
#include <fcntl.h>
#include <iostream>

using namespace std;
using namespace boost::filesystem;

namespace SimpleIndexer {

    bool DirectoryTraverser::next(path& fPath, size_t& fSize) {

        bool success = false;
        while(curr_ != end_ && !success) {
            std::tie(fSize,success) = filter_(curr_->path(), curr_->symlink_status());
            if(success)
                fPath = curr_->path();

            //Increment curr_ to be ready for subsequent invocation of next()            
            //Incrementing curr_ can throw if it tries to enter a directory that we do not have the right
            //perms on.
            auto f = curr_->path();
            try {
                ++curr_;
            }
            catch(const std::exception& e) {
                std::cerr << "skipping " << f << " because of :" << e.what() << std::endl;
            }
            //Else keep going till we find an acceptable file or till we have finished all iterations
        }

        return success;
    }    
    
    void DirectoryTraverser::_traverse(IndexingWorkQueue& outQ) noexcept {
        try {
            path fPath;
            size_t fSize;
            while(next(fPath,fSize)) {
                try {
                    if(fSize > 0)
                        outQ.put(IndexingWorkItem{fPath,fSize});
                }
                catch(const WorkItemConstructionError& e) {
                    std::cerr << e.what() << std::endl;
                }
            }
        }
        catch(...){
            eptr_ = std::current_exception();
        }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // FileFilter implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////    
        
    std::pair<size_t,bool> 
    FileFilter::operator()(const path& fPath, const file_status& status) {
        
        size_t filesz{0};
        bool success{false};
        
        //Does it have expected name pattern
        if(std::regex_match(fPath.c_str(),nameRegex_)) {
            //only consider regular files, no symlinks or directories and so on
            if(is_regular_file(status)) { 
                //Will we be able to read this file if we want to
                if(::eaccess(fPath.c_str(),R_OK) == 0) {
                    //Make sure this is not something we have already seen before
                    //via a hardlink
                    
                    //boost::filesystem does not cache hardlink counts or the fileSize
                    //and would have needed repeated stat syscalls to get them. So we stop 
                    //using it for the rest of this function
                    
                    struct stat statbuf{};
                    if(::lstat(fPath.c_str(),&statbuf) == 0) {
                        if( 1 == statbuf.st_nlink || inodes_.insert(std::make_pair(statbuf.st_dev,statbuf.st_ino)).second) {
                            //There are either no multiple hard-links or we have not seen this inode before
                            filesz = statbuf.st_size;
                            success = true;
                        }
                    }
                }
            }
        }
        
        return std::make_pair(filesz,success);
    }
}
