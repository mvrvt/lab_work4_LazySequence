#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/Stream.hpp"
#include "../src/LazySequence.hpp"
#include "../src/lab2_files/ArraySequence.hpp"

using namespace my_utils;

int StreamNumRule(int idx) { return idx + 1; }

// === Тесты ReadOnlyStream ===
TEST(ReadOnlyStreamTest, OpenCloseState) {
    LazySequence<int> seq(StreamNumRule, Ordinal(0, 5));
    ReadOnlyStream<int> stream(&seq);
    
    EXPECT_THROW(stream.Read(), std::logic_error); // До открытия
    stream.Open();
    EXPECT_EQ(stream.GetPosition(), 0);
    stream.Close();
    EXPECT_THROW(stream.Read(), std::logic_error); // После закрытия
}

TEST(ReadOnlyStreamTest, ReadSequentialValues) {
    LazySequence<int> seq(StreamNumRule, Ordinal(0, 5));
    ReadOnlyStream<int> stream(&seq);
    stream.Open();
    
    EXPECT_EQ(stream.Read(), 1);
    EXPECT_EQ(stream.Read(), 2);
    EXPECT_EQ(stream.GetPosition(), 2);
}

TEST(ReadOnlyStreamTest, ReadPastEndOfFiniteStreamThrows) {
    LazySequence<int> seq(StreamNumRule, Ordinal(0, 2));
    ReadOnlyStream<int> stream(&seq);
    stream.Open();
    
    stream.Read();
    stream.Read();
    EXPECT_TRUE(stream.IsEndOfStream());
    EXPECT_THROW(stream.Read(), EndOfStream);
}

TEST(ReadOnlyStreamTest, InfiniteStreamNeverEnds) {
    LazySequence<int> seq(StreamNumRule, Ordinal(1, 0));
    ReadOnlyStream<int> stream(&seq);
    stream.Open();
    
    EXPECT_FALSE(stream.IsEndOfStream());
    stream.Seek(1000000);
    EXPECT_FALSE(stream.IsEndOfStream());
}

TEST(ReadOnlyStreamTest, SeekWithinFiniteStream) {
    LazySequence<int> seq(StreamNumRule, Ordinal(0, 10));
    ReadOnlyStream<int> stream(&seq);
    stream.Open();
    
    stream.Seek(5);
    EXPECT_EQ(stream.GetPosition(), 5);
    EXPECT_EQ(stream.Read(), 6);
}

TEST(ReadOnlyStreamTest, SeekOutOfBoundsFiniteStreamThrows) {
    LazySequence<int> seq(StreamNumRule, Ordinal(0, 5));
    ReadOnlyStream<int> stream(&seq);
    stream.Open();
    
    EXPECT_THROW(stream.Seek(10), std::out_of_range);
}

TEST(ReadOnlyStreamTest, SeekOnClosedStreamThrows) {
    LazySequence<int> seq(StreamNumRule, Ordinal(0, 5));
    ReadOnlyStream<int> stream(&seq);
    EXPECT_THROW(stream.Seek(2), std::logic_error);
}

TEST(ReadOnlyStreamTest, CanSeekAndCanGoBack) {
    LazySequence<int> seq(StreamNumRule, Ordinal(0, 5));
    ReadOnlyStream<int> stream(&seq);
    stream.Open();
    EXPECT_TRUE(stream.CanSeek());
    EXPECT_TRUE(stream.CanGoBack());
}

// === Тесты WriteOnlyStream ===
TEST(WriteOnlyStreamTest, WriteAppendsToSequence) {
    MutableArraySequence<int> seq;
    WriteOnlyStream<int> stream(&seq);
    stream.Open();
    
    stream.Write(10);
    stream.Write(20);
    
    EXPECT_EQ(stream.GetPosition(), 2);
    EXPECT_EQ(seq.Get(0), 10);
    EXPECT_EQ(seq.Get(1), 20);
}

TEST(WriteOnlyStreamTest, WriteToClosedStreamThrows) {
    MutableArraySequence<int> seq;
    WriteOnlyStream<int> stream(&seq);
    EXPECT_THROW(stream.Write(10), std::logic_error);
}

TEST(WriteOnlyStreamTest, OpenWithoutDestinationThrows) {
    WriteOnlyStream<int> stream((Sequence<int>*)nullptr);
    EXPECT_THROW(stream.Open(), std::logic_error);
}

TEST(WriteOnlyStreamTest, WriteIncrementsPositionCorrectly) {
    MutableArraySequence<int> seq;
    seq.Append(1); // Исходная длина 1
    WriteOnlyStream<int> stream(&seq);
    
    EXPECT_EQ(stream.GetPosition(), 1); // Должен подхватить длину
    stream.Open();
    stream.Write(2);
    EXPECT_EQ(stream.GetPosition(), 2);
}
