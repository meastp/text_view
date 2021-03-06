# [Text_view]
A [C++ Concepts][ISO/IEC 19217:2015]
based character encoding and code point enumeration library.

- [Overview](#overview)
- [Current features and limitations](#current-features-and-limitations)
- [Requirements](#requirements)
- [Build and installation](#build-and-installation)
  - [Building and installing gcc]
    (#building-and-installing-gcc)
  - [Building and installing Origin-text_view]
    (#building-and-installing-origin-text_view)
  - [Building and installing Text_view]
    (#building-and-installing-text_view)
- [Usage](#usage)
  - [Header &lt;text_view&gt; synopsis](#header-text_view-synopsis)
  - [Concepts](#concepts)
  - [Character sets](#character-sets)
  - [Character set identification](#character-set-identification)
  - [Character set information](#character-set-information)
  - [Characters](#characters)
  - [Encodings](#encodings)
  - [Text iterators](#text-iterators)
  - [Text view](#text-view)
- [Supported Encodings](#supported-encodings)
- [Terminology](#terminology)
  - [Code Unit](#code-unit)
  - [Code Point](#code-point)
  - [Character Set](#character-set)
  - [Character](#character)
  - [Encoding](#encoding)
- [References](#references)

# Overview
[C++11][ISO/IEC 14882:2011] added support for new character types ([N2249]) and
[Unicode] string literals ([N2442]), but neither [C++11][ISO/IEC 14882:2011],
nor more recent standards have provided means of efficiently and conveniently
enumerating [code points](#code-point) in [Unicode] or legacy encodings.  While
it is possible to implement such enumeration using interfaces provided in the
standard `<codecvt>` library, doing to is awkward, requires that text be
provided as pointers to contiguous memory, and inefficent due to virtual
function call overhead (__examples and data required to back up these
assertions__).

[Text_view] provides iterator and range based interfaces for encoding and
decoding strings in a variety of [character encodings](#encoding).  The
interface is intended to support all modern and legacy
[character encodings](#encoding), though this library does not yet provide
implementations for legacy [encodings](#encoding).

An example usage follows.  Note that `\u00F8` (LATIN SMALL LETTER O WITH STROKE)
is encoded as UTF-8 using two [code units](#code-unit) (`\xC3\xB8`), but
iterator based enumeration sees just the single [code point](#code-point).

```C++
using CT = utf8_encoding::character_type;
auto tv = make_text_view<utf8_encoding>(u8"J\u00F8erg is my friend");
auto it = tv.begin();
assert(*it++ == CT{0x004A}); // 'J'
assert(*it++ == CT{0x00F8}); // 'ø'
assert(*it++ == CT{0x0065}); // 'e'
```

The iterators and ranges that [Text_view] provides are compatible with the
non-modifying sequence utilities provided by the standard C++ `<algorithm>`
library.  This enables use of standard algorithms to search encoded text.

```C++
it = std::find(tv.begin(), tv.end(), CT{0x00F8});
assert(it != tv.end());
```

The iterators provided by [Text_view] also provide access to the underlying
[code unit](#code-unit) sequence.

```C++
auto base_it = it.base_range().begin();
assert(*base_it++ == '\xC3');
assert(*base_it++ == '\xB8');
assert(base_it == it.base_range().end());
```

[Text_view] ranges satisfy the requirements for use in
[C++11][ISO/IEC 14882:2011] range-based for statements.  This support is
currently limited to views constructed for stateless [encodings](#encoding)
as a sentinel is used as the end iterator for stateful [encodings](#encoding).
The enhancements to the range-based for statement in the
[ranges proposal][N4560] will remove this limitation.

```C++
for (const auto &ch : tv) {
  ...
}
```

# Current features and limitations
[Text_view] provides interfaces for the following:
- Encoding and decoding of text for the [encodings](#encoding) listed in
  [supported encodings](#supported-encodings).
- Encoding text using [C++11][ISO/IEC 14882:2011] compliant output iterators.
- Decoding text using mostly [C++11][ISO/IEC 14882:2011] compliant input,
  forward, bidirectional, and random access iterators.  These iterators lack
  full compliance for the forward, bidirectional, and random access iterator
  requirements for issues that are addressed by [P0022R1].
- Constructing view adapters for encoded text stored in arrays, containers,
  or std::basic_string, or referenced by another range or view.  These view
  adapters meet the requirements for views in the [ranges proposal][N4560].

[Text_view] does **not** currently provide interfaces for the following:
- Transcoding of code points from one [character set](#character-set) to
  another.
- Iterators for grapheme clusters or other boundary conditions.
- Collation.
- Localization.
- Internationalization.
- [Unicode] code point properties.
- [Unicode] normalization.

# Requirements
[Text_view] requires a C++ compiler that implements [ISO/IEC technical
specification 19217:2015, **C++ Extensions for concepts**][ISO/IEC 19217:2015]
As of 2015-12-31, this specification is only supported by the current
in-development release of [gcc] that currently self-identifies itself as [gcc]
version 6.0.0.  Additionally, [Text_view] depends on the [Origin-text_view] fork
of [Andrew Sutton]'s [Origin] library for concept definitions.  The fork is
necessary to work around the following [gcc] defects and [Origin] issues that
are surfaced by [Text_view].

1. https://github.com/asutton/origin/issues/5
2. https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69235
3. https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67565

# Build and installation
This section provides instructions for building [Text_view] and suitable
versions of its dependencies.

## Building and installing [gcc]
[Text_view] is known to build successfully with [gcc] svn revision 232017; the
top of trunk as of 2015-12-31.  The following commands can be used to perform a
suitable build of that [gcc] revision on Linux.

```sh
$ svn co svn://gcc.gnu.org/svn/gcc/trunk gcc-trunk-src
$ curl -O ftp://ftp.gnu.org/gnu/gmp/gmp-5.1.1.tar.bz2
$ curl -O ftp://ftp.gnu.org/gnu/mpfr/mpfr-3.1.2.tar.bz2
$ curl -O ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.1.tar.gz
$ cd gcc-trunk-src
$ svn update 32017  # Optional command to select a known good gcc version
$ bzip2 -d -c ../gmp-5.1.1.tar.bz2 | tar -xvf -
$ mv gmp-5.1.1 gmp
$ bzip2 -d -c ../mpfr-3.1.2.tar.bz2 | tar -xvf -
$ mv mpfr-3.1.2 mpfr
$ tar -zxvf ../mpc-1.0.1.tar.gz
$ mv mpc-1.0.1 mpc
$ cd ..
$ mkdir gcc-trunk-build
$ cd gcc-trunk-build
$ LIBRARY_PATH=/usr/lib/$(gcc -print-multiarch); export LIBRARY_PATH
$ CPATH=/usr/include/$(gcc -print-multiarch); export CPATH
$ ../gcc-trunk-src/configure \
  CC=gcc \
  CXX=g++ \
  --prefix $(pwd)/../gcc-trunk-install \
  --disable-multilib \
  --disable-bootstrap \
  --enable-languages=c,c++
$ make -j 4
$ make install
$ cd ..
```

When complete, the new [gcc] build will be present in the `gcc-trunk-install`
directory.

## Building and installing [Origin-text_view]
[Text_view] does not currently build successfully with the top of trunk version
of the [Origin] library.  The [Origin-text_view] fork of [Origin] was created
to work around the issues.  The following commands can be used to perform a
suitable build of [Origin] on Linux that will work with [Text_view].  Note that
these commands assume that an appropriate build of [gcc] was performed and
installed as per the previous section.

```sh
$ PATH=$(pwd)/gcc-trunk-install/bin; export PATH
$ git clone https://github.com/tahonermann/origin.git origin-text_view-src
$ mkdir origin-text_view-build
$ cd origin-text_view-build
$ cmake ../origin-text_view-src \
  -DCMAKE_INSTALL_PREFIX:PATH=$(pwd)/../origin-text_view-install
$ make install
$ cd ..
```

## Building and installing [Text_view]
[Text_view] is currently a header-only library, so no build or installation is
currently required to use it.  It is unlikely that [Text_view] will remain a
header-only library in the future, so at some point, a build and installation
step will be required.

[Text_view] currently has a simple build system that is hard-coded to look for
[gcc] and [Origin] at specific locations.  This will change at some point in
the future, but given the present [gcc] and [Origin] requirements, hasn't been
a burden.  The build system is only used to build and run a few test programs.

The following commands suffice to build and run the test programs.  Note that it
may take several minutes to build the test-text_view program.

```sh
$ vi setenv-gcc-trunk.sh  # Update GCC_INSTALL_PATH and ORIGIN_INSTALL_PATH.
$ . ./setenv-gcc-trunk.sh
$ make
```

If the build succeeds, a few test and utility programs will be present in the
`bin` directory.

# Usage
[Text_view] is currently a header-only library.  To use it in your own code,
add include paths for the `text_view/include` and [Origin] installation
locations, and include the `text_view` header file:

```C++
#include <text_view>
```

All interfaces intended for public use are declared in the
`std::experimental::text` namespace.  The `text` namespace is an inline
namespace, so all entities are available from the `std::experimental` namespace
itself.

The interface descriptions in the sections that follow use the concept names
from the [ranges proposal][N4560].  However, the actual [Text_view]
implementation uses the [Origin] library for concept definitions and those
definitions do not currently match those in the [ranges proposal][N4560].  As a
result, the definitions that follow do not exactly match the code in the
[Text_view] library.  The following definitions are intended to be used as
specification and should be considereed authoritative.  Any differences in
behavior as defined by these definitions as compared to the [Text_view]
implementation is unintentional and should be considered indicatative of a
defect.

Once an implementation of the [ranges proposal][N4560] becomes available, it is
expected that [Text_view] will be ported to it.

## Header &lt;text_view&gt; synopsis

```C++
namespace std {
namespace experimental {
inline namespace text {

// concepts:
template<typename T> concept bool CodeUnit();
template<typename T> concept bool CodePoint();
template<typename T> concept bool CharacterSet();
template<typename T> concept bool Character();
template<typename T> concept bool CodeUnitIterator();
template<typename T, typename V> concept bool CodeUnitOutputIterator();
template<typename T> concept bool TextEncodingState();
template<typename T> concept bool TextEncodingStateTransition();
template<typename T> concept bool TextEncoding();
template<typename T, typename I> concept bool TextEncoder();
template<typename T, typename I> concept bool TextDecoder();
template<typename T, typename I> concept bool TextForwardDecoder();
template<typename T, typename I> concept bool TextBidirectionalDecoder();
template<typename T, typename I> concept bool TextRandomAccessDecoder();
template<typename T> concept bool TextIterator();
template<typename T> concept bool TextOutputIterator();
template<typename T, typename I> concept bool TextSentinel();
template<typename T> concept bool TextView();

// character sets:
class any_character_set;
class basic_execution_character_set;
class basic_execution_wide_character_set;
class unicode_character_set;

// implementation defined character set type aliases:
using execution_character_set = /* implementation-defined */ ;
using execution_wide_character_set = /* implementation-defined */ ;
using universal_character_set = /* implementation-defined */ ;

// character set identification:
class character_set_id;

template<typename CST>
  inline character_set_id get_character_set_id();

// character set information:
class character_set_info;

template<typename CST>
  inline const character_set_info& get_character_set_info();
const character_set_info& get_character_set_info(character_set_id id);

// character set and encoding traits:
template<typename T>
  using code_unit_type_t = /* implementation-defined */ ;
template<typename T>
  using code_point_type_t = /* implementation-defined */ ;
template<typename T>
  using character_set_type_t = /* implementation-defined */ ;
template<typename T>
  using character_type_t = /* implementation-defined */ ;
template<typename T>
  using encoding_type_t /* implementation-defined */ ;

// characters:
template<CharacterSet CST> class character;
template <> class character<any_character_set>;

template<CharacterSet CST>
  bool operator==(const character<any_character_set> &lhs,
                  const character<CST> &rhs);
template<CharacterSet CST>
  bool operator==(const character<CST> &lhs,
                  const character<any_character_set> &rhs);
template<CharacterSet CST>
  bool operator!=(const character<any_character_set> &lhs,
                  const character<CST> &rhs);
template<CharacterSet CST>
  bool operator!=(const character<CST> &lhs,
                  const character<any_character_set> &rhs);

// encoding state and transition types:
class trivial_encoding_state;
class trivial_encoding_state_transition;
class utf8bom_encoding_state;
class utf8bom_encoding_state_transition;
class utf16bom_encoding_state;
class utf16bom_encoding_state_transition;
class utf32bom_encoding_state;
class utf32bom_encoding_state_transition;

// encodings:
class basic_execution_character_encoding;
class basic_execution_wide_character_encoding;
#if defined(__STDC_ISO_10646__)
class iso_10646_wide_character_encoding;
#endif // __STDC_ISO_10646__
class utf8_encoding;
class utf8bom_encoding;
class utf16_encoding;
class utf16be_encoding;
class utf16le_encoding;
class utf16bom_encoding;
class utf32_encoding;
class utf32be_encoding;
class utf32le_encoding;
class utf32bom_encoding;

// implementation defined encoding type aliases:
using execution_character_encoding = /* implementation-defined */ ;
using execution_wide_character_encoding = /* implementation-defined */ ;
using char8_character_encoding = /* implementation-defined */ ;
using char16_character_encoding = /* implementation-defined */ ;
using char32_character_encoding = /* implementation-defined */ ;

// itext_iterator:
template<TextEncoding ET, ranges::InputRange RT>
  requires TextDecoder<ET, ranges::iterator_t<const RT>>()
  class itext_iterator;

// itext_sentinel:
template<TextEncoding ET, ranges::InputRange RT>
  class itext_sentinel;

// otext_iterator:
template<TextEncoding E, CodeUnitOutputIterator<code_unit_type_t<E>> CUIT>
  class otext_iterator;

// otext_iterator factory functions:
template<TextEncoding ET, CodeUnitOutputIterator<code_unit_type_t<ET>> IT>
  auto make_otext_iterator(typename ET::state_type state, IT out)
  -> otext_iterator<ET, IT>;
template<TextEncoding ET, CodeUnitOutputIterator<code_unit_type_t<ET>> IT>
  auto make_otext_iterator(IT out)
  -> otext_iterator<ET, IT>;

// basic_text_view:
template<TextEncoding ET, ranges::InputRange RT>
  class basic_text_view;

// basic_text_view type aliases:
using text_view = basic_text_view<execution_character_encoding,
                                  /* implementation-defined */ >;
using wtext_view = basic_text_view<execution_wide_character_encoding,
                                   /* implementation-defined */ >;
using u8text_view = basic_text_view<char8_character_encoding,
                                    /* implementation-defined */ >;
using u16text_view = basic_text_view<char16_character_encoding,
                                     /* implementation-defined */ >;
using u32text_view = basic_text_view<char32_character_encoding,
                                     /* implementation-defined */ >;

// basic_text_view factory functions:
template<TextEncoding ET, ranges::InputIterator IT, ranges::Sentinel<IT> ST>
  auto make_text_view(typename ET::state_type state, IT first, ST last)
  -> basic_text_view<ET, /* implementation-defined */ >;
template<TextEncoding ET, ranges::InputIterator IT, ranges::Sentinel<IT> ST>
  auto make_text_view(IT first, ST last)
  -> basic_text_view<ET, /* implementation-defined */ >;
template<TextEncoding ET, ranges::ForwardIterator IT>
  auto make_text_view(typename ET::state_type state,
                      IT first,
                      typename std::make_unsigned<ranges::difference_type_t<IT>>::type n)
  -> basic_text_view<ET, /* implementation-defined */ >;
template<TextEncoding ET, ranges::ForwardIterator IT>
  auto make_text_view(IT first,
                      typename std::make_unsigned<ranges::difference_type_t<IT>>::type n)
  -> basic_text_view<ET, /* implementation-defined */ >;
template<TextEncoding ET, ranges::InputRange Iterable>
  auto make_text_view(typename ET::state_type state,
                      const Iterable &iterable)
  -> basic_text_view<ET, /* implementation-defined */ >;
template<TextEncoding ET, ranges::InputRange Iterable>
  auto make_text_view(const Iterable &iterable)
  -> basic_text_view<ET, /* implementation-defined */ >;
template<TextIterator TIT, TextSentinel<TIT> TST>
  auto make_text_view(TIT first, TST last)
  -> basic_text_view<ET, /* implementation-defined */ >;
template<TextView TVT>
  TVT make_text_view(TVT tv);

} // inline namespace text
} // namespace experimental
} // namespace std
```

## Concepts

- [Concept CodeUnit](#concept-codeunit)
- [Concept CodePoint](#concept-codepoint)
- [Concept CharacterSet](#concept-characterset)
- [Concept Character](#concept-character)
- [Concept CodeUnitIterator](#concept-codeunititerator)
- [Concept CodeUnitOutputIterator](#concept-codeunitoutputiterator)
- [Concept TextEncodingState](#concept-textencodingstate)
- [Concept TextEncodingStateTransition]
  (#concept-textencodingstatetransition)
- [Concept TextEncoding](#concept-textencoding)
- [Concept TextEncoder](#concept-textencoder)
- [Concept TextDecoder](#concept-textdecoder)
- [Concept TextForwardDecoder](#concept-textforwarddecoder)
- [Concept TextBidirectionalDecoder](#concept-textbidirectionaldecoder)
- [Concept TextRandomAccessDecoder](#concept-textrandomaccessdecoder)
- [Concept TextIterator](#concept-textiterator)
- [Concept TextSentinel](#concept-textsentinel)
- [Concept TextOutputIterator](#concept-textoutputiterator)
- [Concept TextView](#concept-textview)

### Concept CodeUnit
The `CodeUnit` concept specifies requirements for a type usable as the
[code unit](#code-unit) type of a string type.

```C++
template<typename T> concept bool CodeUnit() {
  return /* implementation-defined */ ;
}
```

`CodeUnit<T>()` is satisfied if and only if
`std::is_integral<T>::value` is true and at least one of
`std::is_unsigned<T>::value` is true,
`std::is_same<std::remove_cv<T>::type, char>::value` is true, or
`std::is_same<std::remove_cv<T>::type, wchar_t>::value` is true.

### Concept CodePoint
The `CodePoint` concept specifies requirements for a type usable as the
[code point](#code-point) type of a [character set](#character-set) type.

```C++
template<typename T> concept bool CodePoint() {
  return /* implementation-defined */ ;
}
```

`CodePoint<T>()` is satisfied if and only if
`std::is_integral<T>::value` is true and at least one of
`std::is_unsigned<T>::value` is true,
`std::is_same<std::remove_cv<T>::type, char>::value` is true, or
`std::is_same<std::remove_cv<T>::type, wchar_t>::value` is true.

### Concept CharacterSet
The `CharacterSet` concept specifies requirements for a type that describes
a [character set](#character-set).  Such a type has a member typedef-name
declaration for a type that satisfies `CodePoint` and a static member function
that returns a name for the [character set](#character-set).

```C++
template<typename T> concept bool CharacterSet() {
  return CodePoint<code_point_type_t<T>>()
      && requires () {
           { T::get_name() } noexcept -> const char *;
         };
}
```

### Concept Character
The `Character` concept specifies requirements for a type that describes a
[character](#character) as defined by an associated
[character set](#character-set).  Non-static member functions provide access to
the [code point](#code-point) value of the described [character](#character).
Types that satisfy `Character` are regular and copyable.

```C++
template<typename T> concept bool Character() {
  return ranges::Regular<T>()
      && ranges::Copyable<T>()
      && CharacterSet<character_set_type_t<T>>()
      && requires (T t, code_point_type_t<character_set_type_t<T>> cp) {
           t.set_code_point(cp);
           { t.get_code_point() } -> code_point_type_t<character_set_type_t<T>>;
           { t.get_character_set_id() } -> character_set_id;
         };
}
```

### Concept CodeUnitIterator
The `CodeUnitIterator` concept specifies requirements of an iterator that
has a value type that satisfies `CodeUnit`.

```C++
template<typename T> concept bool CodeUnitIterator() {
  return ranges::Iterator<T>()
      && CodeUnit<ranges::value_type_t<T>>();
}
```

### Concept CodeUnitOutputIterator
The `CodeUnitOutputIterator` concept specifies requirements of an output
iterator that can be assigned from a type that satisfies `CodeUnit`.

```C++
template<typename T, typename V> concept bool CodeUnitOutputIterator() {
  return ranges::OutputIterator<T, V>()
      && CodeUnit<V>();
}
```

### Concept TextEncodingState
The `TextEncodingState` concept specifies requirements of types that hold
[encoding](#encoding) state.  Such types are default constructible and copyable.

```C++
template<typename T> concept bool TextEncodingState() {
  return ranges::DefaultConstructible<T>()
      && ranges::Copyable<T>();
}
```

### Concept TextEncodingStateTransition
The `TextEncodingStateTransition` concept specifies requirements of types
that hold [encoding](#encoding) state transitions.  Such types are default
constructible and copyable.

```C++
template<typename T> concept bool TextEncodingStateTransition() {
  return ranges::DefaultConstructible<T>()
      && ranges::Copyable<T>();
}
```

### Concept TextEncoding
The `TextEncoding` concept specifies requirements of types that define an
[encoding](#encoding).  Such types define member types that identify the
[code unit](#code-unit), [character](#character), encoding state, and encoding
state transition types, a static member function that returns an initial
encoding state object that defines the encoding state at the beginning of a
sequence of encoded characters, and static data members that specify the
minimum and maximum number of [code units](#code-units) used to encode any
single character.

```C++
template<typename T> concept bool TextEncoding() {
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
```

### Concept TextEncoder
The `TextEncoder` concept specifies requirements of types that are used to
encode [characters](#character) using a particular [code unit](#code-unit)
iterator that satisfies `OutputIterator`.  Such a type satisifies
`TextEncoding` and defines static member functions used to encode state
transitions and [characters](#character).

```C++
template<typename T, typename I> concept bool TextEncoder() {
  return TextEncoding<T>()
      && ranges::OutputIterator<CUIT, code_unit_type_t<T>>()
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
```

### Concept TextDecoder
The `TextDecoder` concept specifies requirements of types that are used to
decode [characters](#character) using a particular [code unit](#code-unit)
iterator that satisifies `InputIterator`.  Such a type satisfies
`TextEncoding` and defines a static member function used to decode state
transitions and [characters](#character).

```C++
template<typename T, typename I> concept bool TextDecoder() {
  return TextEncoding<T>()
      && ranges::InputIterator<CUIT>()
      && ranges::ConvertibleTo<ranges::value_type_t<CUIT>,
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
```

### Concept TextForwardDecoder
The `TextForwardDecoder` concept specifies requirements of types that are
used to decode [characters](#character) using a particular
[code unit](#code-unit) iterator that satisifies `ForwardIterator`.  Such a
type satisfies `TextDecoder`.

```C++
template<typename T, typename I> concept bool TextForwardDecoder() {
  return TextDecoder<T, CUIT>()
      && ranges::ForwardIterator<CUIT>();
}
```

### Concept TextBidirectionalDecoder
The `TextBidirectionalDecoder` concept specifies requirements of types that
are used to decode [characters](#character) using a particular
[code unit](#code-unit) iterator that satisifies `BidirectionalIterator`.  Such
a type satisfies `TextForwardDecoder` and defines a static member function
used to decode state transitions and [characters](#character) in the reverse
order of their encoding.

```C++
template<typename T, typename I> concept bool TextBidirectionalDecoder() {
  return TextForwardDecoder<T, CUIT>()
      && ranges::BidirectionalIterator<CUIT>()
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
```

### Concept TextRandomAccessDecoder
The `TextRandomAccessDecoder` concept specifies requirements of types that
are used to decode [characters](#character) using a particular
[code unit](#code-unit) iterator that satisifies `RandomAccessIterator`.  Such a
type satisfies `TextBidirectionalDecoder`, requires that the minimum and
maximum number of [code units](#code-unit) used to encode any character have
the same value, and that the encoding state be an empty type.

```C++
template<typename T, typename I> concept bool TextRandomAccessDecoder() {
  return TextBidirectionalDecoder<T, CUIT>()
      && ranges::RandomAccessIterator<CUIT>()
      && T::min_code_units == T::max_code_units
      && std::is_empty<typename T::state_type>::value;
}
```

### Concept TextIterator
The `TextIterator` concept specifies requirements of types that are used to
iterator over [characters](#character) in an [encoded](#encoding) sequence of
[code units](#code-unit).  [Encoding](#encoding) state is held in each iterator
instance as needed to decode the [code unit](#code-unit) sequence and is made
accessible via non-static member functions.  The value type of a
`TextIterator` satisfies `Character`.

```C++
template<typename T> concept bool TextIterator() {
  return ranges::Iterator<T>()
      && Character<ranges::value_type_t<T>>()
      && TextEncoding<encoding_type_t<T>>()
      && TextEncodingState<typename T::state_type>()
      && requires (T t, const T ct) {
           { t.state() } noexcept
               -> typename encoding_type_t<T>::state_type&;
           { ct.state() } noexcept
               -> const typename encoding_type_t<T>::state_type&;
         };
}
```

### Concept TextSentinel
The `TextSentinel` concept specifies requirements of types that are used to
mark the end of a range of encoded [characters](#character).  A type T that
satisfies `TextIterator` also satisfies `TextSentinel<T>` there by enabling
`TextIterator` types to be used as sentinels.

```C++
template<typename T, typename I> concept bool TextSentinel() {
  return ranges::Sentinel<T, I>()
      && TextIterator<I>();
}
```

### Concept TextOutputIterator
The `TextOutputIterator` concept specifies requirements of types that are used
to [encode](#encoding) [characters](#character) as a sequence of
[code units](#code-unit).  [Encoding](#encoding) state is held in each iterator
instance as needed to encode the [code unit](#code-unit) sequence and is made
accessible via non-static member functions.

```C++
template<typename T> concept bool TextOutputIterator() {
  return ranges::OutputIterator<T, character_type_t<encoding_type_t<T>>>()
      && TextEncoding<encoding_type_t<T>>()
      && TextEncodingState<typename T::state_type>()
      && requires (T t, const T ct) {
           { t.state() } noexcept
               -> typename encoding_type_t<T>::state_type&;
           { ct.state() } noexcept
               -> const typename encoding_type_t<T>::state_type&;
         };
}
```

### Concept TextView
The `TextView` concept specifies requirements of types that provide view access
to an underlying [code unit](#code-unit) range.  Such types satisy
`ranges::View`, provide iterators that satisfy `TextIterator`, define member
types that identify the [encoding](#encoding), encoding state, and underlying
[code unit](#code-unit) range and iterator types.  Non-static member functions
are provided to access the underlying [code unit](#code-unit) range and initial
[encoding](#encoding) state.

Types that satisfy `TextView` do not own the underlying [code unit](#code-unit)
range and are copyable in constant time.  The lifetime of the underlying range
must exceed the lifetime of referencing `TextView` objects.

```C++
template<typename T> concept bool TextView() {
  return ranges::View<T>()
      R& TextIterator<ranges::iterator_t<T>>()
      && TextEncoding<encoding_type_t<T>>()
      && ranges::InputRange<typename T::range_type>()
      && TextEncodingState<typename T::state_type>()
      && CodeUnitIterator<code_unit_iterator_t<T>>()
      R& requires (T t, const T ct) {
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
```

## Character sets

- [Class any_character_set](#class-any_character_set)
- [Class basic_execution_character_set]
  (#class-basic_execution_character_set)
- [Class basic_execution_wide_character_set]
  (#class-basic_execution_wide_character_set)
- [Class unicode_character_set](#class-unicode_character_set)
- [Character set type aliases](#character-set-type-aliases)

### Class any_character_set

```C++
class any_character_set {
public:
  using code_point_type = /* implementation-defined */;

  static const char* get_name() noexcept;
};
```

### Class basic_execution_character_set

```C++
class basic_execution_character_set {
public:
  using code_point_type = char;

  static const char* get_name() noexcept;
};
```

### Class basic_execution_wide_character_set

```C++
class basic_execution_wide_character_set {
public:
  using code_point_type = wchar_t;

  static const char* get_name() noexcept;
};
```

### Class unicode_character_set

```C++
class unicode_character_set {
public:
  using code_point_type = char32_t;

  static const char* get_name() noexcept;
};
```

### Character set type aliases

```C++
using execution_character_set = /* implementation-defined */ ;
using execution_wide_character_set = /* implementation-defined */ ;
using universal_character_set = /* implementation-defined */ ;
```

## Character set identification

- [Class character_set_id](#class-character_set_id)
- [get_character_set_id](#get_character_set_id)

### Class character_set_id

```C++
class character_set_id {
public:
  character_set_id() = delete;

  friend bool operator==(character_set_id lhs, character_set_id rhs);
  friend bool operator!=(character_set_id lhs, character_set_id rhs);

  friend bool operator<(character_set_id lhs, character_set_id rhs);
  friend bool operator>(character_set_id lhs, character_set_id rhs);
  friend bool operator<=(character_set_id lhs, character_set_id rhs);
  friend bool operator>=(character_set_id lhs, character_set_id rhs);
};
```

### get_character_set_id

```C++
template<typename CST>
  inline character_set_id get_character_set_id();
```

## Character set information

- [Class character_set_info](#class-character_set_info)
- [get_character_set_info](#get_character_set_info)

### Class character_set_info

```C++
class character_set_info {
public:
  character_set_info() = delete;

  character_set_id get_id() const noexcept;

  const char* get_name() const noexcept;

private:
  character_set_id id; // exposition only
};
```

### get_character_set_info

```C++
const character_set_info& get_character_set_info(character_set_id id);

template<typename CST>
  inline const character_set_info& get_character_set_info();
```

## Characters

- [Class template character](#class-template-character)

### Class template character

```C++
template<CharacterSet CST>
class character {
public:
  using character_set_type = CST;
  using code_point_type = code_point_type_t<character_set_type>;

  character() = default;
  explicit character(code_point_type code_point);

  friend bool operator==(const character &lhs, const character &rhs);
  friend bool operator!=(const character &lhs, const character &rhs);

  void set_code_point(code_point_type code_point);
  code_point_type get_code_point() const;

  static character_set_id get_character_set_id();

private:
  code_point_type code_point; // exposition only
};

template<>
class character<any_character_set> {
public:
  using character_set_type = any_character_set;
  using code_point_type = code_point_type_t<character_set_type>;

  character() = default;
  explicit character(code_point_type code_point);
  character(character_set_id cs_id, code_point_type code_point);

  friend bool operator==(const character &lhs, const character &rhs);
  friend bool operator!=(const character &lhs, const character &rhs);

  void set_code_point(code_point_type code_point);
  code_point_type get_code_point() const;

  void set_character_set_id(character_set_id new_cs_id);
  character_set_id get_character_set_id() const;

private:
  character_set_id cs_id;     // exposition only
  code_point_type code_point; // exposition only
};

template<CharacterSet CST>
  bool operator==(const character<any_character_set> &lhs,
                  const character<CST> &rhs);
template<CharacterSet CST>
  bool operator==(const character<CST> &lhs,
                  const character<any_character_set> &rhs);
template<CharacterSet CST>
  bool operator!=(const character<any_character_set> &lhs,
                  const character<CST> &rhs);
template<CharacterSet CST>
  bool operator!=(const character<CST> &lhs,
                  const character<any_character_set> &rhs);
```

## Encodings

- [class trivial_encoding_state](#class-trivial_encoding_state)
- [class trivial_encoding_state_transition]
  (#class-trivial_encoding_state_transition)
- [Class basic_execution_character_encoding]
  (#class-basic_execution_character_encoding)
- [Class basic_execution_wide_character_encoding]
  (#class-basic_execution_wide_character_encoding)
- [Class iso_10646_wide_character_encoding]
  (#class-iso_10646_wide_character_encoding)
- [Class utf8_encoding](#class-utf8_encoding)
- [Class utf8bom_encoding](#class-utf8bom_encoding)
- [Class utf16_encoding](#class-utf16_encoding)
- [Class utf16be_encoding](#class-utf16be_encoding)
- [Class utf16le_encoding](#class-utf16le_encoding)
- [Class utf16bom_encoding](#class-utf16bom_encoding)
- [Class utf32_encoding](#class-utf32_encoding)
- [Class utf32be_encoding](#class-utf32be_encoding)
- [Class utf32le_encoding](#class-utf32le_encoding)
- [Class utf32bom_encoding](#class-utf32bom_encoding)
- [Encoding type aliases](#encoding-type-aliases)

### Class trivial_encoding_state

```C++
class trivial_encoding_state {};
```

### Class trivial_encoding_state_transition

```C++
class trivial_encoding_state_transition {};
```

### Class basic_execution_character_encoding

```C++
class basic_execution_character_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<basic_execution_character_set>;
  using code_unit_type = char;

  static constexpr int min_code_units = 1;
  static constexpr int max_code_units = 1;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class basic_execution_wide_character_encoding

```C++
class basic_execution_wide_character_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<basic_execution_wide_character_set>;
  using code_unit_type = wchar_t;

  static constexpr int min_code_units = 1;
  static constexpr int max_code_units = 1;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class iso_10646_wide_character_encoding

```C++
#if defined(__STDC_ISO_10646__)
class iso_10646_wide_character_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = wchar_t;

  static constexpr int min_code_units = 1;
  static constexpr int max_code_units = 1;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
#endif // __STDC_ISO_10646__
```

### Class utf8_encoding

```C++
class utf8_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char;

  static constexpr int min_code_units = 1;
  static constexpr int max_code_units = 4;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<std::make_unsigned_t<code_unit_type>> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<std::make_unsigned_t<code_unit_type>> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class utf8bom_encoding

```C++
class utf8bom_encoding_state {
  /* implementation-defined */
};

class utf8bom_encoding_state_transition {
public:
  static utf8bom_encoding_state_transition to_initial_state();
  static utf8bom_encoding_state_transition to_bom_written_state();
  static utf8bom_encoding_state_transition to_assume_bom_written_state();
};

class utf8bom_encoding {
public:
  using state_type = utf8bom_encoding_state;
  using state_transition_type = utf8bom_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char;

  static constexpr int min_code_units = 1;
  static constexpr int max_code_units = 4;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<std::make_unsigned_t<code_unit_type>> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<std::make_unsigned_t<code_unit_type>> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class utf16_encoding

```C++
class utf16_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char16_t;

  static constexpr int min_code_units = 1;
  static constexpr int max_code_units = 2;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class utf16be_encoding

```C++
class utf16be_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char;

  static constexpr int min_code_units = 2;
  static constexpr int max_code_units = 4;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class utf16le_encoding

```C++
class utf16le_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char;

  static constexpr int min_code_units = 2;
  static constexpr int max_code_units = 4;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class utf16bom_encoding

```C++
class utf16bom_encoding_state {
  /* implementation-defined */
};

class utf16bom_encoding_state_transition {
public:
  static utf16bom_encoding_state_transition to_initial_state();
  static utf16bom_encoding_state_transition to_bom_written_state();
  static utf16bom_encoding_state_transition to_be_bom_written_state();
  static utf16bom_encoding_state_transition to_le_bom_written_state();
  static utf16bom_encoding_state_transition to_assume_bom_written_state();
  static utf16bom_encoding_state_transition to_assume_be_bom_written_state();
  static utf16bom_encoding_state_transition to_assume_le_bom_written_state();
};

class utf16bom_encoding {
public:
  using state_type = utf16bom_encoding_state;
  using state_transition_type = utf16bom_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char;

  static constexpr int min_code_units = 2;
  static constexpr int max_code_units = 4;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class utf32_encoding

```C++
class utf32_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char32_t;

  static constexpr int min_code_units = 1;
  static constexpr int max_code_units = 1;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class utf32be_encoding

```C++
class utf32be_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char;

  static constexpr int min_code_units = 4;
  static constexpr int max_code_units = 4;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class utf32le_encoding

```C++
class utf32le_encoding {
public:
  using state_type = trivial_encoding_state;
  using state_transition_type = trivial_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char;

  static constexpr int min_code_units = 4;
  static constexpr int max_code_units = 4;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Class utf32bom_encoding

```C++
class utf32bom_encoding_state {
  /* implementation-defined */
};

class utf32bom_encoding_state_transition {
public:
  static utf32bom_encoding_state_transition to_initial_state();
  static utf32bom_encoding_state_transition to_bom_written_state();
  static utf32bom_encoding_state_transition to_be_bom_written_state();
  static utf32bom_encoding_state_transition to_le_bom_written_state();
  static utf32bom_encoding_state_transition to_assume_bom_written_state();
  static utf32bom_encoding_state_transition to_assume_be_bom_written_state();
  static utf32bom_encoding_state_transition to_assume_le_bom_written_state();
};

class utf32bom_encoding {
public:
  using state_type = utf32bom_encoding_state;
  using state_transition_type = utf32bom_encoding_state_transition;
  using character_type = character<unicode_character_set>;
  using code_unit_type = char;

  static constexpr int min_code_units = 4;
  static constexpr int max_code_units = 4;

  static const state_type& initial_state();

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode_state_transition(state_type &state,
                                        CUIT &out,
                                        const state_transition_type &stt,
                                        int &encoded_code_units)

  template<CodeUnitOutputIterator<code_unit_type> CUIT>
    static void encode(state_type &state,
                       CUIT &out,
                       character_type c,
                       int &encoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool decode(state_type &state,
                       CUIT &in_next,
                       CUST in_end,
                       character_type &c,
                       int &decoded_code_units)

  template<CodeUnitIterator CUIT, typename CUST>
    requires ranges::InputIterator<CUIT>()
          && ranges::Convertible<ranges::value_type_t<CUIT>, code_unit_type>()
          && ranges::Sentinel<CUST, CUIT>()
    static bool rdecode(state_type &state,
                        CUIT &in_next,
                        CUST in_end,
                        character_type &c,
                        int &decoded_code_units)
};
```

### Encoding type aliases

```C++
using execution_character_encoding = /* implementation-defined */ ;
using execution_wide_character_encoding = /* implementation-defined */ ;
using char8_character_encoding = /* implementation-defined */ ;
using char16_character_encoding = /* implementation-defined */ ;
using char32_character_encoding = /* implementation-defined */ ;
```

## Text iterators

- [Class template itext_iterator](#class-template-itext_iterator)
- [Class template itext_sentinel](#class-template-itext_sentinel)
- [Class template otext_iterator](#class-template-otext_iterator)
- [make_otext_iterator](#make_otext_iterator)

### Class template itext_iterator

```C++
template<TextEncoding ET, ranges::InputRange RT>
  requires TextDecoder<
             ET,
             ranges::iterator_t<std::add_const_t<std::remove_reference_t<RT>>>>()
class itext_iterator {
public:
  using encoding_type = ET;
  using range_type = std::remove_reference_t<RT>;
  using state_type = typename encoding_type::state_type;

  using iterator = ranges::iterator_t<std::add_const_t<range_type>>;
  using iterator_category = /* implementation-defined */;
  using value_type = character_type_t<encoding_type>;
  using reference = std::add_const_t<value_type>&;
  using pointer = std::add_const_t<value_type>*;
  using difference_type = ranges::difference_type_t<iterator>;

  itext_iterator();

  itext_iterator(const state_type &state,
                 const range_type *range,
                 iterator first);

  reference operator*() const noexcept;
  pointer operator->() const noexcept;

  friend bool operator==(const itext_iterator &l, const itext_iterator &r);
  friend bool operator!=(const itext_iterator &l, const itext_iterator &r);

  friend bool operator<(const itext_iterator &l, const itext_iterator &r)
    requires TextRandomAccessDecoder<encoding_type, iterator>();
  friend bool operator>(const itext_iterator &l, const itext_iterator &r)
    requires TextRandomAccessDecoder<encoding_type, iterator>();
  friend bool operator<=(const itext_iterator &l, const itext_iterator &r)
    requires TextRandomAccessDecoder<encoding_type, iterator>();
  friend bool operator>=(const itext_iterator &l, const itext_iterator &r)
    requires TextRandomAccessDecoder<encoding_type, iterator>();

  itext_iterator& operator++();
  itext_iterator& operator++()
    requires TextForwardDecoder<encoding_type, iterator>();
  itext_iterator operator++(int);

  itext_iterator& operator--()
    requires TextBidirectionalDecoder<encoding_type, iterator>();
  itext_iterator operator--(int)
    requires TextBidirectionalDecoder<encoding_type, iterator>();

  itext_iterator& operator+=(difference_type n)
    requires TextRandomAccessDecoder<encoding_type, iterator>();
  itext_iterator& operator-=(difference_type n)
    requires TextRandomAccessDecoder<encoding_type, iterator>();

  friend itext_iterator operator+(itext_iterator l, difference_type n)
    requires TextRandomAccessDecoder<encoding_type, iterator>();
  friend itext_iterator operator+(difference_type n, itext_iterator r)
    requires TextRandomAccessDecoder<encoding_type, iterator>();

  friend itext_iterator operator-(itext_iterator l, difference_type n)
    requires TextRandomAccessDecoder<encoding_type, iterator>();
  friend difference_type operator-(const itext_iterator &l,
                                   const itext_iterator &r)
    requires TextRandomAccessDecoder<encoding_type, iterator>();

  value_type operator[](difference_type n) const
    requires TextRandomAccessDecoder<encoding_type, iterator>();

  const state_type& state() const noexcept;
  state_type& state() noexcept;

  iterator base() const;

  /* implementation-defined */ base_range() const
    requires TextDecoder<encoding_type, iterator>()
          && ranges::ForwardIterator<iterator>();

  bool is_ok() const noexcept;

private:
  state_type base_state;  // exposition only
  iterator base_iterator; // exposition only
  bool ok;                // exposition only
};
```

### Class template itext_sentinel

```C++
template<TextEncoding ET, ranges::InputRange RT>
class itext_sentinel {
public:
  using range_type = std::remove_reference_t<RT>;
  using sentinel = ranges::sentinel_t<std::add_const_t<range_type>>;

  itext_sentinel(sentinel s);

  itext_sentinel(const itext_iterator<ET, RT> &ti)
    requires ranges::ConvertibleTo<decltype(ti.base()), sentinel>();

  friend bool operator==(const itext_sentinel &l, const itext_sentinel &r);
  friend bool operator!=(const itext_sentinel &l, const itext_sentinel &r);

  friend bool operator==(const itext_iterator<ET, RT> &ti,
                         const itext_sentinel &ts);
  friend bool operator!=(const itext_iterator<ET, RT> &ti,
                         const itext_sentinel &ts);
  friend bool operator==(const itext_sentinel &ts,
                         const itext_iterator<ET, RT> &ti);
  friend bool operator!=(const itext_sentinel &ts,
                         const itext_iterator<ET, RT> &ti);

  friend bool operator<(const itext_sentinel &l, const itext_sentinel &r);
  friend bool operator>(const itext_sentinel &l, const itext_sentinel &r);
  friend bool operator<=(const itext_sentinel &l, const itext_sentinel &r);
  friend bool operator>=(const itext_sentinel &l, const itext_sentinel &r);

  friend bool operator<(const itext_iterator<ET, RT> &ti,
                        const itext_sentinel &ts)
    requires ranges::StrictWeakOrder<
                 std::less<>,
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>();
  friend bool operator>(const itext_iterator<ET, RT> &ti,
                        const itext_sentinel &ts)
    requires ranges::StrictWeakOrder<
                 std::less<>,
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>();
  friend bool operator<=(const itext_iterator<ET, RT> &ti,
                         const itext_sentinel &ts)
    requires ranges::StrictWeakOrder<
                 std::less<>,
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>();
  friend bool operator>=(const itext_iterator<ET, RT> &ti,
                         const itext_sentinel &ts)
    requires ranges::StrictWeakOrder<
                 std::less<>,
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>();

  friend bool operator<(const itext_sentinel &ts,
                        const itext_iterator<ET, RT> &ti)
    requires ranges::StrictWeakOrder<
                 std::less<>,
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>();
  friend bool operator>(const itext_sentinel &ts,
                        const itext_iterator<ET, RT> &ti)
    requires ranges::StrictWeakOrder<
                 std::less<>,
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>();
  friend bool operator<=(const itext_sentinel &ts,
                         const itext_iterator<ET, RT> &ti)
    requires ranges::StrictWeakOrder<
                 std::less<>,
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>();
  friend bool operator>=(const itext_sentinel &ts,
                         const itext_iterator<ET, RT> &ti)
    requires ranges::StrictWeakOrder<
                 std::less<>,
                 typename itext_iterator<ET, RT>::iterator,
                 sentinel>();

  sentinel base() const;

private:
  sentinel base_sentinel; // exposition only
};
```

### Class template otext_iterator

```C++
template<TextEncoding E, CodeUnitOutputIterator<code_unit_type_t<E>> CUIT>
class otext_iterator {
public:
  using encoding_type = E;
  using state_type = typename E::state_type;
  using state_transition_type = typename E::state_transition_type;

  using iterator = CUIT;
  using iterator_category = std::output_iterator_tag;
  using value_type = character_type_t<encoding_type>;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = ranges::difference_type_t<iterator>;

  otext_iterator();

  otext_iterator(state_type state, iterator current);

  otext_iterator& operator*();

  otext_iterator& operator++();
  otext_iterator& operator++(int);

  otext_iterator& operator=(const state_transition_type &stt);
  otext_iterator& operator=(const character_type_t<encoding_type> &value);

  const state_type& state() const noexcept;
  state_type& state() noexcept;

  iterator base() const;

private:
  state_type base_state;  // exposition only
  iterator base_iterator; // exposition only
};
```

### make_otext_iterator

```C++
template<TextEncoding ET, CodeUnitOutputIterator<code_unit_type_t<ET>> IT>
  auto make_otext_iterator(typename ET::state_type state, IT out)
  -> otext_iterator<ET, IT>;
template<TextEncoding ET, CodeUnitOutputIterator<code_unit_type_t<ET>> IT>
  auto make_otext_iterator(IT out)
  -> otext_iterator<ET, IT>;
```

## Text view

- [Class template basic_text_view](#class-template-basic_text_view)
- [Text view type aliases](#text-view-type-aliases)
- [make_text_view](#make_text_view)

### Class template basic_text_view

```C++
template<TextEncoding ET, ranges::InputRange RT>
class basic_text_view {
public:
  using encoding_type = ET;
  using range_type = RT;
  using state_type = typename ET::state_type;
  using code_unit_iterator = ranges::iterator_t<std::add_const_t<range_type>>;
  using code_unit_sentinel = ranges::sentinel_t<std::add_const_t<range_type>>;
  using iterator = itext_iterator<ET, RT>;
  using sentinel = itext_sentinel<ET, RT>;

  basic_text_view();

  basic_text_view(state_type state,
                  range_type r)
    requires ranges::CopyConstructible<range_type>();

  basic_text_view(range_type r)
    requires ranges::CopyConstructible<range_type>();

  basic_text_view(state_type state,
                  code_unit_iterator first,
                  code_unit_sentinel last)
    requires ranges::Constructible<range_type,
                                   code_unit_iterator,
                                   code_unit_sentinel>();

  basic_text_view(code_unit_iterator first,
                  code_unit_sentinel last)
    requires ranges::Constructible<range_type,
                                   code_unit_iterator,
                                   code_unit_sentinel>();

  basic_text_view(state_type state,
                  code_unit_iterator first,
                  ranges::difference_type_t<code_unit_iterator> n)
    requires ranges::Constructible<range_type,
                                   code_unit_iterator,
                                   code_unit_iterator>();

  basic_text_view(code_unit_iterator first,
                  ranges::difference_type_t<code_unit_iterator> n)
    requires ranges::Constructible<range_type,
                                   code_unit_iterator,
                                   code_unit_iterator>();

  template<typename charT, typename traits, typename Allocator>
    basic_text_view(state_type state,
                    const basic_string<charT, traits, Allocator> &str)
    requires ranges::Constructible<code_unit_iterator, const charT *>()
          && ranges::Constructible<ranges::difference_type_t<code_unit_iterator>,
                                   typename basic_string<charT, traits, Allocator>::size_type>()
          && ranges::Constructible<range_type,
                                   code_unit_iterator,
                                   code_unit_sentinel>();

  template<typename charT, typename traits, typename Allocator>
    basic_text_view(const basic_string<charT, traits, Allocator> &str)
    requires ranges::Constructible<code_unit_iterator, const charT *>()
          && ranges::Constructible<ranges::difference_type_t<code_unit_iterator>,
                                   typename basic_string<charT, traits, Allocator>::size_type>()
          && ranges::Constructible<range_type,
                                   code_unit_iterator,
                                   code_unit_sentinel>();

  template<ranges::InputRange Iterable>
    basic_text_view(state_type state,
                    const Iterable &iterable)
    requires ranges::Constructible<code_unit_iterator,
                                   ranges::iterator_t<const Iterable>>()
          && ranges::Constructible<range_type,
                                   code_unit_iterator,
                                   code_unit_sentinel>();

  template<ranges::InputRange Iterable>
    basic_text_view(const Iterable &iterable)
    requires ranges::Constructible<code_unit_iterator,
                                   ranges::iterator_t<const Iterable>>()
          && ranges::Constructible<range_type,
                                   code_unit_iterator,
                                   code_unit_sentinel>();

  basic_text_view(iterator first, sentinel last)
    requires ranges::Constructible<code_unit_iterator,
                                   decltype(std::declval<iterator>().base())>()
          && ranges::Constructible<range_type,
                                   code_unit_iterator,
                                   code_unit_sentinel>();

  const range_type& base() const noexcept;
  range_type& base() noexcept;

  const state_type& initial_state() const noexcept;
  state_type& initial_state() noexcept;

  iterator begin() const;
  iterator end() const
    requires std::is_empty<state_type>::value
          && ranges::Iterator<code_unit_sentinel>();
  sentinel end() const
    requires !std::is_empty<state_type>::value
          || !ranges::Iterator<code_unit_sentinel>();

private:
  state_type base_state; // exposition only
  range_type base_range; // exposition only
};

```

### Text view type aliases

```C++
using text_view = basic_text_view<
          execution_character_encoding,
          /* implementation-defined */ >;
using wtext_view = basic_text_view<
          execution_wide_character_encoding,
          /* implementation-defined */ >;
using u8text_view = basic_text_view<
          char8_character_encoding,
          /* implementation-defined */ >;
using u16text_view = basic_text_view<
          char16_character_encoding,
          /* implementation-defined */ >;
using u32text_view = basic_text_view<
          char32_character_encoding,
          /* implementation-defined */ >;
```

### make_text_view

```C++
template<TextEncoding ET, ranges::InputIterator IT, ranges::Sentinel<IT> ST>
  auto make_text_view(typename ET::state_type state,
                      IT first, ST last)
  -> basic_text_view<ET, /* implementation-defined */ >;


template<TextEncoding ET, ranges::InputIterator IT, ranges::Sentinel<IT> ST>
  auto make_text_view(IT first, ST last)
  -> basic_text_view<ET, /* implementation-defined */ >;

template<TextEncoding ET, ranges::ForwardIterator IT>
  auto make_text_view(typename ET::state_type state,
                      IT first,
                      ranges::difference_type_t<IT> n)
  -> basic_text_view<ET, /* implementation-defined */ >;

template<TextEncoding ET, ranges::ForwardIterator IT>
  auto make_text_view(IT first,
                      ranges::difference_type_t<IT> n)
  -> basic_text_view<ET, /* implementation-defined */ >;

template<TextEncoding ET, ranges::InputRange Iterable>
  auto make_text_view(typename ET::state_type state,
                      const Iterable &iterable)
  -> basic_text_view<ET, /* implementation-defined */ >;

template<TextEncoding ET, ranges::InputRange Iterable>
  auto make_text_view(const Iterable &iterable)
  -> basic_text_view<ET, /* implementation-defined */ >;

template<TextIterator TIT, TextSentinel<TIT> TST>
  auto make_text_view(TIT first, TST last)
  -> basic_text_view<ET, /* implementation-defined */ >;

template<TextView TVT>
  TVT make_text_view(TVT tv);
```

# Supported Encodings
As of 2015-12-31, supported [encodings](#encoding) include:

Encoding name | Description | Encoding type
------------- | ----------- | -------------
execution_character_encoding | Type alias for the encoding of ordinary string literals (C++11 2.3p3) | implementation defined
execution_wide_character_encoding | Type alias for the encoding of wide string literals (C++11 2.3p3) | implementation defined
char8_character_encoding | Type alias for the encoding of UTF-8 string literals (C++11 2.14.5p7) | implementation defined
char16_character_encoding | Type alias for the encoding of char16_t string literals (C++11 2.14.5p9) | implementation defined
char32_character_encoding | Type alias for the encoding of char32_t string literals (C++11 2.14.5p10) | implementation defined
basic_execution_character_encoding | An encoding that meets the minimum requirements of C++11 2.3p3 | trivial
basic_execution_wide_character_encoding | An encoding that meets the minimum requirements of C++11 2.3p3 | trivial
iso_10646_wide_character_encoding | An ISO 10646 encoding.  Only defined if __STDC_ISO_10646__ is defined | trivial
utf8_encoding | [Unicode] UTF-8 | stateless, variable width
utf8bom_encoding | [Unicode] UTF-8 with a byte order mark | stateful, variable width
utf16_encoding | [Unicode] UTF-16, native endian | stateless, variable width
utf16be_encoding | [Unicode] UTF-16, big endian | stateless, variable width
utf16le_encoding | [Unicode] UTF-16, little endian | stateless, variable width
utf16bom_encoding | [Unicode] UTF-16 with a byte order mark | stateful, variable width
utf32_encoding | [Unicode] UTF-32, native endian | trivial
utf32be_encoding | [Unicode] UTF-16, big endian | stateless, fixed width
utf32le_encoding | [Unicode] UTF-16, little endian | stateless, fixed width
utf32bom_encoding | [Unicode] UTF-32 with a byte order mark | stateful, variable width

# Terminology
The terminology used in this document and in the [Text_view] library has been
chosen to be consistent with industry standards and, in particular, the
[Unicode standard].  Any inconsistencies in the use of this terminology and that
in the [Unicode standard] is unintentional.  The terms described in this
document and used within the [Text_view] library comprise a subset of the
terminology used within the [Unicode standard]; only those terms necessary to
specify functionality exhibited by this library are included here.  Those who
would like to learn more about general text processing terminology in computer
systems are encouraged to read chatper 2, "General Structure" of the
[Unicode standard].

## Code Unit
A single, indivisible, integral element of an encoded sequence of characters.  A
sequence of one or more code units specifies a code point or encoding state
transition as defined by a character encoding.  A code unit does not, by itself,
identify any particular character or code point; the meaning ascribed to a
particular code unit value is derived from a character encoding definition.

The [C++11][ISO/IEC 14882:2011] `char`, `wchar_t`, `char16_t`, and `char32_t`
types are most commonly used as code unit types.

The string literal `u8"J\u00F8erg"` contains 7 code units and 6 code unit
sequences; "\u00F8" is encoded using two code units and string literals contain
a trailing NUL code unit.

The string literal `"J\u00F8erg"` contains an implementation defined number of
code units.  The standard does not specify the encoding of ordinary and wide
string literals, so the number of code units encoded by "\u00F8" depends on the
implementation defined encoding used for ordinary string literals.

## Code Point
An integral value denoting an abstract character as defined by a character set.
A code point does not, by itself, identify any particular character; the
meaning ascribed to a particular code point value is derived from a character
set definition.

The [C++11][ISO/IEC 14882:2011] `char`, `wchar_t`, `char16_t`, and `char32_t`
types are most commonly used as code point types.

The string literal `u8"J\u00F8erg"` describes a sequence of 6 code point values;
string literals implicitly specify a trailing NUL code point.

The string literal `"J\u00F8erg"` describes a sequence of an implementation
defined number of code point values.  The standard does not specify the encoding
of ordinary and wide string literals, so the number of code points encoded by
"\u00F8" depends on the implementation defined encoding used for ordinary string
literals.  Implementations are permitted to translate a single code point in the
source or Unicode character sets to multiple code points in the execution
encoding.

## Character Set
A mapping of code point values to abstract characters.  A character set need not
provide a mapping for every possible code point value representable by the code
point type.

[C++11][ISO/IEC 14882:2011] does not specify the use of any particular character
set or encoding for ordinary and wide character and string literals, though it
does place some restrictions on them.  Unicode character and string literals are
governed by the [Unicode] standard.

Common character sets include [ASCII], [Unicode][Unicode code charts], and
[Windows code page 1252].

## Character
An element of written language, for example, a letter, number, or symbol.  A
character is identified by the combination of a character set and a code point
value.

## Encoding
A method of representing a sequence of characters as a sequence of code unit
sequences.

An encoding may be stateless or stateful.  In stateless encodings, characters
may be encoded or decoded starting from the beginning of any code unit sequence.
In stateful encodings, it may be necessary to record certain affects of
previously encoded characters in order to correctly encode additional
characters, or to decode preceding code unit sequences in order to correctly
decode following code unit sequences.

An encoding may be fixed width or variable width.  In fixed width encodings,
all characters are encoded using a single code unit sequence and all code unit
sequences have the same length.  In variable width encodings, different
characters may require multiple code unit sequences, or code unit sequences of
varying length.

An encoding may support bidirectional or random access decoding of code unit
sequences.  In bidirectional encodings, characters may be decoded by traversing
code unit sequences in reverse order.  Such encodings must support a method to
identify the start of a preceding code unit sequence.  In random access
encodings, characters may be decoded from any code unit sequence within the
sequence of code unit sequences, in constant time, without having to decode any
other code unit sequence.  Random access encodings are necessarily stateless
and fixed length.  An encoding that is neither bidirectional, nor random
access, may only be decoded by traversing code unit sequences in forward order.

An encoding may support encoding characters from multiple character sets.  Such
an encoding is either stateful and defines code unit sequences that switch the
active character set, or defines code unit sequences that implicitly identify
a character set, or both.

A trivial encoding is one in which all encoded characters correspond to a single
character set and where each code unit encodes exactly one character using the
same value as the code point for that character.  Such an encoding is stateless,
fixed width, and supports random access decoding.

Common encodings include the [Unicode] UTF-8, UTF-16, and UTF-32 encodings, the
ISO/IEC 8859 series of encodings including [ISO/IEC 8859-1], and many trivial
encodings such as [Windows code page 1252].

# References
- [Text_view]  
  https://github.com/tahonermann/text_view
- [Origin]  
  http://asutton.github.io/origin
- [Origin-text_view]  
  https://github.com/tahonermann/origin
- [ISO/IEC technical specification 19217:2015, C++ Extensions for concepts][ISO/IEC 19217:2015]  
  http://www.iso.org/iso/home/store/catalogue_tc/catalogue_detail.htm?csnumber=64031
- [P0022R1: Proxy Iterators for the Ranges Extensions][P0022R1]  
  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0022r1.html
- [N4560: Working Draft, C++ Extensions for Ranges][N4560]  
  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4560.pdf
- [N2442: Raw and Unicode String Literals; Unified Proposal (Rev. 2)][N2442]  
  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2442.htm
- [N2249: New Character Types in C++][N2249]  
  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2249.html
- [Unicode]  
  http://unicode.org

[Text_view]:
https://github.com/tahonermann/text_view
(Text_view library)
[Andrew Sutton]:
https://github.com/asutton
(Andrew Sutton)
[Origin]:
http://asutton.github.io/origin
(Origin libraries)
[Origin-text_view]:
https://github.com/tahonermann/origin
(Origin libraries for text_view)
[Unicode]:
http://unicode.org
(The Unicode Consortium)
[Unicode standard]:
http://www.unicode.org/standard/standard.html
(The Unicode Standard)
[Unicode code charts]:
http://www.unicode.org/charts
(Unicode Character Code Charts)
[ASCII]:
http://webstore.ansi.org/RecordDetail.aspx?sku=INCITS+4-1986[R2012]
(INCITS 4-1986[R2012])
[Windows code page 1252]:
https://msdn.microsoft.com/en-us/library/cc195054.aspx
(Code Page 1252 Windows Latin 1)
[ISO/IEC 8859-1]:
http://webstore.ansi.org/RecordDetail.aspx?sku=ISO%2fIEC+8859-1%3a1998
(ISO/IEC 8859-1:1998 Information technology - 8-bit single-byte coded graphic character sets - Part 1: Latin alphabet No. 1)
[ISO/IEC 14882:2011]:
http://www.iso.org/iso/home/store/catalogue_ics/catalogue_detail_ics.htm?csnumber=50372
(ISO/IEC 14882:2011 Information technology -- Programming languages -- C++)
[ISO/IEC 19217:2015]:
http://www.iso.org/iso/home/store/catalogue_tc/catalogue_detail.htm?csnumber=64031
(ISO/IEC technical specification 19217:2015, C++ Extensions for concepts)
[N2249]:
http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2249.html
(N2249: New Character Types in C++)
[N2442]:
http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2442.htm
(N2442: Raw and Unicode String Literals; Unified Proposal (Rev. 2))
[N4560]:
http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4560.pdf
(N4560: Working Draft, C++ Extensions for Ranges)
[P0022R1]:
http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0022r1.html
(P0022R1: Proxy Iterators for the Ranges Extensions)
[gcc]:
https://gcc.gnu.org
(GCC, the GNU Compiler Collection)
