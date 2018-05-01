#pragma once
#include "MMapWorkItem.h"
#include "BlockingQueue.h"

namespace SimpleIndexer {

    using IndexingWorkItem = MMapWorkItem;    
    using IndexingWorkQueue = BlockingQueue<IndexingWorkItem>;
    
}
