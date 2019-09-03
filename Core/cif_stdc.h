/* vim:set ts=4 sw=4 noet: */
//  Copyright (c) 2007-2008, 2014 byArvind Umrao.

#ifndef _cif_stdc_h
#define _cif_stdc_h 1





#ifdef __GNUC__
# define __A_UNUSED__			__attribute__((unused))
# define __A_FORMAT__(t,nf,np)	__attribute__((format (t, nf, np)))
# define __A_NORETURN__			__attribute__((noreturn))
#else
# define __A_UNUSED__
# define __A_FORMAT__(t,nf,np)
# define __A_NORETURN__
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <unistd.h>
#include <stdlib.h>

#ifdef __linux__
# include <endian.h>
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define CIF_LITTLE_ENDIAN 1
# else
#  define CIF_BIG_ENDIAN 1
# endif
#else /* Default to Solaris */
# include <sys/isa_defs.h>
# ifdef _LITTLE_ENDIAN
#  define CIF_LITTLE_ENDIAN 1
# else
#  define CIF_BIG_ENDIAN 1
# endif
#endif /* __linux__ */

#include <string.h>
#include <inttypes.h>
#include <stdio.h>

#ifndef __HERE__
# define __HERE__  __FILE__, __LINE__
#endif

#endif /* !_cif_stdc_h */
