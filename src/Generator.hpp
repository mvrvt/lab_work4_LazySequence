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

    // Возвращает мощность генератора
    virtual my_utils::Cardinality GetCardinality() const = 0;

};

// Базовый генератор на основе правила (функции)
template <typename T>
class RuleGenerator : public IGenerator<T> {
public:
    typedef std::function<T( const Sequence<T>* )> RuleFunc;

    RuleGenerator( RuleFunc rule, const Sequence<T>* cache_ref, 
                   my_utils::Cardinality card = my_utils::Cardinality( 1, 0 ), int generated_count = 0 ) 
                   : rule_( rule ), cache_ref_( cache_ref ), cardinality_( card ), generated_count_( generated_count ) {
        if ( !rule_ )
            throw std::out_of_range( "RuleGenerator: rule function can't be null" );
    }
    
    T GetNext() override {
        if ( !HasNext() ) 
            throw std::out_of_range( "RuleGenerator: reached end of sequence" );
        T value = rule_( cache_ref_ );
        generated_count_++;
        return value;
    }

    my_utils::Optional<T> TryGetNext() override {
        if ( HasNext() ) return my_utils::Optional<T>( GetNext() );
        return my_utils::Optional<T>();
    }

    bool HasNext() const override {
        // Если он бесконечный - элементы есть всегда
        if ( cardinality_.IsInfinite() ) return true;
        // Если конечный - проверяем, не превышен ли лимит
        return generated_count_ < cardinality_.finite_elements;
    }

    IGenerator<T>* Clone() const override {
        return new RuleGenerator<T>( rule_, cache_ref_, cardinality_, generated_count_ );
    }

    my_utils::Cardinality GetCardinality() const override {
        return cardinality_;
    }

private:
    RuleFunc              rule_;
    const Sequence<T>*    cache_ref_;
    my_utils::Cardinality cardinality_;
    int                   generated_count_; // счётчик выданных элементов 
};

// Композитный генератор для ленивой конкатенации
template <typename T>
class ConcatGenerator : public IGenerator<T> {
public:
    ConcatGenerator( IGenerator<T>* left, IGenerator<T>* right ) {
        left_gen_  = left->Clone();
        right_gen_ = right->Clone();
    } 

    ~ConcatGenerator() override {
        delete left_gen_;
        delete right_gen_;
    }

    // 
    my_utils::Optional<T> TryGetNext() override {
        auto left_val = left_gen_->TryGetNext();
        if ( left_val.HasValue() ) 
            return left_val;
        
        // Если левая LazySeq закончилась, то начинаем тянуть из правой
        return right_gen_->TryGetNext();
    }

    T GetNext() override {
        auto val = TryGetNext();
        if ( !val.HasValue() ) throw std::out_of_range( "ConcatGenerator: reached end" );
        return val.Value();
    }

    bool HasNext() const override {
        return left_gen_->HasNext() || right_gen_->HasNext();
    }

    IGenerator<T>* Clone() const override {
        return new ConcatGenerator<T>( left_gen_, right_gen_ );
    }

    my_utils::Cardinality GetCardinality() const override {
        return left_gen_->GetCardinality() + right_gen_->GetCardinality();
    }

private:
    IGenerator<T>* left_gen_;
    IGenerator<T>* right_gen_;
};

template <typename T, typename T2>
class MapGenerator : public IGenerator<T2> {
public:
    // Конструктор принимает указатель на исходный генератор и функцию.
    // Важно: мы берем владение над source_, поэтому в деструкторе его удаляем
    MapGenerator( IGenerator<T>* source, T2 ( *func )( T ) ) : source_( source ), func_( func ) {}

    ~MapGenerator() override {
        delete source_;
    }

    my_utils::Optional<T2> TryGetNext() override {
        auto val = source_->TryGetNext();
        if ( val.HasValue() ) {
            return my_utils::Optional<T2>( func_( val.Value() ) );
        }
        return my_utils::Optional<T2>(); // Если источник иссяк
    }

    T2 GetNext() override {
        auto val = TryGetNext();
        if ( !val.HasValue() ) throw std::out_of_range( "MapGenerator: reached end" );
        return val.Value();
    }

    bool HasNext() const override {
        return source_->HasNext();
    }

    IGenerator<T2>* Clone() const override {
        // При клонировании обязательно клонируем и внутренний генератор
        return new MapGenerator<T, T2>( source_->Clone(), func_ );
    }

    my_utils::Cardinality GetCardinality() const override {
        return source_->GetCardinality(); // Мощность не меняется
    }

private:
    IGenerator<T>* source_;
    T2             ( *func_ )( T );
};

template <typename T>
class WhereGenerator : public IGenerator<T> {
public:
    WhereGenerator( IGenerator<T>* source, bool ( *pred )( T ) ) : source_( source ), pred_( pred ) {}

    ~WhereGenerator() override { delete source_; }

    my_utils::Optional<T> TryGetNext() override {
        // Крутим цикл, пока в источнике есть элементы
        while ( source_->HasNext() ) {
            auto val = source_->TryGetNext();
            if ( val.HasValue() && pred_( val.Value() ) ) {
                return val; // Нашли подходящий — сразу отдаём
            }
        }
        return my_utils::Optional<T>();
    }

    T GetNext() override {
        auto val = TryGetNext();
        if ( !val.HasValue() ) throw std::out_of_range( "WhereGenerator: reached end" );
        return val.Value();
    }

    bool HasNext() const override {
        // Мы не знаем наверняка, есть ли подходящие элементы дальше,
        // поэтому опираемся на источник.
        return source_->HasNext(); 
    }

    IGenerator<T>* Clone() const override {
        return new WhereGenerator<T>( source_->Clone(), pred_ );
    }

    my_utils::Cardinality GetCardinality() const override {
        return source_->GetCardinality(); 
    }

private:
    IGenerator<T>* source_;
    bool           ( *pred_ )( T );
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
    my_utils::Cardinality GetCardinality() const override {
        return my_utils::Cardinality( 0, 0 );
    }
};

// append and insert generator'ы
// грубо говоря на каждую операцию нужен свой генератор
