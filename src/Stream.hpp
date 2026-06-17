#pragma once
#include <stdexcept>
#include <string>
#include <cstddef>
#include <fstream>
#include <sstream>
#include "lab2_files/Sequence.hpp"
#include "LazySequence.hpp"

class EndOfStream : public std::runtime_error {
public:
    explicit EndOfStream(const std::string& message) : std::runtime_error(message) {}
};

template <typename T>
class ReadOnlyStream {
public:
    typedef T (*DeserializerFunc)(const std::string&);
    enum StreamType { SEQ, FILE_STREAM, STRING_STREAM };

    explicit ReadOnlyStream(const Sequence<T>* sequence)
        : stream_type_(SEQ), source_sequence_(sequence), current_position_(0), is_open_(false), 
          deserializer_(nullptr), file_stream_(nullptr), string_stream_(nullptr) {}

    ReadOnlyStream(const std::string& source, DeserializerFunc deserializer, bool is_file)
        : source_sequence_(nullptr), current_position_(0), is_open_(false), 
          deserializer_(deserializer), file_stream_(nullptr), string_stream_(nullptr) {
        if (is_file) {
            stream_type_ = FILE_STREAM;
            filename_ = source;
        } else {
            stream_type_ = STRING_STREAM;
            raw_string_data_ = source;
        }
    }

    ~ReadOnlyStream() { Close(); }

    void Open() {
        // Вернули проверку на nullptr для последовательностей
        if (stream_type_ == SEQ && !source_sequence_) {
            throw std::logic_error("ReadOnlyStream: no data source attached");
        } else if (stream_type_ == FILE_STREAM) {
            file_stream_ = new std::ifstream(filename_);
            if (!file_stream_->is_open()) throw std::logic_error("ReadOnlyStream: cannot open file");
        } else if (stream_type_ == STRING_STREAM) {
            string_stream_ = new std::istringstream(raw_string_data_);
        }
        is_open_ = true; 
        current_position_ = 0;
    }

    void Close() {
        if (file_stream_) { file_stream_->close(); delete file_stream_; file_stream_ = nullptr; }
        if (string_stream_) { delete string_stream_; string_stream_ = nullptr; }
        is_open_ = false;
    }

    bool IsEndOfStream() const {
        if (!is_open_) throw std::logic_error("ReadOnlyStream: stream is closed");
        if (stream_type_ == SEQ) {
            const LazySequence<T>* lazy_seq = dynamic_cast<const LazySequence<T>*>(source_sequence_);
            if (lazy_seq != nullptr && lazy_seq->GetOrdinalCardinality().IsInfinite()) return false; 
            return current_position_ >= static_cast<size_t>(source_sequence_->GetLength());
        } else if (stream_type_ == FILE_STREAM) {
            // ИСПРАВЛЕНО: Добавлен peek() для мгновенного детектирования пустых файлов
            return file_stream_->eof() || file_stream_->peek() == EOF;
        } else {
            // ИСПРАВЛЕНО: Добавлен peek() для мгновенного детектирования пустых строк
            return string_stream_->eof() || string_stream_->peek() == EOF;
        }
    }

    T Read() {
        if (!is_open_) throw std::logic_error("ReadOnlyStream: stream is closed");
        if (IsEndOfStream()) throw EndOfStream("ReadOnlyStream: reached the end");

        T item;
        if (stream_type_ == SEQ) {
            item = source_sequence_->Get(current_position_);
        } else {
            std::string token;
            if (stream_type_ == FILE_STREAM) *file_stream_ >> token;
            else *string_stream_ >> token;
            if (token.empty() && IsEndOfStream()) throw EndOfStream("Stream ended");
            item = deserializer_(token);
        }
        current_position_++;
        return item;
    }

    std::size_t GetPosition() const { return current_position_; }
    bool CanSeek() const { return stream_type_ == SEQ; } 
    bool CanGoBack() const { return stream_type_ == SEQ; }

    std::size_t Seek(std::size_t index) {
        if (!is_open_) throw std::logic_error("ReadOnlyStream: stream is closed");
        if (!CanSeek()) throw std::logic_error("ReadOnlyStream: stream doesn't support seek");

        bool is_infinite = false;
        const LazySequence<T>* lazy_seq = dynamic_cast<const LazySequence<T>*>(source_sequence_);
        if (lazy_seq != nullptr && lazy_seq->GetOrdinalCardinality().IsInfinite()) is_infinite = true;

        if (!is_infinite && index > static_cast<size_t>(source_sequence_->GetLength())) {
            throw std::out_of_range("ReadOnlyStream: seek index out of bounds");
        }
        current_position_ = index;
        return current_position_;
    }

private:
    StreamType stream_type_;
    const Sequence<T>* source_sequence_;
    DeserializerFunc deserializer_;
    std::string filename_;
    std::string raw_string_data_;
    std::ifstream* file_stream_;
    std::istringstream* string_stream_;
    std::size_t current_position_;
    bool is_open_;
};

template <typename T>
class WriteOnlyStream {
public:
    typedef std::string (*SerializerFunc)(const T&);
    enum StreamType { SEQ, FILE_STREAM, STRING_STREAM };

    explicit WriteOnlyStream(Sequence<T>* sequence)
        : stream_type_(SEQ), destination_sequence_(sequence), current_position_(0), is_open_(false), 
          serializer_(nullptr), file_stream_(nullptr), string_stream_(nullptr) {
        if (destination_sequence_) current_position_ = static_cast<size_t>(destination_sequence_->GetLength());
    }

    WriteOnlyStream(const std::string& source, SerializerFunc serializer, bool is_file)
        : destination_sequence_(nullptr), current_position_(0), is_open_(false), 
          serializer_(serializer), file_stream_(nullptr), string_stream_(nullptr) {
        if (is_file) { stream_type_ = FILE_STREAM; filename_ = source; }
        else stream_type_ = STRING_STREAM;
    }

    ~WriteOnlyStream() { Close(); }

    void Open() {
        if (stream_type_ == SEQ && !destination_sequence_) {
            throw std::logic_error("WriteOnlyStream: no data destination attached");
        } else if (stream_type_ == FILE_STREAM) {
            file_stream_ = new std::ofstream(filename_);
            if (!file_stream_->is_open()) throw std::logic_error("WriteOnlyStream: cannot open file");
        } else if (stream_type_ == STRING_STREAM) {
            string_stream_ = new std::ostringstream();
        }
        is_open_ = true; 
    }

    void Close() {
        if (file_stream_) { file_stream_->close(); delete file_stream_; file_stream_ = nullptr; }
        if (string_stream_) { raw_string_data_ = string_stream_->str(); delete string_stream_; string_stream_ = nullptr; }
        is_open_ = false;
    }

    std::size_t GetPosition() const { return current_position_; }

    std::size_t Write(const T& item) {
        if (!is_open_) throw std::logic_error("WriteOnlyStream: stream is closed");
        if (stream_type_ == SEQ) {
            Sequence<T>* new_seq = destination_sequence_->Append(item);
            if (new_seq != destination_sequence_) destination_sequence_ = new_seq; 
        } else if (stream_type_ == FILE_STREAM) {
            *file_stream_ << serializer_(item) << " ";
        } else {
            *string_stream_ << serializer_(item) << " ";
        }
        current_position_++;
        return current_position_;
    }

    std::string GetString() const {
        if (stream_type_ != STRING_STREAM) throw std::logic_error("Not a string stream");
        return raw_string_data_;
    }

private:
    StreamType stream_type_;
    Sequence<T>* destination_sequence_;
    SerializerFunc serializer_;
    std::string filename_;
    mutable std::string raw_string_data_; 
    std::ofstream* file_stream_;
    std::ostringstream* string_stream_;
    std::size_t current_position_;
    bool is_open_;
};
