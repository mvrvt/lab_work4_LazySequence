#pragma once

#include <stdexcept>
#include "lab2_files/DynamicArray.hpp"

// Forward declaration 
class LazySequence;

template <typename T>
class Generator {
public:
    // Определение типа "указатель на функцию", который принимает кэш и возвращает новый элемент
    typedef T ( *RuleFunc ) ( const DynamicArray<T>& );

    Generator( LazySequence<T>* owner, RuleFunc rule ) 
        : owner_( owner ), rule_( rule ) {
            if ( !rule )
                throw std::invalid_argument( "Generator: Rule function pointer can't be null" );
    }
    
    // Метод порождения очередного элемента
    T GetNext() {
        if ( !owner_ ) {
            throw std::runtime_error( "Generator: owner is null" ); 
        }
        // Вызов функции-правила, передавая ей уже вычисленные элементы (кэш хозяина)
        return rule_( owner_->GetCache() );
    }

    // Для бесконечных последовательностей всегда возвращаем true 
    bool HasNext() const {
        return true;
    }

private:
    LazySequence<T>* owner_;
    RuleFunc rule_;
};
