#pragma once

#include <new>
#include <concepts>
#include <type_traits>

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Safe reinterpretation
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T, typename TInput>
    T safe_reinterpret(TInput input) requires((std::is_pointer_v<TInput> || std::is_reference_v<TInput>) && (std::is_pointer_v<T> || std::is_reference_v<T>))
    {
        return std::launder(reinterpret_cast<T>(input));
    }

}