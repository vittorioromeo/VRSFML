#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"
#include "SFML/Base/Trait/MakeUnsigned.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"


////////////////////////////////////////////////////////////
// References:
//
// https://www.unicode.org/
// https://www.unicode.org/Public/PROGRAMS/CVTUTF/ConvertUTF.c
// https://www.unicode.org/Public/PROGRAMS/CVTUTF/ConvertUTF.h
// https://people.w3.org/rishida/scripts/uniview/conversion
//
////////////////////////////////////////////////////////////


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename In, typename Out>
inline constexpr Out copyBits(In begin, const In end, Out output)
{
    using InputType = SFML_BASE_REMOVE_CVREF(decltype(*begin));
    static_assert(SFML_BASE_IS_INTEGRAL(InputType));

    using OutputType = typename Out::container_type::value_type;
    static_assert(SFML_BASE_IS_INTEGRAL(OutputType));

    static_assert(sizeof(OutputType) >= sizeof(InputType));

    // The goal is to copy the byte representation of the input into the output type.
    // A single static_cast will try to preserve the value as opposed to the byte representation
    // which leads to issues when the input is signed and has a negative value. That will get
    // wrapped to a very large unsigned value which is incorrect. To address this, we first
    // cast the input to its unsigned equivalent then cast that to the destination type which has
    // the property of preserving the byte representation of the input. A simple memcpy seems
    // like a viable solution but copying the bytes of a type into a larger type yields different
    // results on big versus little endian machines so it's not a possibility.
    //
    // Why do this? For example take the Latin1 character é. It has a byte representation of 0xE9
    // and a signed integer value of -23. If you cast -23 to a char32_t, you get a value of
    // 4294967273 which is not a valid Unicode codepoint. What we actually wanted was a char32_t
    // with the byte representation 0x000000E9.
    while (begin != end)
        *output++ = static_cast<OutputType>(static_cast<base::MakeUnsigned<InputType>>(*begin++));

    return output;
}

} // namespace sf::priv


namespace sf
{
template <unsigned int N>
class Utf;

////////////////////////////////////////////////////////////
/// \brief Specialization of the Utf template for UTF-8
///
////////////////////////////////////////////////////////////
template <>
class Utf<8>
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Decode a single UTF-8 character
    ///
    /// Decoding a character means finding its unique 32-bits
    /// code (called the codepoint) in the Unicode standard.
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Codepoint of the decoded UTF-8 character
    /// \param replacement Replacement character to use in case the UTF-8 sequence is invalid
    ///
    /// \return Iterator pointing to one past the last read element of the input sequence
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    static In decode(In begin, In end, char32_t& output, char32_t replacement)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        // clang-format off
        // Some useful precomputed data
        static constexpr base::U8 trailing[256] =
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
        };

        static constexpr base::U32 offsets[6] =
        {
            0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080
        };
        // clang-format on

        // decode the character
        const auto trailingBytes = trailing[static_cast<base::U8>(*begin)];
        if (trailingBytes < (end - begin))
        {
            output = 0;

            // clang-format off
            switch (trailingBytes)
            {
                case 5: output += static_cast<base::U8>(*begin++); output <<= 6; [[fallthrough]];
                case 4: output += static_cast<base::U8>(*begin++); output <<= 6; [[fallthrough]];
                case 3: output += static_cast<base::U8>(*begin++); output <<= 6; [[fallthrough]];
                case 2: output += static_cast<base::U8>(*begin++); output <<= 6; [[fallthrough]];
                case 1: output += static_cast<base::U8>(*begin++); output <<= 6; [[fallthrough]];
                case 0: output += static_cast<base::U8>(*begin++);
            }
            // clang-format on

            output -= offsets[trailingBytes];
        }
        else
        {
            // Incomplete character
            begin  = end;
            output = replacement;
        }

        return begin;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Encode a single UTF-8 character
    ///
    /// Encoding a character means converting a unique 32-bits
    /// code (called the codepoint) in the target encoding, UTF-8.
    ///
    /// \param input       Codepoint to encode as UTF-8
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to UTF-8 (use 0 to skip them)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename Out>
    static Out encode(char32_t input, Out output, base::U8 replacement)
    {
        // Some useful precomputed data
        static constexpr base::U8 firstBytes[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

        // encode the character
        if ((input > 0x00'10'FF'FF) || ((input >= 0xD8'00) && (input <= 0xDB'FF)))
        {
            // Invalid character
            if (replacement)
                *output++ = static_cast<typename Out::container_type::value_type>(replacement);
        }
        else
        {
            // Valid character

            // Get the number of bytes to write
            base::SizeT bytestoWrite = 1;

            // clang-format off
            if      (input <  0x80)       bytestoWrite = 1;
            else if (input <  0x800)      bytestoWrite = 2;
            else if (input <  0x10000)    bytestoWrite = 3;
            else if (input <= 0x0010FFFF) bytestoWrite = 4;
            // clang-format on

            // Extract the bytes to write
            unsigned char bytes[4]{};

            // clang-format off
            switch (bytestoWrite)
            {
                case 4: bytes[3] = static_cast<unsigned char>((input | 0x80) & 0xBF); input >>= 6; [[fallthrough]];
                case 3: bytes[2] = static_cast<unsigned char>((input | 0x80) & 0xBF); input >>= 6; [[fallthrough]];
                case 2: bytes[1] = static_cast<unsigned char>((input | 0x80) & 0xBF); input >>= 6; [[fallthrough]];
                case 1: bytes[0] = static_cast<unsigned char> (input | firstBytes[bytestoWrite]);
            }
            // clang-format on

            // Add them to the output
            output = priv::copyBits(bytes, bytes + bytestoWrite, output);
        }

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Advance to the next UTF-8 character
    ///
    /// This function is necessary for multi-elements encodings, as
    /// a single character may use more than 1 storage element.
    ///
    /// \param begin Iterator pointing to the beginning of the input sequence
    /// \param end   Iterator pointing to the end of the input sequence
    ///
    /// \return Iterator pointing to one past the last read element of the input sequence
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    static In next(In begin, In end)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        char32_t codepoint = 0;
        return decode(begin, end, codepoint, 0);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Count the number of characters of a UTF-8 sequence
    ///
    /// This function is necessary for multi-elements encodings, as
    /// a single character may use more than 1 storage element, thus the
    /// total size can be different from (begin - end).
    ///
    /// \param begin Iterator pointing to the beginning of the input sequence
    /// \param end   Iterator pointing to the end of the input sequence
    ///
    /// \return Iterator pointing to one past the last read element of the input sequence
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    static base::SizeT count(In begin, In end)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        base::SizeT length = 0;
        while (begin != end)
        {
            begin = next(begin, end);
            ++length;
        }

        return length;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert an ANSI characters range to UTF-8
    ///
    /// The current global locale will be used by default, unless you
    /// pass a custom one in the `locale` parameter.
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    /// \param facet Facet to use for conversion
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out, typename Facet>
    static Out fromAnsi(In begin, In end, Out output, const Facet& facet);

    ////////////////////////////////////////////////////////////
    /// \brief Convert a wide characters range to UTF-8
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out fromWide(In begin, In end, Out output);

    ////////////////////////////////////////////////////////////
    /// \brief Convert a latin-1 (ISO-5589-1) characters range to UTF-8
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out fromLatin1(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        // Latin-1 is directly compatible with Unicode encodings,
        // and can thus be treated as (a sub-range of) UTF-32
        while (begin != end)
            output = encode(static_cast<base::U8>(*begin++), output, 0);

        return output;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert an UTF-8 characters range to ANSI characters
    ///
    /// The current global locale will be used by default, unless you
    /// pass a custom one in the `locale` parameter.
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to ANSI (use 0 to skip them)
    /// \param facet       Facet to use for conversion
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out, typename Facet>
    static Out toAnsi(In begin, In end, Out output, char replacement, const Facet& facet);

    ////////////////////////////////////////////////////////////
    /// \brief Convert an UTF-8 characters range to wide characters
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to wide (use 0 to skip them)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toWide(In begin, In end, Out output, wchar_t replacement);

    ////////////////////////////////////////////////////////////
    /// \brief Convert an UTF-8 characters range to latin-1 (ISO-5589-1) characters
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to wide (use 0 to skip them)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toLatin1(In begin, In end, Out output, char replacement)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        // Latin-1 is directly compatible with Unicode encodings,
        // and can thus be treated as (a sub-range of) UTF-32
        while (begin != end)
        {
            char32_t codepoint = 0;
            begin              = decode(begin, end, codepoint, 0);
            *output++          = codepoint < 256 ? static_cast<char>(codepoint) : replacement;
        }

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert a UTF-8 characters range to UTF-8
    ///
    /// This functions does nothing more than a direct copy;
    /// it is defined only to provide the same interface as other
    /// specializations of the `sf::Utf<>` template, and allow
    /// generic code to be written on top of it.
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toUtf8(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        return priv::copyBits(begin, end, output);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert a UTF-8 characters range to UTF-16
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toUtf16(In begin, In end, Out output);

    ////////////////////////////////////////////////////////////
    /// \brief Convert a UTF-8 characters range to UTF-32
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toUtf32(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        while (begin != end)
        {
            char32_t codepoint = 0;
            begin              = decode(begin, end, codepoint, 0);
            *output++          = codepoint;
        }

        return output;
    }
};

////////////////////////////////////////////////////////////
/// \brief Specialization of the Utf template for UTF-16
///
////////////////////////////////////////////////////////////
template <>
class Utf<16>
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Decode a single UTF-16 character
    ///
    /// Decoding a character means finding its unique 32-bits
    /// code (called the codepoint) in the Unicode standard.
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Codepoint of the decoded UTF-16 character
    /// \param replacement Replacement character to use in case the UTF-8 sequence is invalid
    ///
    /// \return Iterator pointing to one past the last read element of the input sequence
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    static In decode(In begin, In end, char32_t& output, char32_t replacement)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char16_t));

        const char16_t first = *begin++;

        // If it's a surrogate pair, first convert to a single UTF-32 character
        if ((first >= 0xD8'00) && (first <= 0xDB'FF))
        {
            if (begin != end)
            {
                const base::U32 second = *begin++;
                if ((second >= 0xDC'00) && (second <= 0xDF'FF))
                {
                    // The second element is valid: convert the two elements to a UTF-32 character
                    output = ((first - 0xD8'00u) << 10) + (second - 0xDC'00) + 0x0'01'00'00;
                }
                else
                {
                    // Invalid character
                    output = replacement;
                }
            }
            else
            {
                // Invalid character
                begin  = end;
                output = replacement;
            }
        }
        else
        {
            // We can make a direct copy
            output = static_cast<char32_t>(first);
        }

        return begin;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Encode a single UTF-16 character
    ///
    /// Encoding a character means converting a unique 32-bits
    /// code (called the codepoint) in the target encoding, UTF-16.
    ///
    /// \param input       Codepoint to encode as UTF-16
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to UTF-16 (use 0 to skip them)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename Out>
    static Out encode(char32_t input, Out output, char16_t replacement)
    {
        if (input <= 0xFF'FF)
        {
            // The character can be copied directly, we just need to check if it's in the valid range
            if ((input >= 0xD8'00) && (input <= 0xDF'FF))
            {
                // Invalid character (this range is reserved)
                if (replacement)
                    *output++ = replacement;
            }
            else
            {
                // Valid character directly convertible to a single UTF-16 character
                *output++ = static_cast<char16_t>(input);
            }
        }
        else if (input > 0x00'10'FF'FF)
        {
            // Invalid character (greater than the maximum Unicode value)
            if (replacement)
                *output++ = replacement;
        }
        else
        {
            // The input character will be converted to two UTF-16 elements
            input -= 0x0'01'00'00;
            *output++ = static_cast<char16_t>((input >> 10) + 0xD8'00);
            *output++ = static_cast<char16_t>((input & 0x3'FFUL) + 0xDC'00);
        }

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Advance to the next UTF-16 character
    ///
    /// This function is necessary for multi-elements encodings, as
    /// a single character may use more than 1 storage element.
    ///
    /// \param begin Iterator pointing to the beginning of the input sequence
    /// \param end   Iterator pointing to the end of the input sequence
    ///
    /// \return Iterator pointing to one past the last read element of the input sequence
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    static In next(In begin, In end)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char16_t));

        char32_t codepoint = 0;
        return decode(begin, end, codepoint, 0);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Count the number of characters of a UTF-16 sequence
    ///
    /// This function is necessary for multi-elements encodings, as
    /// a single character may use more than 1 storage element, thus the
    /// total size can be different from (begin - end).
    ///
    /// \param begin Iterator pointing to the beginning of the input sequence
    /// \param end   Iterator pointing to the end of the input sequence
    ///
    /// \return Iterator pointing to one past the last read element of the input sequence
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    static base::SizeT count(In begin, In end)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char16_t));

        base::SizeT length = 0;
        while (begin != end)
        {
            begin = next(begin, end);
            ++length;
        }

        return length;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert an ANSI characters range to UTF-16
    ///
    /// The current global locale will be used by default, unless you
    /// pass a custom one in the `locale` parameter.
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    /// \param facet Facet to use for conversion
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out, typename Facet>
    static Out fromAnsi(In begin, In end, Out output, const Facet& facet);

    ////////////////////////////////////////////////////////////
    /// \brief Convert a wide characters range to UTF-16
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out fromWide(In begin, In end, Out output);

    ////////////////////////////////////////////////////////////
    /// \brief Convert a latin-1 (ISO-5589-1) characters range to UTF-16
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out fromLatin1(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        // Latin-1 is directly compatible with Unicode encodings,
        // and can thus be treated as (a sub-range of) UTF-32
        return priv::copyBits(begin, end, output);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert an UTF-16 characters range to ANSI characters
    ///
    /// The current global locale will be used by default, unless you
    /// pass a custom one in the `locale` parameter.
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to ANSI (use 0 to skip them)
    /// \param facet       Facet to use for conversion
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out, typename Facet>
    static Out toAnsi(In begin, In end, Out output, char replacement, const Facet& facet);

    ////////////////////////////////////////////////////////////
    /// \brief Convert an UTF-16 characters range to wide characters
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to wide (use 0 to skip them)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toWide(In begin, In end, Out output, wchar_t replacement);

    ////////////////////////////////////////////////////////////
    /// \brief Convert an UTF-16 characters range to latin-1 (ISO-5589-1) characters
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to wide (use 0 to skip them)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toLatin1(In begin, In end, Out output, char replacement)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char16_t));

        // Latin-1 is directly compatible with Unicode encodings,
        // and can thus be treated as (a sub-range of) UTF-32
        while (begin != end)
        {
            *output++ = *begin < 256 ? static_cast<char>(*begin) : replacement;
            ++begin;
        }

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert a UTF-16 characters range to UTF-8
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toUtf8(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char16_t));

        while (begin != end)
        {
            char32_t codepoint = 0;
            begin              = decode(begin, end, codepoint, 0);
            output             = Utf<8>::encode(codepoint, output, 0);
        }

        return output;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert a UTF-16 characters range to UTF-16
    ///
    /// This functions does nothing more than a direct copy;
    /// it is defined only to provide the same interface as other
    /// specializations of the `sf::Utf<>` template, and allow
    /// generic code to be written on top of it.
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toUtf16(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char16_t));

        return priv::copyBits(begin, end, output);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert a UTF-16 characters range to UTF-32
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toUtf32(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char16_t));

        while (begin != end)
        {
            char32_t codepoint = 0;
            begin              = decode(begin, end, codepoint, 0);
            *output++          = codepoint;
        }

        return output;
    }
};

////////////////////////////////////////////////////////////
/// \brief Specialization of the Utf template for UTF-32
///
////////////////////////////////////////////////////////////
template <>
class Utf<32>
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Decode a single UTF-32 character
    ///
    /// Decoding a character means finding its unique 32-bits
    /// code (called the codepoint) in the Unicode standard.
    /// For UTF-32, the character value is the same as the codepoint.
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Codepoint of the decoded UTF-32 character
    /// \param replacement Replacement character to use in case the UTF-8 sequence is invalid
    ///
    /// \return Iterator pointing to one past the last read element of the input sequence
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    static In decode(In begin, In end, char32_t& output, char32_t replacement)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char32_t));

        output = *begin++;
        return begin;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Encode a single UTF-32 character
    ///
    /// Encoding a character means converting a unique 32-bits
    /// code (called the codepoint) in the target encoding, UTF-32.
    /// For UTF-32, the codepoint is the same as the character value.
    ///
    /// \param input       Codepoint to encode as UTF-32
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to UTF-32 (use 0 to skip them)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename Out>
    static Out encode(char32_t input, Out output, char32_t replacement)
    {
        *output++ = input;
        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Advance to the next UTF-32 character
    ///
    /// This function is trivial for UTF-32, which can store
    /// every character in a single storage element.
    ///
    /// \param begin Iterator pointing to the beginning of the input sequence
    /// \param end   Iterator pointing to the end of the input sequence
    ///
    /// \return Iterator pointing to one past the last read element of the input sequence
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    static In next(In begin, In end)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char32_t));

        return ++begin;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Count the number of characters of a UTF-32 sequence
    ///
    /// This function is trivial for UTF-32, which can store
    /// every character in a single storage element.
    ///
    /// \param begin Iterator pointing to the beginning of the input sequence
    /// \param end   Iterator pointing to the end of the input sequence
    ///
    /// \return Iterator pointing to one past the last read element of the input sequence
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    [[nodiscard]] static base::SizeT count(In begin, In end)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char32_t));

        return static_cast<base::SizeT>(end - begin);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert an ANSI characters range to UTF-32
    ///
    /// The current global locale will be used by default, unless you
    /// pass a custom one in the `locale` parameter.
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    /// \param facet Facet to use for conversion
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out, typename Facet>
    static Out fromAnsi(In begin, In end, Out output, const Facet& facet)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        while (begin != end)
            *output++ = decodeAnsi(*begin++, facet);

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert a wide characters range to UTF-32
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out fromWide(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(wchar_t));

        while (begin != end)
            *output++ = decodeWide(*begin++);

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert a latin-1 (ISO-5589-1) characters range to UTF-32
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out fromLatin1(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char));

        // Latin-1 is directly compatible with Unicode encodings,
        // and can thus be treated as (a sub-range of) UTF-32
        return priv::copyBits(begin, end, output);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert an UTF-32 characters range to ANSI characters
    ///
    /// The current global locale will be used by default, unless you
    /// pass a custom one in the `locale` parameter.
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to ANSI (use 0 to skip them)
    /// \param facet       Facet to use for conversion
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out, typename Facet>
    static Out toAnsi(In begin, In end, Out output, char replacement, const Facet& facet)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char32_t));

        while (begin != end)
            output = encodeAnsi(*begin++, output, replacement, facet);

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert an UTF-32 characters range to wide characters
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to wide (use 0 to skip them)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toWide(In begin, In end, Out output, wchar_t replacement)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char32_t));

        while (begin != end)
            output = encodeWide(*begin++, output, replacement);

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert an UTF-16 characters range to latin-1 (ISO-5589-1) characters
    ///
    /// \param begin       Iterator pointing to the beginning of the input sequence
    /// \param end         Iterator pointing to the end of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement for characters not convertible to wide (use 0 to skip them)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toLatin1(In begin, In end, Out output, char replacement)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char32_t));

        // Latin-1 is directly compatible with Unicode encodings,
        // and can thus be treated as (a sub-range of) UTF-32
        while (begin != end)
        {
            *output++ = *begin < 256 ? static_cast<char>(*begin) : replacement;
            ++begin;
        }

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert a UTF-32 characters range to UTF-8
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toUtf8(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char32_t));

        while (begin != end)
            output = Utf<8>::encode(*begin++, output, 0);

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert a UTF-32 characters range to UTF-16
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toUtf16(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char32_t));

        while (begin != end)
            output = Utf<16>::encode(*begin++, output, 0);

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Convert a UTF-32 characters range to UTF-32
    ///
    /// This functions does nothing more than a direct copy;
    /// it is defined only to provide the same interface as other
    /// specializations of the `sf::Utf<>` template, and allow
    /// generic code to be written on top of it.
    ///
    /// \param begin  Iterator pointing to the beginning of the input sequence
    /// \param end    Iterator pointing to the end of the input sequence
    /// \param output Iterator pointing to the beginning of the output sequence
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Out>
    static Out toUtf32(In begin, In end, Out output)
    {
        static_assert(sizeof(decltype(*begin)) == sizeof(char32_t));

        return priv::copyBits(begin, end, output);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Decode a single ANSI character to UTF-32
    ///
    /// This function does not exist in other specializations
    /// of `sf::Utf<>`, it is defined for convenience (it is used by
    /// several other conversion functions).
    ///
    /// \param input  Input ANSI character
    /// \param facet Facet to use for conversion
    ///
    /// \return Converted character
    ///
    ////////////////////////////////////////////////////////////
    template <typename In, typename Facet>
    [[nodiscard]] static char32_t decodeAnsi(In input, const Facet& facet)
    {
        // Use the facet to convert each character of the input string
        return static_cast<char32_t>(facet.widen(input));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Decode a single wide character to UTF-32
    ///
    /// This function does not exist in other specializations
    /// of `sf::Utf<>`, it is defined for convenience (it is used by
    /// several other conversion functions).
    ///
    /// \param input Input wide character
    ///
    /// \return Converted character
    ///
    ////////////////////////////////////////////////////////////
    template <typename In>
    [[nodiscard]] static char32_t decodeWide(In input)
    {
        // The encoding of wide characters is not well defined and is left to the system;
        // however we can safely assume that it is UCS-2 on Windows and
        // UCS-4 on Unix systems.
        // In both cases, a simple copy is enough (UCS-2 is a subset of UCS-4,
        // and UCS-4 *is* UTF-32).

        return static_cast<char32_t>(input);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Encode a single UTF-32 character to ANSI
    ///
    /// This function does not exist in other specializations
    /// of `sf::Utf<>`, it is defined for convenience (it is used by
    /// several other conversion functions).
    ///
    /// \param codepoint   Iterator pointing to the beginning of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement if the input character is not convertible to ANSI (use 0 to skip it)
    /// \param facet       Facet to use for conversion
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename Out, typename Facet>
    static Out encodeAnsi(char32_t codepoint, Out output, char replacement, const Facet& facet)
    {
        // Use the facet to convert each character of the input string
        *output++ = facet.narrow(static_cast<wchar_t>(codepoint), replacement);

        return output;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Encode a single UTF-32 character to wide
    ///
    /// This function does not exist in other specializations
    /// of `sf::Utf<>`, it is defined for convenience (it is used by
    /// several other conversion functions).
    ///
    /// \param codepoint   Iterator pointing to the beginning of the input sequence
    /// \param output      Iterator pointing to the beginning of the output sequence
    /// \param replacement Replacement if the input character is not convertible to wide (use 0 to skip it)
    ///
    /// \return Iterator to the end of the output sequence which has been written
    ///
    ////////////////////////////////////////////////////////////
    template <typename Out>
    static Out encodeWide(char32_t codepoint, Out output, wchar_t replacement)
    {
        // The encoding of wide characters is not well defined and is left to the system;
        // however we can safely assume that it is UCS-2 on Windows and
        // UCS-4 on Unix systems.
        // For UCS-2 we need to check if the source characters fits in (UCS-2 is a subset of UCS-4).
        // For UCS-4 we can do a direct copy (UCS-4 *is* UTF-32).

        if constexpr (sizeof(wchar_t) == 4)
        {
            *output++ = static_cast<wchar_t>(codepoint);
        }
        else
        {
            if ((codepoint <= 0xFF'FF) && ((codepoint < 0xD8'00) || (codepoint > 0xDF'FF)))
            {
                *output++ = static_cast<wchar_t>(codepoint);
            }
            else if (replacement)
            {
                *output++ = replacement;
            }
        }

        return output;
    }
};


////////////////////////////////////////////////////////////
template <typename In, typename Out, typename Facet>
Out Utf<8>::fromAnsi(In begin, In end, Out output, const Facet& facet)
{
    static_assert(sizeof(decltype(*begin)) == sizeof(char));

    while (begin != end)
    {
        const char32_t codepoint = Utf<32>::decodeAnsi(*begin++, facet);
        output                   = encode(codepoint, output, 0);
    }

    return output;
}


////////////////////////////////////////////////////////////
template <typename In, typename Out>
Out Utf<8>::fromWide(In begin, In end, Out output)
{
    static_assert(sizeof(decltype(*begin)) == sizeof(wchar_t));

    while (begin != end)
    {
        const char32_t codepoint = Utf<32>::decodeWide(*begin++);
        output                   = encode(codepoint, output, 0);
    }

    return output;
}


////////////////////////////////////////////////////////////
template <typename In, typename Out, typename Facet>
Out Utf<8>::toAnsi(In begin, In end, Out output, char replacement, const Facet& facet)
{
    static_assert(sizeof(decltype(*begin)) == sizeof(char));

    while (begin != end)
    {
        char32_t codepoint = 0;
        begin              = decode(begin, end, codepoint, 0);
        output             = Utf<32>::encodeAnsi(codepoint, output, replacement, facet);
    }

    return output;
}


////////////////////////////////////////////////////////////
template <typename In, typename Out>
Out Utf<8>::toWide(In begin, In end, Out output, wchar_t replacement)
{
    static_assert(sizeof(decltype(*begin)) == sizeof(char));

    while (begin != end)
    {
        char32_t codepoint = 0;
        begin              = decode(begin, end, codepoint, 0);
        output             = Utf<32>::encodeWide(codepoint, output, replacement);
    }

    return output;
}


////////////////////////////////////////////////////////////
template <typename In, typename Out>
Out Utf<8>::toUtf16(In begin, In end, Out output)
{
    static_assert(sizeof(decltype(*begin)) == sizeof(char));

    while (begin != end)
    {
        char32_t codepoint = 0;
        begin              = decode(begin, end, codepoint, 0);
        output             = Utf<16>::encode(codepoint, output, 0);
    }

    return output;
}

////////////////////////////////////////////////////////////
template <typename In, typename Out, typename Facet>
Out Utf<16>::fromAnsi(In begin, In end, Out output, const Facet& facet)
{
    static_assert(sizeof(decltype(*begin)) == sizeof(char));

    while (begin != end)
    {
        const char32_t codepoint = Utf<32>::decodeAnsi(*begin++, facet);
        output                   = encode(codepoint, output, 0);
    }

    return output;
}


////////////////////////////////////////////////////////////
template <typename In, typename Out>
Out Utf<16>::fromWide(In begin, In end, Out output)
{
    static_assert(sizeof(decltype(*begin)) == sizeof(wchar_t));

    while (begin != end)
    {
        const char32_t codepoint = Utf<32>::decodeWide(*begin++);
        output                   = encode(codepoint, output, 0);
    }

    return output;
}


////////////////////////////////////////////////////////////
template <typename In, typename Out, typename Facet>
Out Utf<16>::toAnsi(In begin, In end, Out output, char replacement, const Facet& facet)
{
    static_assert(sizeof(decltype(*begin)) == sizeof(char16_t));

    while (begin != end)
    {
        char32_t codepoint = 0;
        begin              = decode(begin, end, codepoint, 0);
        output             = Utf<32>::encodeAnsi(codepoint, output, replacement, facet);
    }

    return output;
}


////////////////////////////////////////////////////////////
template <typename In, typename Out>
Out Utf<16>::toWide(In begin, In end, Out output, wchar_t replacement)
{
    static_assert(sizeof(decltype(*begin)) == sizeof(char16_t));

    while (begin != end)
    {
        char32_t codepoint = 0;
        begin              = decode(begin, end, codepoint, 0);
        output             = Utf<32>::encodeWide(codepoint, output, replacement);
    }

    return output;
}


////////////////////////////////////////////////////////////
// Make type aliases to get rid of the template syntax
using Utf8  = Utf<8>;
using Utf16 = Utf<16>;
using Utf32 = Utf<32>;

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Utf
/// \ingroup system
///
/// Utility class providing generic functions for UTF conversions.
///
/// `sf::Utf` is a low-level, generic interface for counting, iterating,
/// encoding and decoding Unicode characters and strings. It is able
/// to handle ANSI, wide, latin-1, UTF-8, UTF-16 and UTF-32 encodings.
///
/// `sf::Utf<X>` functions are all static, these classes are not meant to
/// be instantiated. All the functions are template, so that you
/// can use any character / string type for a given encoding.
///
/// It has 3 specializations:
/// \li `sf::Utf<8>` (with `sf::Utf8` type alias)
/// \li `sf::Utf<16>` (with `sf::Utf16` type alias)
/// \li `sf::Utf<32>` (with `sf::Utf32` type alias)
///
////////////////////////////////////////////////////////////
