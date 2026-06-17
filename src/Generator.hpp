#pragma once
#include <stdexcept>
#include "MyUtils.hpp"

template <typename T>
class IGenerator {
public:
    virtual ~IGenerator() = default; 
    virtual T GetAt(my_utils::Ordinal idx) = 0;
    virtual my_utils::Ordinal GetCardinality() const = 0;
    virtual IGenerator<T>* Clone() const = 0;
};

// 1. Базовый генератор (Rule)
template <typename T>
class RuleGenerator : public IGenerator<T> {
public:
    typedef T (*RuleFunc)(int);

    RuleGenerator(RuleFunc rule, my_utils::Ordinal card) : rule_(rule), cardinality_(card) {}
    
    T GetAt(my_utils::Ordinal idx) override {
        if (idx >= cardinality_) throw std::out_of_range("RuleGenerator: index out of range");
        return rule_(idx.finite); // Базовое правило работает только с конечными индексами
    }

    IGenerator<T>* Clone() const override { return new RuleGenerator<T>(rule_, cardinality_); }
    my_utils::Ordinal GetCardinality() const override { return cardinality_; }

private:
    RuleFunc rule_;
    my_utils::Ordinal cardinality_;
};

// 2. Map Generator
template <typename T, typename T2>
class MapGenerator : public IGenerator<T2> {
public:
    typedef T2 (*TransformFunc)(const T&);

    MapGenerator(IGenerator<T>* source, TransformFunc func) : func_(func) { source_ = source->Clone(); }
    ~MapGenerator() override { delete source_; }

    T2 GetAt(my_utils::Ordinal idx) override { return func_(source_->GetAt(idx)); }
    IGenerator<T2>* Clone() const override { return new MapGenerator<T, T2>(source_, func_); }
    my_utils::Ordinal GetCardinality() const override { return source_->GetCardinality(); }

private:
    IGenerator<T>* source_;
    TransformFunc func_;
};

// 3. Concat Generator (Магия ординалов)
template <typename T>
class ConcatGenerator : public IGenerator<T> {
public:
    ConcatGenerator(IGenerator<T>* left, IGenerator<T>* right) {
        left_ = left->Clone();
        right_ = right->Clone();
    } 
    ~ConcatGenerator() override { delete left_; delete right_; }

    T GetAt(my_utils::Ordinal idx) override {
        my_utils::Ordinal left_card = left_->GetCardinality();
        if (idx < left_card) {
            return left_->GetAt(idx);
        } else {
            return right_->GetAt(idx - left_card);
        }
    }

    IGenerator<T>* Clone() const override { return new ConcatGenerator<T>(left_, right_); }
    my_utils::Ordinal GetCardinality() const override { return left_->GetCardinality() + right_->GetCardinality(); }

private:
    IGenerator<T>* left_;
    IGenerator<T>* right_;
};

// 4. Skip Generator
template <typename T>
class SkipGenerator : public IGenerator<T> {
public:
    SkipGenerator(IGenerator<T>* source, int count) : count_(count) { source_ = source->Clone(); }
    ~SkipGenerator() override { delete source_; }

    T GetAt(my_utils::Ordinal idx) override {
        return source_->GetAt(idx + my_utils::Ordinal(0, count_));
    }
    IGenerator<T>* Clone() const override { return new SkipGenerator<T>(source_, count_); }
    my_utils::Ordinal GetCardinality() const override { 
        auto card = source_->GetCardinality();
        if (card.omega0 == 0 && card.finite <= count_) return my_utils::Ordinal(0, 0);
        return my_utils::Ordinal(card.omega0, card.finite > count_ ? card.finite - count_ : 0);
    }
private:
    IGenerator<T>* source_;
    int count_;
};
