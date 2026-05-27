#pragma once

namespace my_utils {

// Замена std::pair
template <typename T1, typename T2>
struct Pair {
public:
    T1 first;
    T2 second;

    Pair() : first( T1() ), second( T2() ) { }
    Pair( const T1& f , const T2& s ) : first( f ), second ( s ) { }  
};

// Замена std::optional (для безопасного возврата значений)
template <typename T>
class Optional {
public:
    Optional() : has_value_( false ) { }
    explicit Optional( const T& val ) : value_( val ), has_value_( true ) { }

    bool HasValue() const { return has_value_; }

    const T& Value() const {
        if ( !has_value_ )
            throw std::runtime_error( "Optional object contains no value" );
        return value_;
    }

private:
    T    value_;
    bool has_value_;
};

}
