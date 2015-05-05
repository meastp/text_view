#ifndef TEXT_VIEW_UNICODE_CHARSETS_HPP // {
#define TEXT_VIEW_UNICODE_CHARSETS_HPP


namespace std {
namespace experimental {
namespace text_view {


/*
 * C++ universal character set
 * ISO/IEC 14882:2011(E) 2.3
 */
template<Code_point CPT>
struct unicode_character_set_template {
    using code_point_type = CPT;
};
using unicode_character_set = unicode_character_set_template<char32_t>;


} // namespace text_view
} // namespace experimental
} // namespace std


#endif // } TEXT_VIEW_UNICODE_CHARSETS_HPP
