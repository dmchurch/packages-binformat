/*  Part of SWI-Prolog

    Author:        Danielle Church
    E-mail:        dani.church@gmail.com
    WWW:           http://www.swi-prolog.org
    Copyright (c)  2021, Second Star Solutions
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in
       the documentation and/or other materials provided with the
       distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

#include <SWI-Stream.h>
#include <SWI-Prolog.h>

#include <config.h>

#ifdef WORDS_BIGENDIAN
# define BE_NATIVE(name) name ## _native
# define LE_NATIVE(name) name ## _swapped
#else
# define BE_NATIVE(name) name ## _swapped
# define LE_NATIVE(name) name ## _native
#endif

#define TRY(call) do {if (!(call)) PL_fail;} while (0)
#define IN_RANGE(i,sign,width,culprit) (((i) < TYPE_LO(sign,width) || (i) > TYPE_HI(sign,width)) ? PL_domain_error(#sign "int" #width, culprit) : TRUE)

#define TYPE_LO(sign, width) TYPE_LO_##sign (width)
#define TYPE_HI(sign, width) TYPE_HI_##sign (width)

#define TYPE_LO_s(width) (-(TYPE_HI_s(width)) - 1)
#define TYPE_HI_s(width) ((int64_t)((1ULL << (width-1)) - 1))
#define TYPE_LO_u(width) (0)
#define TYPE_HI_u(width) ((((uint64_t)TYPE_HI_s(width))<<1)+1)

#define INIT_native(var) ((unsigned char*)&var);
#define INIT_swapped(var) (((unsigned char*)&var) + sizeof(var));
#define INCR_native(var) (*var++)
#define INCR_swapped(var) (*--var)
#define PL2C_integer int
#define PL2C_uint64 uint64_t
#define PL2C_int64 int64_t
#define CTYP_s(bits) int##bits##_t
#define CTYP_u(bits) uint##bits##_t


		 /*******************************
		 *	 DCG NONTERMINALS	*
		 *******************************/

#define BIN_INT_FUNC(signed, bits, pltyp, order) \
static foreign_t \
bin_##signed##int##bits##_##order(term_t i, term_t a, term_t b) \
{ PL2C_##pltyp value; \
  CTYP_##signed(bits) rep; \
  unsigned char *repcodes = INIT_##order(rep); \
  term_t list = PL_copy_term_ref(a), \
	 head = PL_new_term_ref(); \
  if (!PL_is_variable(i)) \
  { TRY(PL_get_##pltyp##_ex(i, &value)); \
    rep = value; \
    for (int n = 0; n < sizeof(rep); n++) \
    { TRY(PL_unify_list_ex(list, head, list) \
	&&PL_unify_integer(head, INCR_##order(repcodes))); \
    } \
    return PL_unify(list, b); \
  } else \
  { for (int n = 0, code=0; n < sizeof(rep); n++) \
    { TRY(PL_unify_list_ex(list, head, list) \
	&&PL_get_integer_ex(head, &code) \
	&&IN_RANGE(code, u, 8, head)); \
      INCR_##order(repcodes) = (unsigned char)code; \
    } \
    return PL_unify_##pltyp(i, rep) \
	&& PL_unify(list, b); \
  } \
}

BIN_INT_FUNC(u, 8, integer, native)
BIN_INT_FUNC(s, 8, integer, native)

BIN_INT_FUNC(u, 16, integer, native)
BIN_INT_FUNC(u, 16, integer, swapped)
BIN_INT_FUNC(s, 16, integer, native)
BIN_INT_FUNC(s, 16, integer, swapped)
BIN_INT_FUNC(u, 32, uint64, native)
BIN_INT_FUNC(u, 32, uint64, swapped)
BIN_INT_FUNC(s, 32, int64, native)
BIN_INT_FUNC(s, 32, int64, swapped)
BIN_INT_FUNC(u, 64, uint64, native)
BIN_INT_FUNC(u, 64, uint64, swapped)
BIN_INT_FUNC(s, 64, int64, native)
BIN_INT_FUNC(s, 64, int64, swapped)


		 /*******************************
		 *	 SIGN CONVERSION	*
		 *******************************/

#define BIN_CONV_SIGN_FUNC(bits, plutyp, plstyp) \
static foreign_t \
bin_conv_u##bits##_s##bits(term_t u, term_t s) \
{ if (!PL_is_variable(u)) \
  { PL2C_##plutyp value; \
    return PL_get_##plutyp##_ex(u, &value) \
	&& PL_unify_##plstyp(s, (CTYP_s(bits))(CTYP_u(bits))value); \
  } else \
  { PL2C_##plstyp value; \
    return PL_get_##plstyp##_ex(s, &value) \
  	&& PL_unify_##plutyp(u, (CTYP_u(bits))(CTYP_s(bits))value); \
  } \
}
BIN_CONV_SIGN_FUNC(8, integer, integer)
BIN_CONV_SIGN_FUNC(16, integer, integer)
BIN_CONV_SIGN_FUNC(32, uint64, int64)
BIN_CONV_SIGN_FUNC(64, uint64, int64)


		 /*******************************
		 *	  BYTE SWAPPING		*
		 *******************************/

#define SWAP16(value)	( ((value & (0xFF << 0)) << 8) \
			| ((value & (0xFF << 8)) >> 8) )
#define SWAP32(value)	( ((value & (0xFFUL << 0)) << 24)  \
			| ((value & (0xFFUL << 8)) << 8)   \
			| ((value & (0xFFUL << 16)) >> 8)  \
			| ((value & (0xFFUL << 24)) >> 24) )
#define SWAP64(value)	( ((value & (0xFFULL << 0)) << 56)  \
			| ((value & (0xFFULL << 8)) << 40)  \
			| ((value & (0xFFULL << 16)) << 24) \
			| ((value & (0xFFULL << 24)) << 8)  \
			| ((value & (0xFFULL << 32)) >> 8)  \
			| ((value & (0xFFULL << 40)) >> 24) \
			| ((value & (0xFFULL << 48)) >> 40) \
			| ((value & (0xFFULL << 56)) >> 56) )

#define BIN_SWAP_FUNC(bits, pltyp) \
static foreign_t \
bin_swap##bits(term_t native, term_t swapped) \
{ PL2C_##pltyp value; \
  return PL_get_##pltyp##_ex(native, &value) \
      && PL_unify_##pltyp(swapped, SWAP##bits(value)); \
}

BIN_SWAP_FUNC(16, integer)
BIN_SWAP_FUNC(32, int64)
BIN_SWAP_FUNC(64, uint64)

#define BIN_CONV_NATIVE_FUNC(signed, bits, pltyp) \
static foreign_t \
bin_conv_##signed##bits##_native(term_t a1, term_t a2) \
{ PL2C_##pltyp value; \
  if (!PL_is_variable(a1)) \
    return PL_get_##pltyp##_ex(a1, &value) \
	&& IN_RANGE(value, signed, bits, a1) \
	&& PL_unify_##pltyp(a2, value); \
  else \
    return PL_get_##pltyp##_ex(a2, &value) \
	&& IN_RANGE(value, signed, bits, a2) \
	&& PL_unify_##pltyp(a1, value); \
}

#define BIN_CONV_SWAP_FUNC(signed, bits, pltyp) \
static foreign_t \
bin_conv_##signed##bits##_swapped(term_t a1, term_t a2) \
{ PL2C_##pltyp value; \
  if (!PL_is_variable(a1)) \
    return PL_get_##pltyp##_ex(a1, &value) \
	&& IN_RANGE(value, signed, bits, a1) \
	&& PL_unify_##pltyp(a2, SWAP##bits(value)); \
  else \
    return PL_get_##pltyp##_ex(a2, &value) \
	&& IN_RANGE(value, signed, bits, a2) \
	&& PL_unify_##pltyp(a1, SWAP##bits(value)); \
}

BIN_CONV_NATIVE_FUNC(u, 16, integer)
BIN_CONV_NATIVE_FUNC(u, 32, int64)
BIN_CONV_NATIVE_FUNC(u, 64, uint64)
BIN_CONV_NATIVE_FUNC(s, 16, integer)
BIN_CONV_NATIVE_FUNC(s, 32, int64)
BIN_CONV_NATIVE_FUNC(s, 64, int64)

BIN_CONV_SWAP_FUNC(u, 16, integer)
BIN_CONV_SWAP_FUNC(u, 32, int64)
BIN_CONV_SWAP_FUNC(u, 64, uint64)
BIN_CONV_SWAP_FUNC(s, 16, integer)
BIN_CONV_SWAP_FUNC(s, 32, int64)
BIN_CONV_SWAP_FUNC(s, 64, int64)

		 /*******************************
		 *	      REGISTER		*
		 *******************************/

#define MKATOM(n) \
        ATOM_ ## n = PL_new_atom(#n)
#define MKFUNCTOR(n,a) \
        FUNCTOR_ ## n ## a = PL_new_functor(PL_new_atom(#n), a)

install_t
install_binformat4pl(void)
{ PL_register_foreign("bin_uint8", 3, bin_uint8_native, 0);
  PL_register_foreign("bin_sint8", 3, bin_sint8_native, 0);
  PL_register_foreign("bin_uint16", 3, bin_uint16_native, 0);
  PL_register_foreign("bin_sint16", 3, bin_sint16_native, 0);
  PL_register_foreign("bin_uint32", 3, bin_uint32_native, 0);
  PL_register_foreign("bin_sint32", 3, bin_sint32_native, 0);
  PL_register_foreign("bin_uint64", 3, bin_uint64_native, 0);
  PL_register_foreign("bin_sint64", 3, bin_sint64_native, 0);
  PL_register_foreign("bin_uint16_le", 3, LE_NATIVE(bin_uint16), 0);
  PL_register_foreign("bin_sint16_le", 3, LE_NATIVE(bin_sint16), 0);
  PL_register_foreign("bin_uint32_le", 3, LE_NATIVE(bin_uint32), 0);
  PL_register_foreign("bin_sint32_le", 3, LE_NATIVE(bin_sint32), 0);
  PL_register_foreign("bin_uint64_le", 3, LE_NATIVE(bin_uint64), 0);
  PL_register_foreign("bin_sint64_le", 3, LE_NATIVE(bin_sint64), 0);
  PL_register_foreign("bin_uint16_be", 3, BE_NATIVE(bin_uint16), 0);
  PL_register_foreign("bin_sint16_be", 3, BE_NATIVE(bin_sint16), 0);
  PL_register_foreign("bin_uint32_be", 3, BE_NATIVE(bin_uint32), 0);
  PL_register_foreign("bin_sint32_be", 3, BE_NATIVE(bin_sint32), 0);
  PL_register_foreign("bin_uint64_be", 3, BE_NATIVE(bin_uint64), 0);
  PL_register_foreign("bin_sint64_be", 3, BE_NATIVE(bin_sint64), 0);
  PL_register_foreign("bin_conv_u8_s8", 2, bin_conv_u8_s8, 0);
  PL_register_foreign("bin_conv_u16_s16", 2, bin_conv_u16_s16, 0);
  PL_register_foreign("bin_conv_u32_s32", 2, bin_conv_u32_s32, 0);
  PL_register_foreign("bin_conv_u64_s64", 2, bin_conv_u64_s64, 0);
  PL_register_foreign("bin_swap16", 2, bin_swap16, 0);
  PL_register_foreign("bin_swap32", 2, bin_swap32, 0);
  PL_register_foreign("bin_swap64", 2, bin_swap64, 0);
  PL_register_foreign("bin_conv_u16_le", 2, LE_NATIVE(bin_conv_u16), 0);
  PL_register_foreign("bin_conv_u32_le", 2, LE_NATIVE(bin_conv_u32), 0);
  PL_register_foreign("bin_conv_u64_le", 2, LE_NATIVE(bin_conv_u64), 0);
  PL_register_foreign("bin_conv_s16_le", 2, LE_NATIVE(bin_conv_s16), 0);
  PL_register_foreign("bin_conv_s32_le", 2, LE_NATIVE(bin_conv_s32), 0);
  PL_register_foreign("bin_conv_s64_le", 2, LE_NATIVE(bin_conv_s64), 0);
  PL_register_foreign("bin_conv_u16_be", 2, BE_NATIVE(bin_conv_u16), 0);
  PL_register_foreign("bin_conv_u32_be", 2, BE_NATIVE(bin_conv_u32), 0);
  PL_register_foreign("bin_conv_u64_be", 2, BE_NATIVE(bin_conv_u64), 0);
  PL_register_foreign("bin_conv_s16_be", 2, BE_NATIVE(bin_conv_s16), 0);
  PL_register_foreign("bin_conv_s32_be", 2, BE_NATIVE(bin_conv_s32), 0);
  PL_register_foreign("bin_conv_s64_be", 2, BE_NATIVE(bin_conv_s64), 0);
  PL_register_foreign("htons", 2, BE_NATIVE(bin_conv_u16), 0);
  PL_register_foreign("ntohs", 2, BE_NATIVE(bin_conv_u16), 0);
  PL_register_foreign("htonl", 2, BE_NATIVE(bin_conv_u32), 0);
  PL_register_foreign("ntohl", 2, BE_NATIVE(bin_conv_u32), 0);
}
