### SimpleIndexer

This is a toy program that crawls the local filesystem and builds an index of the most frequent words in the files it finds.

#### Design

The indexer has the following components. It is designed to separate the crawling from the task to be performed on each file. The indexing is done in helper threads that pull work items off a queue, compute results locally and then periodically merge results back to a global container.

- FileFilter 

This class is used to filter the files we want to process further. Filtering is done via the specified regex (defaults to `*.txt`), and it rules out things like :

    * Only considers regular files
    * Doesn't follow symlinks
    * Keeps track of inodes to make sure that we do not process the same file twice via a hardlink
    * Ensures that the effective uid of the program has read access on the file

- DirectoryTraverser

This class takes a starting filesystem path and a FileFilter object. Once constructed, it behaves as a generator and user can keep calling its next() method to retrieve the next file that passes the filter. The class can be used synchronously via the next() method or it can be told to launch a thread and work asynchronously. This is achieved via the asyncTraverse() method. In its async mode, whenever the traverser obtains the next file, it enqueues a MMapWorkItem representing this file into a workqueue

- MMapWorkItem

This represents the file to be indexed. We could have used stdio to read the file but mmapped reads will be more efficient - both - because of reduced memory footprint and less overhead when reading the contents. An additional advantage is that since we know our usage pattern (will need the whole range in the near future), we can minimize page faults (via techniques like MAP_POPULATE or madvise) before the indexing thread actually gets to accessing this range. We can additionally easily split one file into multiple MMapWorkItems so we can exploit parallelism within one file too.

This class has move-only semantics to rule out accidentally unmapping the same range twice.

- WorkQueue

This is a mutex protected queue allowing producer and consumers to wait for conditions. Since both sides of the system are i/o heavy , we should be fine with a mutex synchronized queue (as opposed to a lockfree spinning queue) since the queue will not be very contended. To minimize the contention, we will have the consumers read from it in batches of size N.

- Indexer

This class encapsulates the indexing work. It can either be invoked synchronously or be told to launch any number of
threads internally with each thread reading from a WorkQueue and building the index. A relevant design issue for the Indexer is - where should each thread aggregate its result? We can do one of several things : 

    * Have a global synchronized container and have each indexing thread write to it. This is the least desirable option.
    * We can have each thread build its index into a local container and then at the end merge all results together. This in general would have the least contention related overhead but this option would also require the maximum memory footprint. So for large datasets, this may be an issue.

The approach taken in this program is a compromise between the above two extremes. Each indexing thread is given a random "budget" (say 20000 words). A thread will build the index into a local container. When it crosses its budget, it will still keep operating on its local container but will also periodically get a nonblocking (try_lock) mutex on the global shared container. If it is successful (i.e. the global container was not contended), it will then merge the local results,release the mutex and keep operating on the local container.

- WordCount

This class wraps some kind of a HashTable to store (string,unsigned) pairs representing word frequencies. There are two design issues with this class

    * Sometimes we will need a WordCount object that is only ever accessed by one thread (like the local containers used by the Indexing threads, or like when the program is being run in single threaded mode). At other times, we will need a WordCount object that is accessed by multiple threads. So to support both cases, this class requires the user to supply a LockingPolicy. Please see SimpleWordCount.h for intended usage
    * Finally, our task requires us to have a hashmap of <string,integer> pairs. As of `C++11` most major compilers support the short string optimization for `std::string` objects. So, using a `std::string` is a viable option since most of our words are going to be small words. Prior to `C++11`, we cold have instead used the non standard `__gnu_cxx::vstring` which is a sso optimized string. The next problem is the implementation of `std::unordered_map`. This uses a vector of buckets and each bucket is a linked list of `<key,value>` pairs. This implementation is widely believed to be inferior as compared with an open-addressing implementation which has better cache performance. In this program, we will still use `std::unordered_map` and have the flexibility to easily drop in a different hashtabe in the future.

- PriorityQueue

Finally, once we have the full index we then need to extract the top `N` words. This is achieved by feeding the whole index into a priority queue of bounded size `N`.

### Building

Works only on linux. Look at the `CMakeLists.txt` file. Should build on a modern linux box off the bat. Uses `boost` and the `googletest` unit testing framework.

### Testing

Writing *useful* unit tests for multithreaded code requires some infrastructure support. This toy program has the following approach:

    * Googletest based  unit tests for the individual components that do not exercise the threading aspects
    * Design the components so that they can be invoked synchronously as well as asynchronously. So now we can run the system in two modes and compare the results
    * Write a python script to build the index and compare the python results with the cpp results

### TODO

The priority queue is not needed and can be replaced with an online algorithm to find the most frequent `N` words in a stream of words







