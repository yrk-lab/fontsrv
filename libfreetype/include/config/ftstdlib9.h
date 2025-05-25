/***************************************************************************/
/*                                                                         */
/*  ftstdlib.h                                                             */
/*                                                                         */
/*    ANSI-specific library and header configuration file (specification   */
/*    only).                                                               */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This file is used to group all #includes to the ANSI C library that   */
  /* FreeType normally requires.  It also defines macros to rename the     */
  /* standard functions within the FreeType source code.                   */
  /*                                                                       */
  /* Load a file which defines __FTSTDLIB_H__ before this one to override  */
  /* it.                                                                   */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FTSTDLIB_H__
#define __FTSTDLIB_H__


  /**********************************************************************/
  /*                                                                    */
  /*                           integer limits                           */
  /*                                                                    */
  /* UINT_MAX and ULONG_MAX are used to automatically compute the size  */
  /* of `int' and `long' in bytes at compile-time.  So far, this works  */
  /* for all platforms the library has been tested on.                  */
  /*                                                                    */
  /* Note that on the extremely rare platforms that do not provide      */
  /* integer types that are _exactly_ 16 and 32 bits wide (e.g. some    */
  /* old Crays where `int' is 36 bits), we do not make any guarantee    */
  /* about the correct behaviour of FT2 with all fonts.                 */
  /*                                                                    */
  /* In these case, "ftconfig.h" will refuse to compile anyway with a   */
  /* message like "couldn't find 32-bit type" or something similar.     */
  /*                                                                    */
  /* IMPORTANT NOTE: We do not define aliases for heap management and   */
  /*                 i/o routines (i.e. malloc/free/fopen/fread/...)    */
  /*                 since these functions should all be encapsulated   */
  /*                 by platform-specific implementations of            */
  /*                 "ftsystem.c".                                      */
  /*                                                                    */
  /**********************************************************************/

#define FT_UINT_MAX   0xFFFFFFFFUL
#define FT_ULONG_MAX  0xFFFFFFFFUL
#define ULONG_MAX  0xFFFFFFFFUL
#define FT_CHAR_BIT    8
#define FT_INT_MIN     -0x7FFFFFFFL
#define FT_INT_MAX     0x7FFFFFFFL
#define INT_MAX     0x7FFFFFFFL
#define SHRT_MAX  0x7FFF
#define FT_USHORT_MAX  0xFFFF
#define FT_LONG_MIN    -0x7FFFFFFFL
#define FT_LONG_MAX    0x7FFFFFFFL
/*
#define FT_UINT_MAX    UINT_MAX
#define FT_ULONG_MAX   ULONG_MAX
*/

#ifdef FT2_BUILD_LIBRARY
#include <u.h>
#include <libc.h>

#define ABS(x)	((x)>0? (x) : -(x))

  /**********************************************************************/
  /*                                                                    */
  /*                 character and string processing                    */
  /*                                                                    */
  /**********************************************************************/


/*
#include <ctype.h>
#define ft_isalnum  isalnum
#define ft_isupper  isupper
#define ft_islower  islower
#define ft_xdigit   isxdigit
*/

#define ft_memchr   memchr
#define ft_memcmp   memcmp
#define ft_memcpy   memcpy
#define ft_memmove  memmove
#define ft_memset   memset
#define ft_strcat   strcat
#define ft_strcmp   strcmp
#define ft_strcpy   strcpy
#define ft_strlen   strlen
#define ft_strncmp  strncmp
#define ft_strncpy  strncpy
#define ft_strrchr  strrchr
#define ft_strstr   strstr


  /**********************************************************************/
  /*                                                                    */
  /*                           file handling                            */
  /*                                                                    */
  /**********************************************************************/

#include <stdio.h>

#define FT_FILE     FILE
#define ft_fclose   fclose
#define ft_fopen    fopen
#define ft_fread    fread
#define ft_fseek    fseek
#define ft_ftell    ftell
#define ft_sprintf  sprintf


  /**********************************************************************/
  /*                                                                    */
  /*                             sorting                                */
  /*                                                                    */
  /**********************************************************************/

#define ft_qsort  qsort

  /**********************************************************************/
  /*                                                                    */
  /*                        memory allocation                           */
  /*                                                                    */
  /**********************************************************************/


#define ft_scalloc   calloc
#define ft_sfree     free
#define ft_smalloc   malloc
#define ft_srealloc  realloc


  /**********************************************************************/
  /*                                                                    */
  /*                          miscellaneous                             */
  /*                                                                    */
  /**********************************************************************/


#define ft_strtol  strtol
#define ft_getenv  getenv

  /**********************************************************************/
  /*                                                                    */
  /*                         execution control                          */
  /*                                                                    */
  /**********************************************************************/



#define ft_jmp_buf  jmp_buf   /* note: this cannot be a typedef since */
                              /*       jmp_buf is defined as a macro  */
                              /*       on certain platforms           */

#define ft_setjmp   setjmp    /* same thing here */
#define ft_longjmp  longjmp   /* "               */


  /* the following is only used for debugging purposes, i.e. when */
  /* FT_DEBUG_LEVEL_ERROR or FT_DEBUG_LEVEL_TRACE are defined     */
  /*                                                              */


#endif /* __FTSTDLIB_H__ */

#endif /* FT2_BUILD_LIBRARY */

/* END */
