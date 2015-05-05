#ifndef TEXT_VIEW_STD_ENCODINGS_HPP // {
#define TEXT_VIEW_STD_ENCODINGS_HPP


#include <text_view_detail/encodings/basic_encodings.hpp>
#include <text_view_detail/encodings/unicode_encodings.hpp>


#if !defined(TEXT_VIEW_EXECUTION_CHARACTER_ENCODING)
#define TEXT_VIEW_EXECUTION_CHARACTER_ENCODING \
        ::std::experimental::text_view::basic_execution_character_encoding
#endif
#if !defined(TEXT_VIEW_EXECUTION_WIDE_CHARACTER_ENCODING)
#define TEXT_VIEW_EXECUTION_WIDE_CHARACTER_ENCODING \
        ::std::experimental::text_view::basic_execution_wide_character_encoding
#endif
#if !defined(TEXT_VIEW_CHAR8_CHARACTER_ENCODING)
#define TEXT_VIEW_CHAR8_CHARACTER_ENCODING \
        ::std::experimental::text_view::utf8_encoding
#endif
#if !defined(TEXT_VIEW_CHAR16_CHARACTER_ENCODING)
#define TEXT_VIEW_CHAR16_CHARACTER_ENCODING \
        ::std::experimental::text_view::utf16_encoding
#endif
#if !defined(TEXT_VIEW_CHAR32_CHARACTER_ENCODING)
#define TEXT_VIEW_CHAR32_CHARACTER_ENCODING \
        ::std::experimental::text_view::utf32_encoding
#endif


namespace std {
namespace experimental {
namespace text_view {


/*
 * C++ execution character encoding
 * ISO/IEC 14882:2011(E) 2.14.5 [lex.string]
 */
using execution_character_encoding = TEXT_VIEW_EXECUTION_CHARACTER_ENCODING;


/*
 * C++ execution wide character encoding
 * ISO/IEC 14882:2011(E) 2.14.5 [lex.string]
 */
using execution_wide_character_encoding = TEXT_VIEW_EXECUTION_WIDE_CHARACTER_ENCODING;


/*
 * C++ char (u8) character encoding
 * ISO/IEC 14882:2011(E) 2.14.5 [lex.string]
 */
using char8_character_encoding = TEXT_VIEW_CHAR8_CHARACTER_ENCODING;


/*
 * C++ char16_t (u) character encoding
 * ISO/IEC 14882:2011(E) 2.14.5 [lex.string]
 */
using char16_character_encoding = TEXT_VIEW_CHAR16_CHARACTER_ENCODING;


/*
 * C++ char32_t (U) character encoding
 * ISO/IEC 14882:2011(E) 2.14.5 [lex.string]
 */
using char32_character_encoding = TEXT_VIEW_CHAR32_CHARACTER_ENCODING;


} // namespace text_view
} // namespace experimental
} // namespace std


#endif // } TEXT_VIEW_STD_ENCODINGS_HPP
