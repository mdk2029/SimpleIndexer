#pragma once
#include <iostream>
#include <algorithm>

namespace SimpleIndexer {
    
template<typename Iterator, typename OutputContainer>
inline void extractWords(Iterator begin, Iterator end, OutputContainer& cont) {
    auto curr_start = begin;

    while (curr_start != end) {

        auto itr1 = std::find_if(curr_start, end, [](char ch) { return std::isalnum(ch); });
        auto itr2 = end;

        if (itr1 != end) {
            itr2 = std::find_if(itr1, end, [](char ch) { return !std::isalnum(ch); });
            //[itr1,itr2) now denotes a valid word (without a trailing \0), even if itr2 == end
            cont.insert(itr1, itr2);
        }

        //Start over with itr2;
        curr_start = itr2;
    }
}

}
