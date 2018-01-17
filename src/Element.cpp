#include "Element.h"


Element::Element(long long i) : h1(i), h2(i*11){

}

Element::Element(){
	
}


uint64_t Element::hash1() const{
    return h1;
}


uint64_t Element::hash2() const{
    return h2*11;
}
