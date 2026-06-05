#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace za
{
//////////////////////////////////////////////////////////////
/// \brief Outcome of a `fromChars` parse attempt
///
//////////////////////////////////////////////////////////////
enum class [[nodiscard]] FromCharsError
{
    None,            //!< Successful parse
    InvalidArgument, //!< No digits found, or sign without digits
    ResultOutOfRange //!< Value would overflow the destination type
};


//////////////////////////////////////////////////////////////
/// \brief Result type returned from `fromChars`, mirroring `std::from_chars_result`
///
//////////////////////////////////////////////////////////////
struct [[nodiscard]] FromCharsResult
{
    const char*    ptr; //!< One past the last character successfully consumed
    FromCharsError ec;  //!< Error code (`None` on success)
};

} // namespace za
