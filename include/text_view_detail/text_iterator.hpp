// Copyright (c) 2016, Tom Honermann
//
// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.

#ifndef TEXT_VIEW_TEXT_ITERATOR_HPP // {
#define TEXT_VIEW_TEXT_ITERATOR_HPP


#include <text_view_detail/adl_customization.hpp>
#include <text_view_detail/concepts.hpp>
#include <iterator>
#include <origin/core/traits.hpp>


namespace std {
namespace experimental {
inline namespace text {


namespace text_detail {
/*
 * Decoder concept       Code unit iterator concept Text iterator category
 * -------------------   -------------------------- ---------------------------
 * Decoder               Input_iterator             input_iterator_tag
 * Decoder               Forward_iterator           forward_iterator_tag
 * Decoder               Bidirectional_iterator     forward_iterator_tag
 * Decoder               Random_access_iterator     forward_iterator_tag
 * Bidirectional_decoder Input_iterator             input_iterator_tag
 * Bidirectional_decoder Forward_iterator           forward_iterator_tag
 * Bidirectional_decoder Bidirectional_iterator     bidirectional_iterator_tag
 * Bidirectional_decoder Random_access_iterator     bidirectional_iterator_tag
 * Random_access_decoder Input_iterator             input_iterator_tag
 * Random_access_decoder Forward_iterator           forward_iterator_tag
 * Random_access_decoder Bidirectional_iterator     bidirectional_iterator_tag
 * Random_access_decoder Random_access_iterator     random_access_iterator_tag
 */
template<typename TextEncoding, typename Iterator>
struct itext_iterator_category_selector;

template<TextEncoding ET, CodeUnitIterator CUIT>
struct itext_iterator_category_selector<ET, CUIT> {
    using type = origin::Iterator_category<CUIT>;
};
template<TextEncoding ET, CodeUnitIterator CUIT>
requires origin::Bidirectional_iterator<CUIT>() // or Random_access_iterator
      && ! TextBidirectionalDecoder<ET, CUIT>() // and ! TextRandomAccessDecoder
struct itext_iterator_category_selector<ET, CUIT> {
    using type = std::forward_iterator_tag;
};
template<TextEncoding ET, CodeUnitIterator CUIT>
requires origin::Random_access_iterator<CUIT>()
      && TextBidirectionalDecoder<ET, CUIT>()
      && ! TextRandomAccessDecoder<ET, CUIT>()
struct itext_iterator_category_selector<ET, CUIT> {
    using type = std::bidirectional_iterator_tag;
};


template<TextEncoding ET, origin::Input_range RT>
class itext_iterator_base
    : protected ET::state_type
{
public:
    using encoding_type = ET;
    using range_type = std::remove_reference_t<RT>;
    using state_type = typename encoding_type::state_type;
    using iterator = origin::Iterator_type<const range_type>;
    using iterator_category =
              typename text_detail::itext_iterator_category_selector<
                  encoding_type,
                  iterator>::type;
    using value_type = character_type_t<encoding_type>;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type = origin::Difference_type<iterator>;

protected:
    itext_iterator_base() = default;

    itext_iterator_base(const state_type &state)
        // CWG DR1467.  List-initialization doesn't consider copy constructors
        // for aggregates.  The state_type base class must be initialized with
        // an expression-list.
        : state_type(state) {}

public:
    const state_type& state() const noexcept {
        return *this;
    }
    state_type& state() noexcept {
        return *this;
    }
};

template<TextEncoding ET, origin::Input_range RT>
requires TextDecoder<ET, origin::Iterator_type<const RT>>()
      && origin::Input_iterator<origin::Iterator_type<const RT>>()
class itext_iterator_data
    : public itext_iterator_base<ET, RT>
{
public:
    using state_type = typename itext_iterator_data::state_type;
    using range_type = typename itext_iterator_data::range_type;
    using iterator = typename itext_iterator_data::iterator;

protected:
    itext_iterator_data() = default;

    itext_iterator_data(
        const state_type &state,
        const range_type *range,
        iterator current)
    :
        itext_iterator_base<ET, RT>{state},
        range{range},
        current{current}
    {}

public:
    iterator base() const {
        return current;
    }

protected:
    const range_type *range;
    iterator current;
};

template<TextEncoding ET, origin::Input_range RT>
requires TextDecoder<ET, origin::Iterator_type<const RT>>()
      && origin::Forward_iterator<origin::Iterator_type<const RT>>()
class itext_iterator_data<ET, RT>
    : public itext_iterator_base<ET, RT>
{
public:
    using state_type = typename itext_iterator_data::state_type;
    using range_type = typename itext_iterator_data::range_type;
    using iterator = typename itext_iterator_data::iterator;

protected:
    itext_iterator_data() = default;

    itext_iterator_data(
        const state_type &state,
        const range_type *range,
        iterator first)
    :
        itext_iterator_base<ET, RT>{state},
        range{range},
        current_range{first, first}
    {}

    struct current_range_type {
        current_range_type()
            : first{}, last{} {}
        current_range_type(iterator first, iterator last)
            : first{first}, last{last} {}

        iterator begin() const { return first; }
        iterator end() const { return last; }

        iterator first;
        iterator last;
    };

public:
    iterator base() const {
        return current_range.first;
    }

    const current_range_type& base_range() const {
        return current_range;
    }

protected:
    const range_type *range;
    current_range_type current_range;
};

} // namespace text_detail


template<TextEncoding ET, origin::Input_range RT>
requires TextDecoder<ET, origin::Iterator_type<const RT>>()
class itext_iterator
    : public text_detail::itext_iterator_data<ET, RT>
{
public:
    using encoding_type = typename itext_iterator::encoding_type;
    using range_type = typename itext_iterator::range_type;
    using state_type = typename itext_iterator::state_type;
    using value_type = typename itext_iterator::value_type;
    using iterator_category = typename itext_iterator::iterator_category;
    using iterator = typename itext_iterator::iterator;
    using pointer = typename itext_iterator::pointer;
    using reference = typename itext_iterator::reference;
    using difference_type = typename itext_iterator::difference_type;

public:
    itext_iterator() = default;

    itext_iterator(
        const state_type &state,
        const range_type *range,
        iterator first)
    :
        text_detail::itext_iterator_data<ET, RT>{state, range, first}
    {
        ++*this;
    }

    // Iterators that are not ok include:
    // - Singular iterators.
    // - Past the end iterators.
    // - Iterators for which a decoding error occurred during increment or
    //   decrement operations.
    bool is_ok() const noexcept {
        return ok;
    }

    reference operator*() const noexcept {
        return value;
    }
    pointer operator->() const noexcept {
        return &value;
    }

    friend bool operator==(
        const itext_iterator &l,
        const itext_iterator &r)
    {
        return l.equal(r);
    }
    friend bool operator!=(
        const itext_iterator &l,
        const itext_iterator &r)
    {
        return !(l == r);
    }

    friend bool operator<(
        const itext_iterator &l,
        const itext_iterator &r)
    requires TextRandomAccessDecoder<
        encoding_type,
        iterator>()
    {
        return (r - l) > 0;
    }
    friend bool operator>(
        const itext_iterator &l,
        const itext_iterator &r)
    requires TextRandomAccessDecoder<
        encoding_type,
        iterator>()
    {
        return r < l;
    }
    friend bool operator<=(
        const itext_iterator &l,
        const itext_iterator &r)
    requires TextRandomAccessDecoder<
        encoding_type,
        iterator>()
    {
        return !(r < l);
    }
    friend bool operator>=(
        const itext_iterator &l,
        const itext_iterator &r)
    requires TextRandomAccessDecoder<
        encoding_type,
        iterator>()
    {
        return !(l < r);
    }

    itext_iterator& operator++() {
        ok = false;
        iterator tmp_iterator{this->current};
        auto end(text_detail::adl_end(*this->range));
        while (tmp_iterator != end) {
            value_type tmp_value;
            int decoded_code_units = 0;
            bool decoded_code_point = encoding_type::decode(
                this->state(),
                tmp_iterator,
                end,
                tmp_value,
                decoded_code_units);
            this->current = tmp_iterator;
            if (decoded_code_point) {
                value = tmp_value;
                ok = true;
                break;
            }
        }
        return *this;
    }

    itext_iterator& operator++()
        requires TextForwardDecoder<encoding_type, iterator>()
    {
        ok = false;
        this->current_range.first = this->current_range.last;
        iterator tmp_iterator{this->current_range.first};
        auto end(text_detail::adl_end(*this->range));
        while (tmp_iterator != end) {
            value_type tmp_value;
            int decoded_code_units = 0;
            bool decoded_code_point = encoding_type::decode(
                this->state(),
                tmp_iterator,
                end,
                tmp_value,
                decoded_code_units);
            this->current_range.last = tmp_iterator;
            if (decoded_code_point) {
                value = tmp_value;
                ok = true;
                break;
            }
            this->current_range.first = this->current_range.last;
        }
        return *this;
    }

    itext_iterator operator++(int) {
        itext_iterator it{*this};
        ++*this;
        return it;
    }

    itext_iterator& operator--()
        requires TextBidirectionalDecoder<encoding_type, iterator>()
    {
        ok = false;
        this->current_range.last = this->current_range.first;
        std::reverse_iterator<iterator> rcurrent{this->current_range.last};
        std::reverse_iterator<iterator> rend{text_detail::adl_begin(*this->range)};
        while (rcurrent != rend) {
            value_type tmp_value;
            int decoded_code_units = 0;
            bool decoded_code_point = encoding_type::rdecode(
                this->state(),
                rcurrent,
                rend,
                tmp_value,
                decoded_code_units);
            this->current_range.first = rcurrent.base();
            if (decoded_code_point) {
                value = tmp_value;
                ok = true;
                break;
            }
            this->current_range.last = this->current_range.first;
        }
        return *this;
    }

    itext_iterator operator--(int)
        requires TextBidirectionalDecoder<encoding_type, iterator>()
    {
        itext_iterator it{*this};
        --*this;
        return it;
    }

    itext_iterator& operator+=(difference_type n)
        requires TextRandomAccessDecoder<encoding_type, iterator>()
    {
        if (n < 0) {
            this->current_range.first +=
                ((n+1) * encoding_type::max_code_units);
            --*this;
        } else if (n > 0) {
            this->current_range.last +=
                ((n-1) * encoding_type::max_code_units);
            ++*this;
        }
        return *this;
    }

    friend itext_iterator operator+(
        itext_iterator l,
        difference_type n)
    requires TextRandomAccessDecoder<encoding_type, iterator>()
    {
        return l += n;
    }

    friend itext_iterator operator+(
        difference_type n,
        itext_iterator r)
    requires TextRandomAccessDecoder<encoding_type, iterator>()
    {
        return r += n;
    }

    itext_iterator& operator-=(difference_type n)
        requires TextRandomAccessDecoder<encoding_type, iterator>()
    {
        return *this += -n;
    }

    friend itext_iterator operator-(
        itext_iterator l,
        difference_type n)
    requires TextRandomAccessDecoder<encoding_type, iterator>()
    {
        return l -= n;
    }

    friend difference_type operator-(
        const itext_iterator &l,
        const itext_iterator &r)
    requires TextRandomAccessDecoder<encoding_type, iterator>()
    {
        return (l.current_range.first - r.current_range.first) /
               encoding_type::max_code_units;
    }

    // Random access iterator requirements state that operator[] must return
    // a reference.  That isn't possible here since the reference would be to
    // a value stored in an object (The itext_iterator instance produced for
    // '*this + n') that is destroyed before the function returns.
    value_type operator[](
        difference_type n) const
    requires TextRandomAccessDecoder<encoding_type, iterator>()
    {
        return *(*this + n);
    }

private:
    bool equal(const itext_iterator &other) const {
        // For input iterators, the base iterator corresponds to the next input
        // to be decoded.  Naively checking for base comparison only therefore
        // results in premature matches when the last code point in the input
        // is consumed.  For this reason, base equality is only considered a
        // match if the iterator is not ok.  It is assumed that the reason the
        // base iterator is not ok is that an attempt was made to decode a
        // code point after the last one in the input.  This heuristic can be
        // defeated when an iterator is not ok for other reasons.
        return ok == other.ok
            && (!ok || this->base() == other.base());
    }
    bool equal(const itext_iterator &other) const
        requires origin::Forward_iterator<origin::Iterator_type<const RT>>()
    {
        return this->base() == other.base();
    }

private:
    value_type value = {};
    bool ok = false;
};


template<TextEncoding ET, origin::Input_range RT>
class itext_sentinel {
public:
    using range_type = std::remove_reference_t<RT>;
    using sentinel = origin::Sentinel_type<RT>;

    itext_sentinel(sentinel s)
        : s{s} {}

    // This constructor is provided in lieu of a specialization of
    // std::common_type in order to satisfy the common type requirement for
    // cross-type equality comparison.  See N4382 19.2.5 "Concept Common",
    // N4382 19.3.2 "Concept EqualityComparable", and N3351 3.3
    // "Foundational Concepts".
    itext_sentinel(const itext_iterator<ET, RT> &ti)
    requires origin::Convertible<
                 decltype(ti.base()),
                 sentinel>()
    :
        s{ti.base()}
    {}

    friend bool operator==(
        const itext_sentinel &l,
        const itext_sentinel &r)
    {
        // Sentinels always compare equal regardless of any internal state.
        // See N4128, 10.1 "Sentinel Equality".
        return true;
    }
    friend bool operator!=(
        const itext_sentinel &l,
        const itext_sentinel &r)
    {
        return !(l == r);
    }

    friend bool operator==(
        const itext_iterator<ET, RT> &ti,
        const itext_sentinel &ts)
    {
        return ts.equal(ti);
    }
    friend bool operator!=(
        const itext_iterator<ET, RT> &ti,
        const itext_sentinel &ts)
    {
        return !(ti == ts);
    }
    friend bool operator==(
        const itext_sentinel &ts,
        const itext_iterator<ET, RT> &ti)
    {
        return ti == ts;
    }
    friend bool operator!=(
        const itext_sentinel &ts,
        const itext_iterator<ET, RT> &ti)
    {
        return !(ts == ti);
    }

    friend bool operator<(
        const itext_sentinel &l,
        const itext_sentinel &r)
    {
        // Sentinels always compare equal regardless of any internal state.
        // See N4128, 10.1 "Sentinel Equality".
        return false;
    }
    friend bool operator>(
        const itext_sentinel &l,
        const itext_sentinel &r)
    {
        return r < l;
    }
    friend bool operator<=(
        const itext_sentinel &l,
        const itext_sentinel &r)
    {
        return !(r < l);
    }
    friend bool operator>=(
        const itext_sentinel &l,
        const itext_sentinel &r)
    {
        return !(l < r);
    }

    friend bool operator<(
        const itext_iterator<ET, RT> &ti,
        const itext_sentinel &ts)
    requires origin::Weakly_ordered<
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>()
    {
        return ti.base() < ts.base();
    }
    friend bool operator>(
        const itext_iterator<ET, RT> &ti,
        const itext_sentinel &ts)
    requires origin::Weakly_ordered<
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>()
    {
        return ti.base() > ts.base();
    }
    friend bool operator<=(
        const itext_iterator<ET, RT> &ti,
        const itext_sentinel &ts)
    requires origin::Weakly_ordered<
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>()
    {
        return ti.base() <= ts.base();
    }
    friend bool operator>=(
        const itext_iterator<ET, RT> &ti,
        const itext_sentinel &ts)
    requires origin::Weakly_ordered<
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>()
    {
        return ti.base() >= ts.base();
    }

    friend bool operator<(
        const itext_sentinel &ts,
        const itext_iterator<ET, RT> &ti)
    requires origin::Weakly_ordered<
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>()
    {
        return ts.base() < ti.base();
    }
    friend bool operator>(
        const itext_sentinel &ts,
        const itext_iterator<ET, RT> &ti)
    requires origin::Weakly_ordered<
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>()
    {
        return ts.base() > ti.base();
    }
    friend bool operator<=(
        const itext_sentinel &ts,
        const itext_iterator<ET, RT> &ti)
    requires origin::Weakly_ordered<
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>()
    {
        return ts.base() <= ti.base();
    }
    friend bool operator>=(
        const itext_sentinel &ts,
        const itext_iterator<ET, RT> &ti)
    requires origin::Weakly_ordered<
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>()
    {
        return ts.base() >= ti.base();
    }

    sentinel base() const {
        return s;
    }

private:
    bool equal(const itext_iterator<ET, RT> &ti) const {
        // For input iterators, the base iterator corresponds to the next input
        // to be decoded.  Naively checking for base comparison only therefore
        // results in premature matches when the last code point in the input
        // is consumed.  For this reason, base equality is only considered a
        // sentinel match if the iterator is not ok.  It is assumed that the
        // reason the base iterator is not ok is that an attempt was made to
        // decode a code point after the last one in the input.  This heuristic
        // can be defeated when an iterator is not ok for other reasons.
        return ti.base() == base()
            && ! ti.is_ok();
    }
    bool equal(const itext_iterator<ET, RT> &ti) const
        requires origin::Forward_iterator<origin::Iterator_type<const RT>>()
    {
        return ti.base() == base();
    }

private:
    sentinel s;
};


template<TextEncoding E, CodeUnitOutputIterator<code_unit_type_t<E>> CUIT>
class otext_iterator
    : private E::state_type
{
public:
    using encoding_type = E;
    using state_type = typename E::state_type;
    using state_transition_type = typename E::state_transition_type;
    using iterator = CUIT;
    using iterator_category = std::output_iterator_tag;
    using value_type = character_type_t<encoding_type>;
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = origin::Difference_type<iterator>;

    otext_iterator() = default;

    otext_iterator(
        state_type state,
        iterator current)
    :
        // CWG DR1467.  List-initialization doesn't consider copy constructors
        // for aggregates.  The state_type base class must be initialized with
        // an expression-list.
        state_type(state),
        current{current}
    {}

    const state_type& state() const noexcept {
        return *this;
    }
    state_type& state() noexcept {
        return *this;
    }

    iterator base() const {
        return current;
    }

    otext_iterator& operator*() {
        return *this;
    }

    otext_iterator& operator++() {
        return *this;
    }
    otext_iterator& operator++(int) {
        return *this;
    }

    otext_iterator& operator=(
        const state_transition_type &stt)
    {
        iterator tmp{current};
        int encoded_code_units = 0;
        encoding_type::encode_state_transition(state(), tmp, stt,
                                               encoded_code_units);
        current = tmp;
        return *this;
    }

    otext_iterator& operator=(
        const character_type_t<encoding_type> &value)
    {
        iterator tmp{current};
        int encoded_code_units = 0;
        encoding_type::encode(state(), tmp, value, encoded_code_units);
        current = tmp;
        return *this;
    }

protected:
    iterator current;
};


/*
 * make_otext_iterator
 */
// Overload to construct an output text iterator from an output iterator and
// and an explicitly specified initial encoding state.
template<TextEncoding ET, CodeUnitOutputIterator<code_unit_type_t<ET>> IT>
auto make_otext_iterator(
    typename ET::state_type state,
    IT out)
{
    return otext_iterator<ET, IT>{state, out};
}

// Overload to construct an output text iterator from an output iterator and
// and an implicit initial encoding state.
template<TextEncoding ET, CodeUnitOutputIterator<code_unit_type_t<ET>> IT>
auto make_otext_iterator(
    IT out)
{
    return otext_iterator<ET, IT>{ET::initial_state(), out};
}


} // inline namespace text
} // namespace experimental
} // namespace std


#endif // } TEXT_VIEW_TEXT_ITERATOR_HPP
