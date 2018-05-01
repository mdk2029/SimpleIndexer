#include <boost/algorithm/string.hpp>
#include <iostream>
#include <regex>
#include "../DirectoryTraverser.h"
#include "../SimpleWordCount.h"
#include "../ExtractWords.h"

#if __cplusplus >= 201103L
#include <string>
#else
#include <ext/vstring.h>
#endif

#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"

using namespace std;

namespace SimpleIndexer {

GTEST_TEST(SimpleIndexer,FileregexTest) {

    regex exp("(.|\n)*\\.txt");
    {
        string path = R"(/tmp/file.txt)";
        EXPECT_TRUE(regex_match(path,exp));
    }
    {
        string path = R"(/tmp/file.txt1)";
        EXPECT_FALSE(regex_match(path,exp));
    }
    {
        string path = R"(/tmp/filetxt)";
        EXPECT_FALSE(regex_match(path,exp));
    }

}

GTEST_TEST(SimpleIndexer,SimpleWordCountTest) {
    using namespace SimpleIndexer;
    SimpleWordCount<NopLockingPolicy> wordCount;
    auto vec = {string{"This"},string{"is"}, string{"a"}, string{"Is"}, string{"IS"}, string{"tHIS"}, string{"test"}, string{"t23est"}, string{"T23est"}};
    decltype(wordCount.wordMap_) eWordCount{{"THIS",2},{"IS",3},{"A",1},{"TEST",1},{"T23EST",2}};

    for(const auto& str : vec) {
        wordCount.insert(str.cbegin(),str.cend());
    }

    EXPECT_EQ(wordCount.wordMap_,eWordCount);
}

GTEST_TEST(SimpleIndexer,SimpleWordCountMergeTest) {
    SimpleWordCount<NopLockingPolicy> wordCount1;
    auto vec1 = {string{"This"},string{"is"}, string{"a"}, string{"Is"}, string{"IS"}, string{"tHIS"}, string{"test"}, string{"t23est"}, string{"T23est"}, string{"ORANGE"}};

    for(const auto& str : vec1) {
        wordCount1.insert(str.cbegin(),str.cend());
    }

    SimpleWordCount<NopLockingPolicy> wordCount2;
    auto vec2 = {string{"his"},string{"tis"}, string{"a"}, string{"Is"}, string{"IS"}, string{"pht"}, string{"tHIS"}, string{"test"}, string{"t23est"}};

    for(const auto& str : vec2) {
        wordCount2.insert(str.cbegin(),str.cend());
    }

    wordCount1 += wordCount2;
    decltype(wordCount1.wordMap_) eWordCount{{"THIS",3},{"IS",5},{"A",2},{"TEST",2},{"T23EST",3},{"HIS",1},{"TIS",1},{"PHT",1},{"ORANGE",1}};
    EXPECT_EQ(wordCount1.wordMap_, eWordCount);
}

namespace {
struct StringContainer {
    std::vector<std::string> val_;
    template<typename InputIterator>
    void insert(InputIterator beg, InputIterator end){
        val_.push_back(std::string(beg,end));
    }
};
}

GTEST_TEST(SimpleIndexer,ExtractWordsTest) {
    StringContainer cont;
    std::string input = R"(This787fa****!a    this 787 &^kjdjnot)";
    SimpleIndexer::extractWords(std::begin(input),std::end(input),cont);

    vector<std::string> expectedVec{string{"This787fa"},string{"a"},string{"this"},string{"787"},string{"kjdjnot"}};
    EXPECT_EQ(cont.val_,expectedVec);
}

}

