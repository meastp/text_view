// Copyright (c) 2016, Tom Honermann
//
// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.

#ifndef TEXT_VIEW_CONCEPTS_HPP // {
#define TEXT_VIEW_CONCEPTS_HPP


#include <origin/core/traits.hpp>
#include <origin/algorithm/concepts.hpp>
#include <origin/range/range.hpp>
#include <text_view_detail/traits.hpp>


namespace std {
namespace experimental {
inline namespace text {


namespace text_detail {
template<typename T, T t1, T t2>
concept bool SameValue() {
    return t1 == t2;
}
} // text_detail namespace


/*
 * Code unit concept
 */
template<typename T>
concept bool CodeUnit() {
    return origin::Integral_type<T>()
        && (origin::Unsigned_type<T>()
            || origin::Same<T, origin::Remove_cv<char>>()
            || origin::Same<T, origin::Remove_cv<wchar_t>>());
}


/*
 * Code point concept
 */
template<typename T>
concept bool CodePoint() {
    return origin::Integral_type<T>()
        && (origin::Unsigned_type<T>()
            || origin::Same<T, origin::Remove_cv<char>>()
            || origin::Same<T, origin::Remove_cv<wchar_t>>());
}


/*
 * Character set concept
 */
template<typename T>
concept bool CharacterSet() {
    return requires () {
               { T::get_name() } noexcept -> const char *;
           }
        && CodePoint<code_point_type_t<T>>();
}


/*
 * Character concept
 */
template<typename T>
concept bool Character() {
    return CharacterSet<character_set_type_t<T>>()
        && origin::Regular<T>()
        && origin::Copyable<T>()
        && requires (T t, code_point_type_t<character_set_type_t<T>> cp) {
               t.set_code_point(cp);
               { t.get_code_point() } -> code_point_type_t<character_set_type_t<T>>;
               { t.get_character_set_id() } -> character_set_id;
           };
}


/*
 * Code unit iterator concept
 */
template<typename T>
concept bool CodeUnitIterator() {
    return origin::Iterator<T>()
        && CodeUnit<origin::Value_type<T>>();
}


/*
 * Code unit output iterator concept
 */
template<typename T, typename V>
concept bool CodeUnitOutputIterator() {
    return origin::Output_iterator<T, V>()
        && CodeUnit<V>();
}


/*
 * Text encoding state concept
 * These requirements are intended to match the char_traits<T>::state_type
 * requirements described in C++11 [char.traits.typedefs] 21.2.2p4.
 */
template<typename T>
concept bool TextEncodingState() {
    return origin::Default_constructible<T>()
        && origin::Copyable<T>();
}


/*
 * Text encoding state transition concept
 */
template<typename T>
concept bool TextEncodingStateTransition() {
    return origin::Default_constructible<T>()
        && origin::Copyable<T>();
}


/*
 * Text encoding concept
 */
template<typename T>
concept bool TextEncoding() {
    return requires () {
               { T::min_code_units } noexcept -> int;
               { T::max_code_units } noexcept -> int;
           }
        && TextEncodingState<typename T::state_type>()
        && TextEncodingStateTransition<typename T::state_transition_type>()
        && CodeUnit<code_unit_type_t<T>>()
        && Character<character_type_t<T>>()
        && requires () {
               { T::initial_state() }
                   -> const typename T::state_type&;
           };
}


/*
 * Text encoder concept
 */
template<typename T, typename CUIT>
concept bool TextEncoder() {
    return TextEncoding<T>()
        && CodeUnitOutputIterator<CUIT, code_unit_type_t<T>>()
        && requires (
               typename T::state_type &state,
               CUIT &out,
               typename T::state_transition_type stt,
               int &encoded_code_units)
           {
               T::encode_state_transition(state, out, stt, encoded_code_units);
           }
        && requires (
               typename T::state_type &state,
               CUIT &out,
               character_type_t<T> c,
               int &encoded_code_units)
           {
               T::encode(state, out, c, encoded_code_units);
           };
}


/*
 * Text decoder concept
 */
template<typename T, typename CUIT>
concept bool TextDecoder() {
    return TextEncoding<T>()
        && origin::Input_iterator<CUIT>()
        && origin::Convertible<origin::Value_type<CUIT>,
                               code_unit_type_t<T>>()
        && requires (
               typename T::state_type &state,
               CUIT &in_next,
               CUIT in_end,
               character_type_t<T> &c,
               int &decoded_code_units)
           {
               { T::decode(state, in_next, in_end, c, decoded_code_units) } -> bool;
           };
}


/*
 * Text forward decoder concept
 */
template<typename T, typename CUIT>
concept bool TextForwardDecoder() {
    return TextDecoder<T, CUIT>()
        && origin::Forward_iterator<CUIT>();
}


/*
 * Text bidirectional decoder concept
 */
template<typename T, typename CUIT>
concept bool TextBidirectionalDecoder() {
    return TextForwardDecoder<T, CUIT>()
        && origin::Bidirectional_iterator<CUIT>()
        && requires (
               typename T::state_type &state,
               CUIT &in_next,
               CUIT in_end,
               character_type_t<T> &c,
               int &decoded_code_units)
           {
               { T::rdecode(state, in_next, in_end, c, decoded_code_units) } -> bool;
           };
}


/*
 * Text random access decoder concept
 */
template<typename T, typename CUIT>
concept bool TextRandomAccessDecoder() {
    return TextBidirectionalDecoder<T, CUIT>()
        && origin::Random_access_iterator<CUIT>()
        && text_detail::SameValue<int, T::min_code_units, T::max_code_units>()
        && std::is_empty<typename T::state_type>::value;
}


/*
 * Text iterator concept
 */
template<typename T>
concept bool TextIterator() {
    return origin::Iterator<T>()
        && Character<origin::Value_type<T>>()
        && TextEncoding<encoding_type_t<T>>()
        && TextEncodingState<typename T::state_type>()
        && requires (T t, const T ct) {
               { t.state() } noexcept
                   -> typename encoding_type_t<T>::state_type&;
               { ct.state() } noexcept
                   -> const typename encoding_type_t<T>::state_type&;
           };
}


/*
 * Text sentinel concept
 */
template<typename T, typename I>
concept bool TextSentinel() {
    return origin::Sentinel<T, I>()
        && TextIterator<I>();
}


/*
 * Text output iterator concept
 */
template<typename T>
concept bool TextOutputIterator() {
    return origin::Output_iterator<T, character_type_t<encoding_type_t<T>>>()
        && TextEncoding<encoding_type_t<T>>()
        && TextEncodingState<typename T::state_type>()
        && requires (T t, const T ct) {
               { t.state() } noexcept
                   -> typename encoding_type_t<T>::state_type&;
               { ct.state() } noexcept
                   -> const typename encoding_type_t<T>::state_type&;
           };
}


/*
 * Text view concept
 */
template<typename T>
concept bool TextView() {
    return origin::Input_range<T>()
        && TextIterator<origin::Iterator_type<T>>()
        && TextEncoding<encoding_type_t<T>>()
        && origin::Input_range<typename T::range_type>()
        && TextEncodingState<typename T::state_type>()
        && CodeUnitIterator<typename T::code_unit_iterator>()
        && requires (T t, const T ct) {
               { t.base() } noexcept
                   -> typename T::range_type&;
               { ct.base() } noexcept
                   -> const typename T::range_type&;
               { t.initial_state() } noexcept
                   -> typename T::state_type&;
               { ct.initial_state() } noexcept
                   -> const typename T::state_type&;
           };
}


} // inline namespace text
} // namespace experimental
} // namespace std


#endif // } TEXT_VIEW_CONCEPTS_HPP
