#pragma once
#include <stdexcept>

namespace my_utils {

template <typename T1, typename T2>
struct Pair {
    T1 first;
    T2 second;
    Pair() : first(T1()), second(T2()) { }
    Pair(const T1& f, const T2& s) : first(f), second(s) { }  
};

template <typename T>
class Optional {
public:
    Optional() : has_value_(false) { }
    explicit Optional(const T& val) : value_(val), has_value_(true) { }
    bool HasValue() const { return has_value_; }
    const T& Value() const {
        if (!has_value_) throw std::runtime_error("Optional object contains no value");
        return value_;
    }
private:
    T    value_;
    bool has_value_;
};

// Структура ординального числа: omega0 * W + finite
struct Ordinal {
    int omega0; // множитель перед бесконечностью
    int finite; // конечный остаток

    Ordinal(int w = 0, int f = 0) : omega0(w), finite(f) { }

    bool IsInfinite() const { return omega0 > 0; }

    // Сравнение ординалов
    bool operator<(const Ordinal& other) const {
        if (omega0 != other.omega0) return omega0 < other.omega0;
        return finite < other.finite;
    }
    bool operator>=(const Ordinal& other) const { return !(*this < other); }
    bool operator==(const Ordinal& other) const { return omega0 == other.omega0 && finite == other.finite; }

    // Сложение ординалов: (w0*A + B) + (w0*C + D)
    Ordinal operator+(const Ordinal& other) const {
        if (other.omega0 > 0) {
            return Ordinal(omega0 + other.omega0, other.finite); // Конечная часть B поглощается
        }
        return Ordinal(omega0, finite + other.finite);
    }

    // Вычитание ординалов (упрощенное для наших задач индексации)
    Ordinal operator-(const Ordinal& other) const {
        if (omega0 == other.omega0) {
            return Ordinal(0, finite - other.finite);
        }
        return Ordinal(omega0 - other.omega0, finite);
    }
};

}
