#pragma once
#include "../external/string_view.hpp"

namespace csv {
    using namespace nonstd;

    #if __cplusplus == 201703L
        #include <string_view>
        using string_view = std::string_view;
    #else
        using string_view = nonstd::string_view;
    #endif
}