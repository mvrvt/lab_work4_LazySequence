#pragma once

#include <stdexcept>
#include "lab2_files/Sequence.hpp"
#include "lab2_files/ArraySequence.hpp"
#include "Generator.hpp"
#include "MyUtils.hpp"

// Вспомогательный класс-итератор для ленивой последовательности
template <typename T>
class LazyEnumerator : public IEnumerator<T> {
public:
    explicit LazyEnumerator( const Sequence<T>* seq_ ) : seq_( seq ), current_index_( -1 ) { }

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
    int   current_index_;
};

template <typename T>
class LazySequence : public Sequence<T> {
public:
    explicit LazySequence( IGenerator<T>* generator ) : generator_( generator ), is_exhausted_( false ) {
        if ( !generator ) 
            throw std::invalid_argument( "LazySequence: can't be null" );
        
        cache_ = new MutableArraySequence<T>();
    }

    ~LazySequence() override {
        delete generator_;
        delete cache_;
    }

    // --- ICollection / IEnumerable ---

    IEnumerator<T>* GetEnumerator() const override {
        return new LazyEnumerator<T>( this );
    }

    std::size_t GetCount() const override {
        MaterializeAll();
        return cache_->GetCount();
    }

    // --- Декомпозиция ---

    const T& Get( std::size_t index ) const override {
        int idx = static_cast<int>( index );
        if ( idx < 0 )
            throw IndexOutOfRange( "LazySequence: index can't be negative" );
        
        MaterializeUpTo( idx );
        
        if ( idx >= cache_->GetLength() )
            throw IndexOutOfRange( "LazySequence: index out of range (sequence ended)" );
        
        // Возвращаем из кэша
        return cache_->Get( idx );
    }

    T& Get( std::size_t index ) override {
        // Делегируем вызов константной версии и снимает const
        return const_cast<T&>( static_cast<const LazySequence<T>*>( this )->Get( index ) );
    }

    T& GetFirst() const override {
        return const_cast<T&>( Get(0) );
    }

    T& GetLast() const override {
        MaterializeAll();
        if (cache_->GetLength() == 0) {
            throw IndexOutOfRange("LazySequence: sequence is empty");
        }
        return cache_->Get(cache_->GetLength() - 1);
    }

    int GetLength() const override {
        MaterializeAll();
        return cache_->GetLength();
    }

    Sequence<T>* GetSubsequence(int start, int end) const override {
        if (start < 0 || end < start) {
            throw IndexOutOfRange("LazySequence: invalid indices for subsequence");
        }
        MaterializeUpTo(end);
        return cache_->GetSubsequence(start, end);
    }

    // --- Операции (пока делаем заглушки или базовую реализацию) ---

    Sequence<T>* CreateEmpty() const override {
        // Возвращаем пустой массив, так как ленивый пустой список не имеет смысла
        return new MutableArraySequence<T>();
    }

    Sequence<T>* Append(const T& item) override {
        // По правилам ленивых списков[cite: 33], мы должны создать новый генератор (ConcatGenerator)
        // Для простоты на данном этапе материализуем, позже напишем ConcatGenerator
        MaterializeAll();
        return cache_->Append(item);
    }

    Sequence<T>* Prepend(const T& item) override {
        MaterializeAll();
        return cache_->Prepend(item);
    }

    Sequence<T>* InsertAt(const T& item, int index) override {
        MaterializeAll();
        return cache_->InsertAt(item, index);
    }

    Sequence<T>* Concat(Sequence<T>* other) const override {
        MaterializeAll();
        return cache_->Concat(other);
    }

    // --- Специфичные методы ленивой последовательности ---

    std::size_t GetMaterializedCount() const {
        return static_cast<std::size_t>(cache_->GetLength());
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

