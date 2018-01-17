#ifndef ELEMENT__H
#define ELEMENT__H


#include <cstdlib>
#include <string>
#include <functional>



class Element{
public:
    Element();
    Element(long long i);
    uint64_t hash1() const;
    uint64_t hash2() const;
private:
    long long h1;
    long long h2;
   // std::hash<long long> hash_fn;
};


#endif
