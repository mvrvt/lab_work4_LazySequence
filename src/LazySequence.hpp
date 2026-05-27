#pragma once

#include "lab2_files/DynamicArray.hpp"
#include "Generator.hpp"
#include "MyUtils.hpp"
#include <stdexcept>

template <typename T>
class LazySequence {
public:
    // Определение типа "указатель на функцию", который принимает кэш и возвращает новый элемент(как в генераторе)
    typedef T ( *RuleFunc )( const DynamicArray<T>& );

    LazySequence( RuleFunc rule, const DynamicArray<T>& initial_items )
        : cache_( initial_items ), generator_( nullptr ) {
            // Создаём генератор и передаём ему указатель на самого себя
            generator_ = new Generator( this, rule );
    }

    ~LazySequence() {
        delete generator_;
    }

    // Получение эл-та по индексу с ленивым вычислением
    T Get( int index ) {
        if ( index < 0 )
            throw IndexOutOfRange( "LazySequence: index can't be negative" );

        // Если элемента еще нет в кэше, нужно его вычислить
        int current_count = static_cast<int>( cache_.GetCount() );
        while ( index >= current_count ) {
            // Просим генератор создать следующий эл-нт
            T next_element = generator_->GetNext();

            // метода Append() нет в DynArr, поэтому увеличиваем размер и записываем
            cache_.Resize( current_count + 1 );
            cache_.Set( current_count, nex_element );

            current_count++;
        }
        return cache_.Get( index );
    }

    // Позволяем генератору читать кэш (но не изменять его)
    const DynamicArray<T>& GetCache() const {
        return cache_;
    }

    // Получить количество уже материализованных (вычисленных) элементов
    std::size_t GetMaterializedCount() const {
        return cache_.GetCount();
    }

private:
    DynamicArray<T> cache_;
    Generator<T>*   generator_;
};
