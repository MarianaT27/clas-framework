#pragma once

#include <string_view>
#include <tuple>

namespace framework {

template <typename T, typename Member>
struct Field {
    std::string_view name;
    Member T::*      ptr;
};

template <typename T, typename Member>
constexpr auto field(std::string_view name, Member T::* ptr) -> Field<T, Member> {
    return Field<T, Member>{name, ptr};
}

template <typename T>
concept HasStaticNTupleFields = requires { T::ntuple_fields(); };

template <typename T>
concept HasStaticConfigFields = requires { T::config_fields(); };

}  // namespace framework
