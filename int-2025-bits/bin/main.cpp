#include <lib/number.h>
#include <iostream>



int main() {
    int32_t a = 10;
    int32_t c = 20;
    int2025_t b = from_int(a);
    int2025_t d = from_int(c);
    std::cout << d + b << '\n';
    return 0;
}