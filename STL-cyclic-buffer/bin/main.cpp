#include <iostream>
#include "../lib/circular_buffer.h"
#include <string>



int main() {
circular_buffer<int> cb(5);

for(int i=0;i<20;i++)
    cb.push_back(i);

for(auto x : cb)
    std::cout << x << " ";
}
