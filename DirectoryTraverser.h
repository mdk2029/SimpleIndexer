#pragma once

#include <cstdlib>
#include <cassert>
#include <unordered_set>
#include <regex>
#include <exception>
#include <thread>
#include <boost/filesystem.hpp>
#include "IndexingWorkQueue.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace SimpleIndexer {

    //FileFilter controls which files are potential candidates for indexing
    class FileFilter
    {
        struct Hasher
        {
            size_t operator()(const std::pair<dev_t,ino_t>& tup) const {
                return std::hash<dev_t>()(tup.first) ^ std::hash<ino_t>()(tup.second);
            }
        };

        std::unordered_set<std::pair<dev_t,ino_t>,Hasher>  inodes_;
        std::regex nameRegex_;

    public:
        explicit FileFilter(const char* fnamePattern) : nameRegex_{fnamePattern} 
        {}
        
        std::pair<size_t,bool> operator()(const boost::filesystem::path& pathname, const boost::filesystem::file_status& st);        
    };

    ////////////////////////////////////////////////////////////////////////////////////////////
    
    // Yields the next filename while recursively traversing the directory
    // Can be instantiated in async mode where it will launch a thread and put all discovered files onto a work queue
    class DirectoryTraverser
    {
        FileFilter filter_;
        boost::filesystem::recursive_directory_iterator curr_,end_;
        std::exception_ptr eptr_;
        std::thread thread_;
        
        void _traverse(IndexingWorkQueue& outQ) noexcept;

    public:

        DirectoryTraverser(boost::filesystem::path dir, const FileFilter& filter) : filter_(filter)
        {
            if(boost::filesystem::exists(dir) && boost::filesystem::is_directory(dir))
                curr_ = boost::filesystem::recursive_directory_iterator(dir);
        }

        //Will retrieve the next file path/size into fPath and fSize. 
        //Returns true on success
        bool next(boost::filesystem::path& fPath, size_t& fSize);
        
        void traverse(IndexingWorkQueue& outQ) {
            _traverse(outQ);
        }

        //Will launch a thread to do the crawling and extract candidate files into outQ.
        void asyncTraverse(IndexingWorkQueue& outQ) {
            assert(!thread_.joinable());
            thread_ = std::thread([this,&outQ](){ _traverse(outQ); });
        }
        
        void waitTillAsyncTraverseDone() {
            if(thread_.joinable())
                thread_.join();
            if(eptr_)
                std::rethrow_exception(eptr_);
        }
        
        ~DirectoryTraverser() {
            if(thread_.joinable())
                thread_.join();
        }
    };
}    

