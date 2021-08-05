:- module(binformat, [% Low-level DCG nonterminals
                      bin_uint8//1,
                      bin_sint8//1,
                      bin_uint16//1,
                      bin_sint16//1,
                      bin_uint32//1,
                      bin_sint32//1,
                      bin_uint64//1,
                      bin_sint64//1,
                      % Endian-specific DCG nonterminals
                      bin_uint16_le//1,
                      bin_sint16_le//1,
                      bin_uint32_le//1,
                      bin_sint32_le//1,
                      bin_uint64_le//1,
                      bin_sint64_le//1,
                      bin_uint16_be//1,
                      bin_sint16_be//1,
                      bin_uint32_be//1,
                      bin_sint32_be//1,
                      bin_uint64_be//1,
                      bin_sint64_be//1,

                      % Signed/unsigned conversion
                      bin_conv_u8_s8/2,
                      bin_conv_u16_s16/2,
                      bin_conv_u32_s32/2,
                      bin_conv_u64_s64/2,

                      % Endianness utilities
                      bin_swap16/2,
                      bin_swap32/2,
                      bin_swap64/2,
                      bin_conv_u16_le/2,
                      bin_conv_u32_le/2,
                      bin_conv_u64_le/2,
                      bin_conv_u16_be/2,
                      bin_conv_u32_be/2,
                      bin_conv_u64_be/2,
                      bin_conv_s16_le/2,
                      bin_conv_s32_le/2,
                      bin_conv_s64_le/2,
                      bin_conv_s16_be/2,
                      bin_conv_s32_be/2,
                      bin_conv_s64_be/2,
                      htons/2,
                      htonl/2,
                      ntohs/2,
                      ntohl/2]).
:- use_foreign_library(foreign(binformat4pl)).

%!  bin_uint8(?Int)// is semidet.
%!  bin_sint8(?Int)// is semidet.
%!  bin_uint16(?Int)// is semidet.
%!  bin_sint16(?Int)// is semidet.
%!  bin_uint32(?Int)// is semidet.
%!  bin_sint32(?Int)// is semidet.
%!  bin_uint64(?Int)// is semidet.
%!  bin_sint64(?Int)// is semidet.
%
%   DCG nonterminals that convert between Prolog integers and their binary
%   representations. These predicates all use *native* byte ordering; for
%   explicit orderings (of multibyte integers) append `_be` or `_le` to the
%   predicate name, for example bin_uint16_le//1 or bin_sint64_be//1.
%
%   These predicates (along with their endian variants) will silently truncate
%   out-of-range integers when operating in (+)// mode.
%
%   @param Int is a Prolog integer.


%!  bin_uint16_le(?Int)// is semidet.
%!  bin_sint16_le(?Int)// is semidet.
%!  bin_uint32_le(?Int)// is semidet.
%!  bin_sint32_le(?Int)// is semidet.
%!  bin_uint64_le(?Int)// is semidet.
%!  bin_sint64_le(?Int)// is semidet.
%!  bin_uint16_be(?Int)// is semidet.
%!  bin_sint16_be(?Int)// is semidet.
%!  bin_uint32_be(?Int)// is semidet.
%!  bin_sint32_be(?Int)// is semidet.
%!  bin_uint64_be(?Int)// is semidet.
%!  bin_sint64_be(?Int)// is semidet.
%
%   DCG nonterminals that convert between Prolog integers and binary
%   representations with explicit byte ordering, rather than host-native
%   byte ordering. See e.g. bin_uint16//1 for more details.


%!  bin_conv_u8_s8(?UInt, ?SInt) is det.
%!  bin_conv_u16_s16(?UInt, ?SInt) is det.
%!  bin_conv_u32_s32(?UInt, ?SInt) is det.
%!  bin_conv_u64_s64(?UInt, ?SInt) is det.
%
%   These predicates are true when UInt is a non-negative integer that can be
%   represented in the given number of bits, and SInt is a signed integer that
%   can be represented in twos-complement form with the given number of bits,
%   and both of these bit-representations are identical. Will throw a
%   domain_error if any of these are out of range.


%!  bin_swap16(+Int, -Swapped) is det.
%!  bin_swap32(+Int, -Swapped) is det.
%!  bin_swap64(+Int, -Swapped) is det.
%
%   These predicates perform unconditional byte-swapping of fixed-size integer
%   representations. You probably don't want to use these; the bin_conv16_le/2
%   family converts between a given endianness and this Prolog build's native
%   format, which is usually more useful.
%
%   @param Int is a Prolog integer representing a fixed-size native integer. It
%          must be ground.
%   @param Swapped is a Prolog integer representing a byte-swapped version of Int.


%!  bin_conv_u16_le(?Int, ?LE) is det.
%!  bin_conv_u32_le(?Int, ?LE) is det.
%!  bin_conv_u64_le(?Int, ?LE) is det.
%!  bin_conv_u16_be(?Int, ?BE) is det.
%!  bin_conv_u32_be(?Int, ?BE) is det.
%!  bin_conv_u64_be(?Int, ?BE) is det.
%
%!  bin_conv_s16_le(?Int, ?LE) is det.
%!  bin_conv_s32_le(?Int, ?LE) is det.
%!  bin_conv_s64_le(?Int, ?LE) is det.
%!  bin_conv_s16_be(?Int, ?BE) is det.
%!  bin_conv_s32_be(?Int, ?BE) is det.
%!  bin_conv_s64_be(?Int, ?BE) is det.
%
%   These predicates are true when Int is an integer that can be represented in
%   the given number of bits (unsigned or signed) and LE (or BE) is the result
%   of representing the same integer in little-endian (or big-endian) form, then
%   reading it back as though it were a native integer of the given length and
%   the same signedness. Will throw a domain_error if a value is outside the
%   representable range for that length/signedness.
%
%   Internally, half of these are mapped to the bin_swap16/2 family (but with
%   appropriate checks and signedness handling), while the others are mapped
%   to (=)/2 (with the same checks).


%!  htons(?Int, ?NetInt) is det.
%!  htonl(?Int, ?NetInt) is det.
%!  ntohs(?NetInt, ?Int) is det.
%!  ntohl(?NetInt, ?Int) is det.
%
%   These are convenience predicates, mapped to bin_conv_u16_be/2 and
%   bin_conv_u32_be/2.