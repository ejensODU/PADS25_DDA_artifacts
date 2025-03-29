#pragma once

#include <cstddef>
#include <string>

template <typename T>
class OoO_SV {
private:
    std::string _name;
    T _value;
    T _minLimit;
    T _maxLimit;
    size_t _modelIndex;
    static size_t _numSVs;
public:
    OoO_SV(std::string name, T initialValue, T minLimit, T maxLimit);
    T get() const;
    void set(T newValue);
    void inc(T incrementBy = 1);
    void dec(T decrementBy = 1);
    std::string getName() const;
    size_t getModelIndex() const;
};

// Include the implementation file
#include "OoO_SV.tpp"
