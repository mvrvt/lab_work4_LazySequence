#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/Stream.hpp"
#include "../src/LazySequence.hpp"
#include "../src/lab2_files/ArraySequence.hpp"
#include "../src/MyUtils.hpp"

using namespace my_utils;

// Обычная функция-правило для тестов
int StreamNumberRule(int index) {
    return index + 1; // 1, 2, 3, 4...
}

LazySequence<int>* CreateStreamSequence() {
    // Создаем бесконечную последовательность w0*1
    return new LazySequence<int>(StreamNumberRule, Ordinal(1, 0), 10);
}

// ==========================================
// ТЕСТЫ READ-ONLY STREAM
// ==========================================

TEST(ReadOnlyStreamTest, OpenClose) {
    auto* seq = CreateStreamSequence();
    ReadOnlyStream<int> stream(seq);

    // До открытия читать нельзя (бросит исключение, но мы проверяем просто статус, если добавим метод IsOpen - хорошо)
    stream.Open();
    EXPECT_EQ(stream.GetPosition(), 0);
    
    stream.Close();
    delete seq;
}

TEST(ReadOnlyStreamTest, ReadValues) {
    auto* seq = CreateStreamSequence();
    ReadOnlyStream<int> stream(seq);

    stream.Open();
    EXPECT_EQ(stream.Read(), 1);
    EXPECT_EQ(stream.Read(), 2);
    EXPECT_EQ(stream.Read(), 3);

    delete seq;
}

TEST(ReadOnlyStreamTest, PositionAfterRead) {
    auto* seq = CreateStreamSequence();
    ReadOnlyStream<int> stream(seq);

    stream.Open();
    stream.Read();
    stream.Read();
    stream.Read();

    EXPECT_EQ(stream.GetPosition(), 3);

    delete seq;
}

TEST(ReadOnlyStreamTest, SeekAndCanGoBack) {
    auto* seq = CreateStreamSequence();
    ReadOnlyStream<int> stream(seq);

    stream.Open();
    EXPECT_TRUE(stream.CanSeek());
    EXPECT_TRUE(stream.CanGoBack());

    stream.Seek(10);
    EXPECT_EQ(stream.GetPosition(), 10);
    EXPECT_EQ(stream.Read(), 11); // Индекс 10 -> Значение 11

    delete seq;
}

TEST(ReadOnlyStreamTest, InfiniteSequenceNeverEnds) {
    auto* seq = CreateStreamSequence();
    ReadOnlyStream<int> stream(seq);

    stream.Open();
    EXPECT_FALSE(stream.IsEndOfStream());
    
    stream.Seek(1000);
    EXPECT_FALSE(stream.IsEndOfStream());

    delete seq;
}

TEST(ReadOnlyStreamTest, ReadClosedStreamThrows) {
    auto* seq = CreateStreamSequence();
    ReadOnlyStream<int> stream(seq);

    EXPECT_THROW(stream.Read(), std::logic_error);

    delete seq;
}

// ==========================================
// ТЕСТЫ WRITE-ONLY STREAM
// ==========================================

TEST(WriteOnlyStreamTest, WriteSingleAndMultipleValues) {
    auto* seq = new MutableArraySequence<int>();
    WriteOnlyStream<int> stream(seq);

    stream.Open();
    stream.Write(10);
    stream.Write(20);
    stream.Write(30);

    // GetPosition() возвращает позицию для следующей записи (то есть длину)
    EXPECT_EQ(stream.GetPosition(), 3);
    EXPECT_EQ(seq->Get(0), 10);
    EXPECT_EQ(seq->Get(2), 30);

    stream.Close();
    delete seq;
}

TEST(WriteOnlyStreamTest, WriteClosedStreamThrows) {
    auto* seq = new MutableArraySequence<int>();
    WriteOnlyStream<int> stream(seq);

    EXPECT_THROW(stream.Write(10), std::logic_error);

    delete seq;
}
