#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <stdexcept>
#include "../src/Stream.hpp"

// === Функции сериализации ===
int ParseIntValid(const std::string& s) { return std::stoi(s); }
std::string StringifyIntValid(const int& v) { return std::to_string(v); }

// === Тесты string stream (ЧТЕНИЕ) ===
TEST(StringStreamTest, ReadStringTokens) {
    ReadOnlyStream<int> stream("10 20 30", ParseIntValid, false);
    stream.Open();
    EXPECT_EQ(stream.Read(), 10);
    EXPECT_EQ(stream.Read(), 20);
    EXPECT_EQ(stream.Read(), 30);
    EXPECT_TRUE(stream.IsEndOfStream());
}

TEST(StringStreamTest, ReadPastEOFThrows) {
    ReadOnlyStream<int> stream("42", ParseIntValid, false);
    stream.Open();
    stream.Read();
    EXPECT_THROW(stream.Read(), EndOfStream);
}

TEST(StringStreamTest, StringStreamCannotSeek) {
    ReadOnlyStream<int> stream("10 20", ParseIntValid, false);
    stream.Open();
    EXPECT_FALSE(stream.CanSeek());
    EXPECT_THROW(stream.Seek(1), std::logic_error);
}

TEST(StringStreamTest, EmptyStringStreamIsEOFImmediately) {
    ReadOnlyStream<int> stream("", ParseIntValid, false);
    stream.Open();
    EXPECT_TRUE(stream.IsEndOfStream());
    EXPECT_THROW(stream.Read(), EndOfStream);
}

// === Тесты string stream (ЗАПИСЬ) ===
TEST(StringStreamTest, WriteStringTokens) {
    WriteOnlyStream<int> stream("", StringifyIntValid, false);
    stream.Open();
    stream.Write(10);
    stream.Write(20);
    stream.Close(); 
    EXPECT_EQ(stream.GetString(), "10 20 ");
}

TEST(StringStreamTest, GetStringFromClosedWithoutOpen) {
    WriteOnlyStream<int> stream("", StringifyIntValid, false);
    EXPECT_EQ(stream.GetString(), "");
}

TEST(StringStreamTest, GetStringOnFileStreamThrows) {
    WriteOnlyStream<int> stream("test.txt", StringifyIntValid, true);
    EXPECT_THROW(stream.GetString(), std::logic_error);
}

// === Тесты file stream (ЧТЕНИЕ) ===
TEST(FileStreamTest, OpenNonExistentFileThrows) {
    ReadOnlyStream<int> stream("non_existent_file_12345.txt", ParseIntValid, true);
    EXPECT_THROW(stream.Open(), std::logic_error);
}

TEST(FileStreamTest, ReadFileTokens) {
    const std::string filename = "gtest_read_test.txt";
    std::ofstream out(filename);
    out << "100 200 300";
    out.close();

    ReadOnlyStream<int> stream(filename, ParseIntValid, true);
    stream.Open();
    EXPECT_EQ(stream.Read(), 100);
    EXPECT_EQ(stream.Read(), 200);
    EXPECT_EQ(stream.Read(), 300);
    EXPECT_TRUE(stream.IsEndOfStream());
    stream.Close();

    std::remove(filename.c_str());
}

TEST(FileStreamTest, FileStreamCannotSeek) {
    const std::string filename = "gtest_seek_test.txt";
    std::ofstream out(filename); out << "10"; out.close();

    ReadOnlyStream<int> stream(filename, ParseIntValid, true);
    stream.Open();
    EXPECT_FALSE(stream.CanSeek());
    EXPECT_THROW(stream.Seek(1), std::logic_error);
    stream.Close();

    std::remove(filename.c_str());
}

// === Тесты file stream (ЗАПИСЬ) ===
TEST(FileStreamTest, WriteFileTokens) {
    const std::string filename = "gtest_write_test.txt";
    WriteOnlyStream<int> stream(filename, StringifyIntValid, true);
    stream.Open();
    stream.Write(555);
    stream.Write(777);
    stream.Close();

    std::ifstream in(filename);
    int a, b;
    in >> a >> b;
    EXPECT_EQ(a, 555);
    EXPECT_EQ(b, 777);
    in.close();

    std::remove(filename.c_str());
}

TEST(FileStreamTest, WriteToClosedFileThrows) {
    WriteOnlyStream<int> stream("test.txt", StringifyIntValid, true);
    EXPECT_THROW(stream.Write(10), std::logic_error);
}

TEST(FileStreamTest, PositionIncrementsOnFileWrite) {
    const std::string filename = "gtest_pos_test.txt";
    WriteOnlyStream<int> stream(filename, StringifyIntValid, true);
    stream.Open();
    EXPECT_EQ(stream.GetPosition(), 0);
    stream.Write(10);
    EXPECT_EQ(stream.GetPosition(), 1);
    stream.Close();
    
    std::remove(filename.c_str());
}

TEST(FileStreamTest, OpenAndCloseMultipleTimes) {
    const std::string filename = "gtest_multi_test.txt";
    WriteOnlyStream<int> stream(filename, StringifyIntValid, true);
    stream.Open();
    stream.Write(1);
    stream.Close();
    
    // Перезапись файла
    stream.Open();
    stream.Write(2);
    stream.Close();

    std::ifstream in(filename);
    int a;
    in >> a;
    EXPECT_EQ(a, 2);
    in.close();
    
    std::remove(filename.c_str());
}
