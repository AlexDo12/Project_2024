#include <iostream>
#include "mergesort.h"
#include "radix.h"
#include "bitonic.h"
#include "sample.h"


int main() {
    std::cout << "Running main" << std::endl;
    
    mergesort();
    bitonic();
    radix();
    sample();
    
    
    
    return 0;
}
