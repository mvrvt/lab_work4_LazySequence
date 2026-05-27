#pragma once

#include <functional>
#include <stdexcept>
#include "MyUtils.hpp"
#include "lab2_files/Sequence.hpp"

// Полиморфный интерфейс генератора
template <typename T>
class IGenerator {
public:
    virtual ~IGenerator() = default; 

    virtual T GetNext() = 0;

    virtual bool HasNext() const = 0;

    virtual IGenerator<T>* Clone() const = 0;

    // Возвращает Optional: если есть значение - возвращает его, если конец - пустой Optional
    virtual my_utils::Optional<T> TryGetNext() = 0;
};

// Базовый генератор на основе правила (функции)
template <typename T>
class RuleGenerator : public IGenerator<T> {
public:
    // Функция не требует знания о кэше самого списка, 
    // она просто вычисляет след. значение на основе своего состояния
    typedef std::function<T( const Sequence<T>* )> RuleFunc;

    RuleGenerator( RuleFunc rule, const Sequence<T>* cache_ref ) : rule_( rule ), cache_ref_( cache_ref ) {
        if ( !rule_ )
            throw std::out_of_range( "RuleGenerator: rule function can't be null" );
    }
    
    T GetNext() override {
        if ( !HasNext() ) 
            throw std::out_of_range( "RuleGenerator: reached end of sequence" );

        return rule_( cache_ref_ );
    }

    my_utils::Optional<T> TryGetNext() override {
        if ( HasNext() ) {
            return my_utils::Optional<T>( GetNext() );
        }
        return my_utils::Optional<T>();
    }

    bool HasNext() const override {
        return true; // пока считаем бесконечным - true
    }

    IGenerator<T>* Clone() const override {
        return new RuleGenerator<T>( rule_, cache_ref_ );
    }

private:
    RuleFunc           rule_;
    const Sequence<T>* cache_ref_;
};

// Генератор-заглушка для конечных/пустых последовательностей 
template <typename T>
class EmptyGenerator : public IGenerator<T> {
public:
    T GetNext() override { throw std::out_of_range( "EmptyGenerator: no elements" ); }
    my_utils::Optional<T> TryGetNext() override { return my_utils::Optional<T>(); }
    bool HasNext()    const override { return false; }
    IGenerator<T>* Clone() const override {
        return new EmptyGenerator<T>();
    }
};
