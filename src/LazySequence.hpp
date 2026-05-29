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

    const T& Current() const override {
        // Константный каст нужен, поскольку IEnumerator требует T&
        return seq_->Get( current_index_ );
    }

    void Reset() override {
        current_index_ = -1;
    }

private:
    const Sequence<T>* seq_;
    int                current_index_;
};

// ------------------------------------------------------------------

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
        if ( generator_->GetCardinality().IsInfinite() )
            throw std::logic_error( "LazySequence: can't get the last element of an infinite sequence" );
        MaterializeAll();
        if ( cache_->GetLength() == 0 ) throw IndexOutOfRange( "LazySequence: sequence is empty" );
        return cache_->Get( cache_->GetLength() - 1 );
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
        if ( generator_->GetCardinality().IsInfinite() ) 
            throw std::logic_error( "LazySequence: can't get integer length of an infinite sequence" );
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

    my_utils::Cardinality GetCardinality() const {
        return generator_->GetCardinality();
    }

    LazySequence<T>* GetSubsequence( int startIndex, int endIndex ) const override {
        if ( startIndex < 0 || endIndex < startIndex ) throw std::out_of_range( "IndexOutOfRange" );
        MaterializeUpTo( endIndex );

        // Создаём новую конечную ленивую пос-ть из вычисленного куска
        Sequence<T>* sub_cache = cache_->GetSubsequence( startIndex, endIndex );
        return new LazySequence<T>( sub_cache );
    }

    Sequence<T>* Clone() const override {
        return new LazySequence<T>( *this );
    }

    // ==== Операции мутации (возвращают новые объекты) ====

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

    LazySequence<T>* Concat( LazySequence<T>* other ) const {
        if ( !other ) throw std::invalid_argument( "LazySequence: other is null" );

        // создаём ленивый генератор конкатенации
        IGenerator<T>* concat_gen = new ConcatGenerator<T>( this->generator_, other->generator_ );

        LazySequence<T>* result = new LazySequence<T>();
        delete result->generator_; // удаляем дефолтный генератор
        result->generator_ = concat_gen;
        result->is_exhausted_ = false; 

        return result; 
    }

    Sequence<T>* Concat( Sequence<T>* other ) const override {
        if ( !other ) throw std::invalid_argument( "LazySequence: other is null" );

        // Попытка превратить other в LazySequence
        LazySequence<T>* lazy_other = dynamic_cast<LazySequence<T>*>( other );
        if ( lazy_other ) {
            return Concat( lazy_other );
        }

        // Если other - это обычный конечный массив, то
        if ( GetCardinality().IsInfinite() ) {
            throw std::logic_error( "LazySequence: Cannot materialize infinite sequence to concat with plain array" );
        }
        
        MaterializeAll();
        // Склеиваем кэши
        Sequence<T>* result_cache = cache_->Concat( other );
        LazySequence<T>* result = new LazySequence<T>();
        delete result->cache_;
        result->cache_ = result_cache;
        return result;
    }

    // ==== Функциональные операции (Map(), Reduce(), Where(), Zip()) ====

    template <typename T2>
    LazySequence<T2>* Map( T2 ( *func )( T ) ) const {
        // Берем копию текущего генератора и оборачиваем её в MapGenerator
        IGenerator<T2>* map_gen = new MapGenerator<T, T2>( generator_->Clone(), func );
        return new LazySequence<T2>( map_gen );
    }

    LazySequence<T>* Where( bool ( *predicate )( T ) ) const {
        IGenerator<T>* where_gen = new WhereGenerator<T>( generator_->Clone(), predicate );
        return new LazySequence<T>( where_gen );
    }

    T Reduce( T ( * func )( T, T ), T initial ) const {
        // Reduce невозможно применить к бесконечности
        if ( GetCardinality().IsInfinite() ) {
            throw std::logic_error( "LazySequence: Cannot reduce an infinite sequence!" );
        }
        
        // Здесь полная материализация безопасна, поскольку список точно конечный
        MaterializeAll(); 
        T result = initial;
        for ( int i = 0; i < cache_->GetLength(); ++i )
            result = func( result, cache_->Get( i ) );
        return result;
    }

    // template <typename T2>
    // LazySequence<my_utils::Pair<T, T2>>* Zip( Sequence<T2>* seq ) const {
    //     MaterializeAll();
    //     LazySequence<my_utils::Pair<T, T2>>* result = new LazySequence<my_utils::Pair<T, T2>>();
    //     int min_len = std::min( cache_->GetLength(), seq->GetLength() );

    //     for ( int i = 0; i < min_len; ++i ) {
    //         my_utils::Pair<T, T2> pair( cache_->Get( i ), seq->Get( i ) );
    //         result = static_cast<LazySequence<my_utils::Pair<T, T2>>*>( result->Append( pair ) );
    //     }
    //     return result;
    // }

protected:
    // Поля mutable, чтобы их можно было менять внутри const-методов 
    mutable Sequence<T>*   cache_;
    mutable IGenerator<T>* generator_;
    mutable bool           is_exhausted_;

    // Конструктор специально для методов Map, Where и т.д.
    explicit LazySequence( IGenerator<T>* generator ) : is_exhausted_( false ) {
        cache_ = new MutableArraySequence<T>();
        generator_ = generator;
    }

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

