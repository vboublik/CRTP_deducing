#include <iostream>
#include <type_traits>
#include <typeinfo>

using FinalType = std::add_pointer_t<
                    std::remove_const_t<
                      std::remove_reference_t<const double&>>>;
static_assert(std::is_same_v<FinalType, double*>);

int main() {
    std::cout << "Classical notation, type successfully transformed: "
              << typeid(FinalType).name() << std::endl;
    return 0;
}
