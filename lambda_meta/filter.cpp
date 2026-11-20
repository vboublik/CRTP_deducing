#include <concepts>
#include <iostream>
#include <type_traits>

template<typename T>
struct IsNotInt {
    static constexpr bool value = !std::same_as<T, int>;
};

constexpr auto is_not_int = []<typename T>() ->
    std::bool_constant<!std::same_as<T, int>> { return {}; };

template<typename... Ts> struct type_list {};

template<typename... As, typename... Bs>
constexpr type_list<As..., Bs...>
operator+(type_list<As...>, type_list<Bs...>) { return {}; }

// Classical: the predicate is a template template parameter
template<template<typename> class Pred, typename List> struct filter;

template<template<typename> class Pred, typename... Ts>
struct filter<Pred, type_list<Ts...>> {
    using type = decltype((type_list<>{} + ... +
        std::conditional_t<Pred<Ts>::value,
                           type_list<Ts>, type_list<>>{}));
};

// The predicate is a function argument: any object with a templated
// operator()<T>(), a lambda meta-expression in particular
template<typename... Ts, typename P>
constexpr auto filter_by(type_list<Ts...>, P pred) {
    return (type_list<>{} + ... +
        std::conditional_t<
            decltype(pred.template operator()<Ts>())::value,
            type_list<Ts>, type_list<>>{});
}

using L = type_list<int, double, int, char>;

static_assert(std::is_same_v<filter<IsNotInt, L>::type,
                             type_list<double, char>>);
static_assert(std::is_same_v<decltype(filter_by(L{}, is_not_int)),
                             type_list<double, char>>);

// A predicate written at the call site: possible only with a lambda
static_assert(std::is_same_v<
    decltype(filter_by(L{}, []<typename T>() ->
        std::bool_constant<std::is_floating_point_v<T>> { return {}; })),
    type_list<double>>);

// A hand-written function object works as well, but, like IsNotInt,
// it cannot be local: local classes may not have member templates
struct NotIntObject {
    template <typename T>
    constexpr auto operator()() const
        -> std::bool_constant<!std::same_as<T, int>> {
        return {};
    }
};
static_assert(std::is_same_v<decltype(filter_by(L{}, NotIntObject{})),
                             type_list<double, char>>);

int main() {
    // Reaching this point means every static_assert above has passed
    std::cout << "Type filter: all compile-time checks passed" << std::endl;
    return 0;
}
