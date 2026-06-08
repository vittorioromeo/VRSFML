#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Immediately terminate the current process abnormally
///
/// This function calls `std::abort()`. It is used internally
/// by Zancle for irrecoverable errors, such as failed assertions
/// in debug mode.
///
////////////////////////////////////////////////////////////
[[noreturn, gnu::cold, gnu::noinline]] void abort() noexcept;

} // namespace za
