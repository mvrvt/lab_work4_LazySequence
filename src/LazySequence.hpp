#pragma once
#include <stdexcept>
#include "lab2_files/Sequence.hpp"
#include "Generator.hpp"
#include "MyUtils.hpp"
#include "SlidingCache.hpp"

template <typename T>
class LazySequence : public Sequence<T> {
public:
    LazySequence( typename RuleGenerator<T>::RuleFunc rule, my_utils::Ordinal card, int window_size = 100 ) 
        : cardinality_( card ), window_size_( window_size ) {
        generator_ = new RuleGenerator<T>( rule, card );
        cache_ = new SlidingCache<T>( window_size );
    }

    LazySequence( IGenerator<T>* gen, int window_size = 100 ) 
        : cardinality_( gen->GetCardinality() ), window_size_( window_size ) {
        generator_ = gen->Clone();
        cache_ = new SlidingCache<T>( window_size );
    }

    ~LazySequence() override {
        delete generator_;
        delete cache_;
    }

    const T& GetByOrdinal( my_utils::Ordinal idx ) const {
        if ( idx >= cardinality_ ) throw std::out_of_range("LazySequence: Index out of range");

        if ( idx.omega0 == 0 ) {
            int int_idx = idx.finite;
            if ( cache_->Contains( int_idx ) ) {
                return cache_->Get( int_idx );
            }
            T value = generator_->GetAt( idx );
            cache_->Push( value, int_idx );
            return cache_->Get( int_idx ); 
        }

        // Для трансфинитных элементов (ω0 + k)
        static T transfinite_temp;
        transfinite_temp = generator_->GetAt( idx );
        return transfinite_temp;
    }

    T& Get( std::size_t index ) override {
        return const_cast<T&>( GetByOrdinal( my_utils::Ordinal( 0, index ) ) );
    }
    const T& Get( std::size_t index ) const override {
        return GetByOrdinal( my_utils::Ordinal( 0, index ) );
    }
    
    my_utils::Ordinal GetOrdinalCardinality() const { return cardinality_; }
    
    int GetLength() const override { 
        if ( cardinality_.IsInfinite() ) throw std::logic_error( "Cannot get int length of infinite sequence" );
        return cardinality_.finite; 
    }

    template <typename T2>
    LazySequence<T2>* Map( typename MapGenerator<T, T2>::TransformFunc func ) const {
        IGenerator<T2>* map_gen = new MapGenerator<T, T2>( this->generator_, func );
        auto* res = new LazySequence<T2>( map_gen, window_size_ );
        delete map_gen;
        return res;
    }

    LazySequence<T>* Skip( int count ) const {
        IGenerator<T>* skip_gen = new SkipGenerator<T>( this->generator_, count );
        auto* res = new LazySequence<T>( skip_gen, window_size_ );
        delete skip_gen;
        return res;
    }

    LazySequence<T>* ConcatSequence( const LazySequence<T>* other ) const {
        IGenerator<T>* concat_gen = new ConcatGenerator<T>( this->generator_, other->generator_ );
        auto* res = new LazySequence<T>( concat_gen, window_size_ );
        delete concat_gen;
        return res;
    }

    // Заглушки под чистый виртуальный интерфейс Sequence
    T&              GetFirst() const override { return const_cast<T&>(GetByOrdinal(my_utils::Ordinal(0, 0))); }
    T&              GetLast() const override { throw std::logic_error("Not supported for Lazy"); }
    Sequence<T>*    Append( const T& ) override { throw std::logic_error("Use specific pipeline"); }
    Sequence<T>*    Prepend( const T& ) override { throw std::logic_error("Use specific pipeline"); }
    Sequence<T>*    InsertAt( const T&, int ) override { throw std::logic_error("Use specific pipeline"); }
    Sequence<T>*    Concat( Sequence<T>* ) const override { throw std::logic_error("Use ConcatSequence"); }
    IEnumerator<T>* GetEnumerator() const override { return nullptr; }
    Sequence<T>*    GetSubsequence( int, int ) const override { return nullptr; }
    Sequence<T>*    Clone() const override { return new LazySequence<T>(generator_, window_size_); }
    Sequence<T>*    CreateEmpty() const override { return nullptr; }

private:
    IGenerator<T>*           generator_;
    my_utils::Ordinal        cardinality_;
    int                      window_size_;
    mutable SlidingCache<T>* cache_;
};
