#include <gtest/gtest.h>
#include <string>
#include "../src/Stream.hpp"

int ParseInt(const std::string& s) { return std::stoi(s); }
std::string StringifyInt(const int& v) { return std::to_string(v); }

TEST(FileStringStreamTest, RunIO) {
    // Тестирование строкового потока
    ReadOnlyStream<int> in_str("10 20 30", ParseInt, false);
    in_str.Open();
    EXPECT_EQ(in_str.Read(), 10); // <--- Исправили in_stream на in_str
    in_str.Close();

    // Тестирование файлового потока
    std::string filename = "gtest_io.txt";
    WriteOnlyStream<int> out_f(filename, StringifyInt, true);
    out_f.Open();
    out_f.Write(500);
    out_f.Close();

    ReadOnlyStream<int> in_f(filename, ParseInt, true);
    in_f.Open();
    EXPECT_EQ(in_f.Read(), 500);
    in_f.Close();

    std::remove(filename.c_str());
}
