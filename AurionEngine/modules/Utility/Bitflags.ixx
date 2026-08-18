module;

#include <type_traits>

export module Aurion.Utility:BitFlags;

export namespace Aurion
{
    // Compile-Time Bitwise toggle utility
    template<typename T>
    struct EnableBitwiseFlags : std::false_type {};

    // Utility class for processing enum classes whose values evaluate to bit identities.
    template<typename T>
    struct Flags
    {
        using mask_t = std::underlying_type_t<T>;

        constexpr Flags() noexcept : m_mask(0) {};
        explicit constexpr Flags(mask_t mask) noexcept : m_mask(mask) {};
        constexpr Flags(T flag) noexcept : m_mask(static_cast<mask_t>(flag)) {};

        constexpr Flags<T> operator&(const Flags<T>& rhs) const noexcept { return Flags<T>(m_mask & rhs.m_mask); };
        constexpr Flags<T> operator|(const Flags<T>& rhs) const noexcept { return Flags<T>(m_mask | rhs.m_mask); };
        constexpr Flags<T> operator^(const Flags<T>& rhs) const noexcept { return Flags<T>(m_mask ^ rhs.m_mask); };
        constexpr Flags<T> operator~() const noexcept { return Flags<T>(static_cast<mask_t>(~m_mask)); };

        constexpr Flags<T>& operator=(const Flags<T>&) noexcept = default;
        constexpr Flags<T>& operator|=(const Flags<T>& rhs) noexcept { m_mask |= rhs.m_mask; return *this; };
        constexpr Flags<T>& operator&=(const Flags<T>& rhs) noexcept { m_mask &= rhs.m_mask; return *this; };
        constexpr Flags<T>& operator^=(const Flags<T>& rhs) noexcept { m_mask ^= rhs.m_mask; return *this; };

        constexpr Flags<T>& operator=(T rhs) noexcept { m_mask = static_cast<mask_t>(rhs); return *this; };

        explicit constexpr operator bool() const noexcept { return !!m_mask; }

    private: mask_t m_mask;
    };

    // Utility operators for bit flag use with Flags<T>

    template<typename T>
    constexpr Flags<T> operator&(T bit, const Flags<T>& flags) noexcept { return Flags<T>(bit) & flags; }

    template<typename T>
    constexpr Flags<T> operator|(T bit, const Flags<T>& flags) noexcept { return Flags<T>(bit) | flags; }

    template<typename T>
    constexpr Flags<T> operator^(T bit, const Flags<T>& flags) noexcept { return Flags<T>(bit) ^ flags; }

    template<typename T, typename = std::enable_if_t<EnableBitwiseFlags<T>::value>>
    constexpr Flags<T> operator&(T lhs, T rhs) noexcept { return Flags<T>(lhs) & rhs; };

    template<typename T, typename = std::enable_if_t<EnableBitwiseFlags<T>::value>>
    constexpr Flags<T> operator|(T lhs, T rhs) noexcept { return Flags<T>(lhs) | rhs; };

    template<typename T, typename = std::enable_if_t<EnableBitwiseFlags<T>::value>>
    constexpr Flags<T> operator^(T lhs, T rhs) noexcept { return Flags<T>(lhs) ^ rhs; };

    template<typename T, typename = std::enable_if_t<EnableBitwiseFlags<T>::value>>
    constexpr Flags<T> operator~(T bit) noexcept { return ~( Flags<T>(bit) ); };
}