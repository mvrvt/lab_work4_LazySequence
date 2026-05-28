#pragma once

#include <functional>
#include <stdexcept>
#include <cstddef>
#include <algorithm>
#include "lab2_files/Sequence.hpp"
#include "lab2_files/ArraySequence.hpp"
#include "Generator.hpp"
#include "MyUtils.hpp"

// Вспомогательный класс-итератор для ленивой последовательности
template <typename T>
class LazyEnumerator : public IEnumerator<T> {
public:
    explicit LazyEnumerator( const Sequence<T>* seq ) : seq_( seq ), current_index_( -1 ) { }

    bool MoveNext() override {
        current_index_++;
        try {
            // Пытаемся получить элемент. Если вышли за пределы - возвращаем false
            seq_->Get( current_index_ );
            return true;
        } catch ( const IndexOutOfRange& ) {
            return false;
        }
    }

    T& Current() override {
        // Константный каст нужен, поскольку IEnumerator требует T&
        return const_cast<T&>( seq_->Get( current_index_ ) );
    }

    void Reset() override {
        current_index_ = -1;
    }

private:
    const Sequence<T>* seq_;
    int                current_index_;
};

// -------------------------------------------------------
template <typename T>
class LazySequence : public Sequence<T> {
public:
    // ==== Constructors etc.  ====

    LazySequence() : generator_( new EmptyGenerator<T>() ), is_exhausted_( true ) {
        cache_ = new MutableArraySequence<T>();
    }

    LazySequence( T* items, int count ) : generator_( new EmptyGenerator<T>() ), is_exhausted_( true ) {
        cache_ = new MutableArraySequence<T>( items, count );
    }

    explicit LazySequence( Sequence<T>* seq ) : generator_( new EmptyGenerator<T>() ), is_exhausted_( true ) {
        cache_ = seq->Clone();
    }

    // Задание (рекуррентого) правила через сырой указатель на функцию
    LazySequence( T ( *rule )( const Sequence<T>* ), Sequence<T>* initial_cache ) : is_exhausted_( false ) {
        cache_ = initial_cache ? initial_cache : new MutableArraySequence<T>();
        generator_ = new RuleGenerator<T>( rule, cache_ );
    }

    // Задание (рекуррентного) правила через std::function
    LazySequence( std::function<T( const Sequence<T>* )> rule, Sequence<T>* initial_cache ) : is_exhausted_( false ) {
        cache_ = initial_cache ? initial_cache : new MutableArraySequence<T>();
        generator_ = new RuleGenerator<T>( rule, cache_ );
    }

    LazySequence( const LazySequence<T>& other ) : is_exhausted_( other.is_exhausted_ ) {
        // Глубокое копирование кэша и генератора через полиморфные вызовы
        cache_ = other.cache_->Clone();
        generator_ = other.generator_->Clone();
    }

    ~LazySequence() override {
        delete generator_;
        delete cache_;
    }

    // ==== Декомпозиция и ICollection (с IEnumerator) ====

    IEnumerator<T>* GetEnumerator() const override {
        return new LazyEnumerator<T>( this );
    }

    T& GetFirst() const override {
        return const_cast<T&>( Get(0) );
    }

    T& GetLast() const override {
        MaterializeAll();
        if (cache_->GetLength() == 0) throw IndexOutOfRange("LazySequence: sequence is empty");
        return cache_->Get(cache_->GetLength() - 1);
    }

    const T& Get( std::size_t index ) const override {
        int idx = static_cast<int>( index );
        if ( idx < 0 ) throw IndexOutOfRange( "LazySequence: index can't be negative" );
        
        MaterializeUpTo( idx );
        
        if ( idx >= cache_->GetLength() ) throw IndexOutOfRange( "LazySequence: index out of range (sequence ended)" );
        
        // Возвращаем из кэша
        return cache_->Get( idx );
    }

    T& Get( std::size_t index ) override {
        // Делегируем вызов константной версии и снимает const
        return const_cast<T&>( static_cast<const LazySequence<T>*>( this )->Get( index ) );
    }

    int GetLength() const override {
        MaterializeAll();
        return cache_->GetLength();
    }

    std::size_t GetCount() const override {
        return static_cast<std::size_t>( GetLength() );
    }

    // Получить кол-во материализованных эл-ов
    std::size_t GetMaterializedCount() const {
        return static_cast<std::size_t>( cache_->GetLength() );
    }

    LazySequence<T>* GetSubsequence( int startIndex, int endIndex ) const override {
        if ( startIndex < 0 || endIndex < startIndex ) throw std::out_of_range( "IndexOutOfRange" );
        MaterializeUpTo( endIndex );

        // Создаём новую конечную ленивую пос-ть из вычисленного куска
        Sequence<T>* sub_cache = cache_->GetSubsequence( startIndex, endIndex );
        return new LazySequence<T>( sub_cache );
    }

    // ==== Операции мутации (возвращают новые объекты ) ====

    Sequence<T>* CreateEmpty() const override {
        return new LazySequence<T>();
    }

    Sequence<T>* Append( const T& item ) override {
        MaterializeAll();
        Sequence<T>* new_cache = cache_->Append( item );
        return new LazySequence<T>( new_cache );
    }

    Sequence<T>* Prepend( const T& item ) override {
        MaterializeAll();
        Sequence<T>* new_cache = cache_->Prepend( item );
        return new LazySequence<T>( new_cache );
    }

    // Вставить эл-нт в заданную позицию 
    LazySequence<T>* InsertAt( const T& item, int index ) override {
        MaterializeAll();
        Sequence<T>* new_cache = cache_->InsertAt( item, index );
        return new LazySequence<T>( new_cache );
    }

    // Сцепляет 2 списка
    LazySequence<T>* Concat( Sequence<T>* other ) const override {
        MaterializeAll();
        Sequence<T>* new_cache = cache_->Concat( other );
        return new LazySequence<T>( new_cache );
    }

    // Сцепляет два списка (перегрузка специально для LazySequence) 
    LazySequence<T>* Concat( LazySequence<T>* list ) const {
        return Concat( static_cast<Sequence<T>*>( list ) );
    }

    // ==== Функциональные операции (Map(), Reduce(), Where(), Zip()) ====

    template <typename T2>
    LazySequence<T2>* Map( T2 ( *func )( T ) ) const {
        MaterializeAll();
        LazySequence<T2>* result = new LazySequence<T2>();
        for ( int i = 0; i < cache_->GetLength(); ++i ) 
            result = static_cast<LazySequence<T2>*>( result->Append( func( cache_->Get( i ) ) ) );
        return result; 
    }

    T Reduce( T ( * func )( T, T ), T initial ) const {
        MaterializeAll();
        T result = initial;
        for ( int i = 0; i < cache_->GetLength(); ++i )
            result = func( result, cache_->Get( i ) );
        return result;
    }

    LazySequence<T>* Where( bool ( *predicate )( T ) ) const {
        MaterializeAll();
        LazySequence<T>* result = new LazySequence<T>();
        for ( int i = 0; i < cache_->GetLength(); ++i ) {
            if ( predicate( cache_->Get( i ) ) ) 
                result = static_cast<LazySequence<T>*>( result->Append( cache_->Get( i ) ) );
        }
        return result;
    }

    template <typename T2>
    LazySequence<my_utils::Pair<T, T2>>* Zip( Sequence<T2>* seq ) const {
        MaterializeAll();
        LazySequence<my_utils::Pair<T, T2>>* result = new LazySequence<my_utils::Pair<T, T2>>();
        int min_len = std::min( cache_->GetLength(), seq->GetLength() );

        for ( int i = 0; i < min_len; ++i ) {
            my_utils::Pair<T, T2> pair( cache_->Get( i ), seq->Get( i ) );
            result = static_cast<LazySequence<my_utils::Pair<T, T2>>*>( result->Append( pair ) );
        }
        return result;
    }

protected:
    // Поля mutable, чтобы их можно было менять внутри const-методов 
    mutable Sequence<T>*   cache_;
    mutable IGenerator<T>* generator_;
    mutable bool           is_exhausted_;

private:
    // Приватный метод для вычисления элементов до нужного индекса
    void MaterializeUpTo( int target_index ) const {
        while ( !is_exhausted_ && cache_->GetLength() <= target_index ) {
            my_utils::Optional<T> next_val = generator_->TryGetNext();

            if ( next_val.HasValue() ) {
                Sequence<T>* next_cache = cache_->Append( next_val.Value() );
                if ( next_cache != cache_ ) {
                    delete cache_;
                    cache_ = next_cache;
                }
            } else {
                is_exhausted_ = true; 
            }
        }
    }

    // Приватный метод для вычисления всей последовательности (опасно для бесконечных)
    void MaterializeAll() const {
        while ( !is_exhausted_ && generator_->HasNext() ) {
            my_utils::Optional<T> next_val = generator_->TryGetNext();
            if ( next_val.HasValue() ) {
                Sequence<T>* next_cache = cache_->Append( next_val.Value() );
                if ( next_cache != cache_ ) {
                    delete cache_;
                    cache_ = next_cache;
                }
            } else {
                is_exhausted_ = true; 
            }
        }
    }
};

