#pragma once

#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/BufferSpec.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // APICaster
    ////////////////////////////////////////////////////////////////////////////////////
    class APICaster
    {
    private:
        template<typename T, typename TInput>
        friend constexpr T api_cast(TInput input) requires((std::is_pointer_v<T>&& std::is_pointer_v<TInput>) && std::is_same_v<typename TInput::Type, T>);
        template<typename T, typename TInput>
        friend constexpr T api_cast(TInput input) requires((std::is_pointer_v<T>&& std::is_pointer_v<TInput>) && !requires{ TInput::Type; });
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Casting methods
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T, typename TInput>
    inline constexpr T api_cast(TInput input) requires((std::is_pointer_v<T>&& std::is_pointer_v<TInput>) && std::is_same_v<typename TInput::Type, T>)
    {
        return std::assume_aligned<alignof(T)>(std::launder(reinterpret_cast<T>(&input->m_Impl)));
    }

    template<typename T, typename TInput>
    inline constexpr T api_cast(TInput input) requires((std::is_pointer_v<T>&& std::is_pointer_v<TInput>) && !requires{ TInput::Type; })
    {
        return std::assume_aligned<alignof(T)>(std::launder(reinterpret_cast<T>(input)));
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // APIObject
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    class APIObject // Note: This class exists since reinterpreting from a class to another class is undefined behaviour unless the underlying type is just a byte array
    {
    public:
        // Constructor & Destructor
        APIObject() = default;
        inline ~APIObject()
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
                api_cast<T*>(m_Storage)->~T();
        }

        // Operators
        inline operator T& () { return Get(); }
        inline operator const T& () const { return Get(); }
        inline operator T* () { return api_cast<T*>(m_Storage); }
        inline operator const T* () const { return api_cast<const T*>(m_Storage); }

        inline T* operator -> () { return api_cast<T*>(m_Storage); }
        inline const T* operator -> () const { return api_cast<const T*>(m_Storage); }

        // Getters
        [[nodiscard]] inline T& Get() { return *api_cast<T*>(m_Storage); }
        [[nodiscard]] inline const T& Get() const { return *api_cast<const T*>(m_Storage); }

        // Methods
        template<typename ...TArgs>
        inline void Construct(TArgs&& ...args)
        {
            new (m_Storage) T(std::forward<TArgs>(args)...);
        }

    private:
        // std::aligned_storage is deprecated as of C++23
        alignas(T) std::byte m_Storage[sizeof(T)] = {};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Specification helper methods
    ////////////////////////////////////////////////////////////////////////////////////
    ImageSliceSpecification ResolveImageSlice(const ImageSliceSpecification& sliceSpec, const ImageSpecification& imageSpec);
    ImageSubresourceSpecification ResolveImageSubresource(const ImageSubresourceSpecification& subresourceSpec, const ImageSpecification& imageSpec, bool singleMip);

    BufferRange ResolveBufferRange(const BufferRange& range, const BufferSpecification& specs);

}