#include <gtest/gtest.h>
#include "../src/Stream.hpp"
#include "../src/LazySequence.hpp"

using namespace my_utils;

int NumRule(int index) { return index + 1; }

TEST(StreamTest, PipelineSequence) {
    LazySequence<int> seq(NumRule, Ordinal(1, 0), 5);
    ReadOnlyStream<int> stream(&seq);

    stream.Open();
    EXPECT_EQ(stream.Read(), 1);
    EXPECT_EQ(stream.Read(), 2);
    
    stream.Seek(10);
    EXPECT_EQ(stream.Read(), 11);
    stream.Close();
}
