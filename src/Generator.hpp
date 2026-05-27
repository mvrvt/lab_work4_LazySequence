#pragma once

#include <stdexcept>
#include "MyUtils.hpp"

// Интерфейс для всех генераторов 
template <typename T>
class IGenerator {
public:
    virtual ~IGenerator() = default; 

    // Возвращает Optional: если есть значение - возвращает его, если конец - пустой Optional
    virtual my_utils::Optional<T> TryGetNext() = 0;

    virtual bool HasNext() const = 0;
};

// Базовый генератор на основе правила (функции)
template <typename T>
class RuleGenerator : public IGenerator<T> {
public:
    // Функция не требует знания о кэше самого списка, 
    // она просто вычисляет след. значение на основе своего состояния
    typedef my_utils::Optional<T> ( *RuleFunc )();

    explicit RuleGenerator( RuleFunc rule ) : rule_( rule ) {
        if ( !rule_ ) 
            throw std::invalid_argument( "RuleGenerator: rule function can't be null" );
    }

    my_utils::Optional<T> TryGetNext() override {
        return rule_();
    }

    bool HasNext() const override {
        return true; // пока считаем бесконечным - true
    }

private:
    RuleFunc rule_;
};
