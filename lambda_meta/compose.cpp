// Composition of lambda meta-expressions and the laws of a category
#include <concepts>
#include <iostream>
#include <type_traits>

// The steps of Section 3 and the identity
constexpr auto remove_ref = []<typename T>() {
    return std::type_identity<std::remove_reference_t<T>>{};
};
constexpr auto remove_const = []<typename T>() {
    return std::type_identity<std::remove_const_t<T>>{};
};
constexpr auto add_ptr = []<typename T>() {
    return std::type_identity<std::add_pointer_t<T>>{};
};
// Identity: returns its argument unchanged
constexpr auto id = []<typename T>() {
    return std::type_identity<T>{};
};

// A lambda meta-expression: an empty, default-constructible object
// whose operator()<T>() returns a tag with a nested ::type
template <class M>
concept meta_expression =
    std::is_empty_v<M> && std::default_initializable<M> &&
    requires {
        typename decltype(M{}.template operator()<int>())::type;
    };

// Composition: f >> g is a new meta-expression "first f, then g"
template <meta_expression F, meta_expression G>
constexpr auto operator>>(F, G) {
    return []<typename T>() {
        using A = typename decltype(F{}.template operator()<T>())::type;
        return decltype(G{}.template operator()<A>()){};
    };
}

// Apply a meta-expression M to a type T
template <class M, class T>
using apply_t = typename decltype(M{}.template operator()<T>())::type;

// The laws of a category for the type T, with f, g, h as in Fig. 2
template <class T>
constexpr bool category_laws_hold =
    // id >> f = f
    std::is_same_v<apply_t<decltype(id >> remove_ref), T>,
                   apply_t<decltype(remove_ref), T>> &&
    // f >> id = f
    std::is_same_v<apply_t<decltype(remove_ref >> id), T>,
                   apply_t<decltype(remove_ref), T>> &&
    // (f >> g) >> h = f >> (g >> h)
    std::is_same_v<
        apply_t<decltype((remove_ref >> remove_const) >> add_ptr), T>,
        apply_t<decltype(remove_ref >> (remove_const >> add_ptr)), T>>;

static_assert(category_laws_hold<const double&>);
static_assert(category_laws_hold<int[3]>);
static_assert(category_laws_hold<void>);
static_assert(std::is_same_v<
    apply_t<decltype(remove_ref >> remove_const >> add_ptr),
            const double&>,
    double*>);

int main() {
    // Reaching this point means every static_assert above has passed
    std::cout << "Category laws: all compile-time checks passed" << std::endl;
    return 0;
}
