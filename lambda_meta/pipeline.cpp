#include <iostream>
#include <type_traits>
#include <typeinfo>

// Basic lambda meta-expressions (pipeline building blocks).
// Each step returns the tag std::type_identity<X>, not a value of X
constexpr auto remove_ref = []<typename T>() {
    return std::type_identity<std::remove_reference_t<T>>{};
};
constexpr auto remove_const = []<typename T>() {
    return std::type_identity<std::remove_const_t<T>>{};
};
constexpr auto add_ptr = []<typename T>() {
    return std::type_identity<std::add_pointer_t<T>>{};
};

// Combinator for sequential composition (type pipeline)
template <typename T>
struct meta_pipeline {
    using type = T;  // terminal type
    template <typename F>
    constexpr auto operator|(F f) const {
        using next_type =
            typename decltype(f.template operator()<T>())::type;
        return meta_pipeline<next_type>{};
    }
};

int main() {
    // Pass const double& through the lambda meta-expression pipeline
    constexpr auto pipeline = meta_pipeline<const double&>{}
        | remove_ref    // const double& -> const double
        | remove_const  // const double  -> double
        | add_ptr;      // double        -> double*

    using FinalType = typename decltype(pipeline)::type;

    // Compile-time verification (zero runtime overhead)
    static_assert(std::is_same_v<
        decltype(meta_pipeline<const double&>{} | remove_ref)::type,
        const double>, "const lost at the first step!");
    static_assert(std::is_same_v<FinalType, double*>,
                  "Metaprogramming error!");

    std::cout << "Type successfully transformed: "
              << typeid(FinalType).name() << std::endl;
    return 0;
}
