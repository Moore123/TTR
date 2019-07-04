/*
   LZ4 HC - High Compression Mode of LZ4
   Copyright (C) 2011-2012, Yann Collet.
   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   - LZ4 homepage : http://fastcompression.blogspot.com/p/lz4.html
   - LZ4 source repository : http://code.google.com/p/lz4/
*/  
	 
//**************************************
// CPU Feature Detection
//**************************************
// 32 or 64 bits ?
#if (defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(__ppc64__) || defined(_WIN64) || defined(__LP64__) || defined(_LP64) )	// Detects 64 bits mode
#define LZ4_ARCH64 1
#else	/* 
#define LZ4_ARCH64 0
#endif	/* 
	 
// Little Endian or Big Endian ? 
#if (defined(__BIG_ENDIAN__) || defined(__BIG_ENDIAN) || defined(_BIG_ENDIAN) || defined(_ARCH_PPC) || defined(__PPC__) || defined(__PPC) || defined(PPC) || defined(__powerpc__) || defined(__powerpc) || defined(powerpc) || ((defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))) )
#define LZ4_BIG_ENDIAN 1
#else	/* 
// Little Endian assumed. PDP Endian and other very rare endian format are unsupported.
#endif	/* 
	 
// Unaligned memory access is automatically enabled for "common" CPU, such as x86.
// For others CPU, the compiler will be more cautious, and insert extra code to ensure aligned access is respected
// If you know your target CPU supports unaligned memory access, you may want to force this option manually to improve performance
#if defined(__ARM_FEATURE_UNALIGNED)
#define LZ4_FORCE_UNALIGNED_ACCESS 1
#endif	/* 
	 
//**************************************
// Compiler Options
//**************************************
#if __STDC_VERSION__ >= 199901L // C99
	 /* "restrict" is a known keyword */ 
#else	/* 
#define restrict					  // Disable restrict
#endif	/* 
	 
#ifdef _MSC_VER
#define inline __forceinline	  // Visual is not C99, but supports some kind of inline
#endif	/* 
	 
#ifdef _MSC_VER					  // Visual Studio
#define bswap16(x) _byteswap_ushort(x)
#else	/* 
#define bswap16(x)  ((unsigned short int) ((((x) >> 8) & 0xffu) | (((x) & 0xffu) << 8)))
#endif	/* 
	 
//**************************************
// Includes
//**************************************
#include <stdlib.h>				  // calloc, free
#include <string.h>				  // memset, memcpy
#include "lz4hc.h"
	 
#define ALLOCATOR(s) calloc(1,s)
#define FREEMEM free
#define MEM_INIT memset
	 
//**************************************
// Basic Types
//**************************************
#if defined(_MSC_VER)			  // Visual Studio does not support 'stdint' natively
#define BYTE	unsigned __int8
#define U16		unsigned __int16
#define U32		unsigned __int32
#define S32		__int32
#define U64		unsigned __int64
#else	/* 
#include <stdint.h>
#define BYTE	uint8_t
#define U16		uint16_t
#define U32		uint32_t
#define S32		int32_t
#define U64		uint64_t
#endif	/* 
	 
#ifndef LZ4_FORCE_UNALIGNED_ACCESS
#pragma pack(push, 1) 
#endif	/* 
	 
	U16 v;
} U16_S;

	U32 v;
} U32_S;

	U64 v;
} U64_S;

#ifndef LZ4_FORCE_UNALIGNED_ACCESS
#pragma pack(pop) 
#endif	/* 
	 
#define A64(x) (((U64_S *)(x))->v)
#define A32(x) (((U32_S *)(x))->v)
#define A16(x) (((U16_S *)(x))->v)
	 
//**************************************
// Constants
//**************************************
#define MINMATCH 4
	 
#define DICTIONARY_LOGSIZE 16
#define MAXD (1<<DICTIONARY_LOGSIZE)
#define MAXD_MASK ((U32)(MAXD - 1))
#define MAX_DISTANCE (MAXD - 1)
	 
#define HASH_LOG (DICTIONARY_LOGSIZE-1)
#define HASHTABLESIZE (1 << HASH_LOG)
#define HASH_MASK (HASHTABLESIZE - 1)
	 
#define MAX_NB_ATTEMPTS 256
	 
#define ML_BITS  4
#define ML_MASK  (size_t)((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)
	 
#define COPYLENGTH 8
#define LASTLITERALS 5
#define MFLIMIT (COPYLENGTH+MINMATCH)
#define MINLENGTH (MFLIMIT+1)
#define OPTIMAL_ML (int)((ML_MASK-1)+MINMATCH)
	 
//**************************************
// Architecture-specific macros
//**************************************
#if LZ4_ARCH64						  // 64-bit
#define STEPSIZE 8
#define LZ4_COPYSTEP(s,d)		A64(d) = A64(s); d+=8; s+=8;
#define LZ4_COPYPACKET(s,d)		LZ4_COPYSTEP(s,d)
#define UARCH U64
#define AARCH A64
#define HTYPE					U32
#define INITBASE(b,s)			const BYTE* const b = s
#else									  // 32-bit
#define STEPSIZE 4
#define LZ4_COPYSTEP(s,d)		A32(d) = A32(s); d+=4; s+=4;
#define LZ4_COPYPACKET(s,d)		LZ4_COPYSTEP(s,d); LZ4_COPYSTEP(s,d);
#define UARCH U32
#define AARCH A32
#define HTYPE					const BYTE*
#define INITBASE(b,s)		    const int b = 0
#endif	/* 
	 
#if defined(LZ4_BIG_ENDIAN)
#define LZ4_READ_LITTLEENDIAN_16(d,s,p) { U16 v = A16(p); v = bswap16(v); d = (s) - v; }
#define LZ4_WRITE_LITTLEENDIAN_16(p,i)  { U16 v = (U16)(i); v = bswap16(v); A16(p) = v; p+=2; }
#else									  // Little Endian
#define LZ4_READ_LITTLEENDIAN_16(d,s,p) { d = (s) - A16(p); }
#define LZ4_WRITE_LITTLEENDIAN_16(p,v)  { A16(p) = v; p+=2; }
#endif	/* 
	 
//************************************************************
// Local Types
//************************************************************
typedef struct 
	
	
	
	


//**************************************
// Macros
//**************************************
#define LZ4_WILDCOPY(s,d,e)		do { LZ4_COPYPACKET(s,d) } while (d<e);
#define LZ4_BLINDCOPY(s,d,l)	{ BYTE* e=d+l; LZ4_WILDCOPY(s,d,e); d=e; }
#define HASH_FUNCTION(i)	(((i) * 2654435761U) >> ((MINMATCH*8)-HASH_LOG))
#define HASH_VALUE(p)		HASH_FUNCTION(*(U32*)(p))
#define HASH_POINTER(p)		(HashTable[HASH_VALUE(p)] + base)
#define DELTANEXT(p)		chainTable[(size_t)(p) & MAXD_MASK] 
#define GETNEXT(p)			((p) - (size_t)DELTANEXT(p))
#define ADD_HASH(p)			{ size_t delta = (p) - HASH_POINTER(p); if (delta>MAX_DISTANCE) delta = MAX_DISTANCE; DELTANEXT(p) = (U16)delta; HashTable[HASH_VALUE(p)] = (p) - base; }
	 
//**************************************
// Private functions
//**************************************
#if LZ4_ARCH64

{
	
#if defined(LZ4_BIG_ENDIAN)
#if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
	unsigned long r = 0;
	
	
	
#elif defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 304) && !defined(LZ4_FORCE_SW_BITCOUNT)
		 return (__builtin_clzll(val) >> 3);
	
#else	/* 
	int r;
	
		r = 4;
	} else {
		r = 0;
		val >>= 32;
	}
	
		r += 2;
		val >>= 8;
	} else {
		val >>= 24;
	}
	
	
	
#endif	/* 
#else	/* 
#if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
	unsigned long r = 0;
	
	
	
#elif defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 304) && !defined(LZ4_FORCE_SW_BITCOUNT)
		 return (__builtin_ctzll(val) >> 3);
	
#else	/* 
	static const int DeBruijnBytePos[64] =
		 { 0, 0, 0, 0, 0, 1, 1, 2, 0, 3, 1, 3, 1, 4, 2, 7, 0, 2, 3, 6, 1, 5, 3, 5, 1, 3, 4, 4, 2, 5, 6, 7, 7, 0, 1, 2, 3,
3, 4, 6, 2, 6, 5, 5, 3, 4, 5, 6, 7, 1, 2, 4, 6, 4, 4, 5, 7, 2, 6, 5, 7, 6, 7, 7 };
	
	
#endif	/* 
#endif	/* 
}


#else	/* 

{
	
#if defined(LZ4_BIG_ENDIAN)
#if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
	unsigned long r = 0;
	
	
	
#elif defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 304) && !defined(LZ4_FORCE_SW_BITCOUNT)
		 return (__builtin_clz(val) >> 3);
	
#else	/* 
	int r;
	
		r = 2;
		val >>= 8;
	} else {
		r = 0;
		val >>= 24;
	}
	
	
	
#endif	/* 
#else	/* 
#if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
	unsigned long r = 0;
	
	
	
#elif defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 304) && !defined(LZ4_FORCE_SW_BITCOUNT)
		 return (__builtin_ctz(val) >> 3);
	
#else	/* 
	static const int DeBruijnBytePos[32] =
		 { 0, 0, 3, 0, 3, 1, 3, 0, 3, 2, 2, 1, 3, 2, 0, 1, 3, 3, 1, 2, 2, 2, 2, 0, 3, 1, 2, 0, 1, 0, 1, 1 };
	
	
#endif	/* 
#endif	/* 
}


#endif	/* 

{
	
	
	
	
	



{
	
	
	



{
	
	
	



{
	
	
	
	
		
		
		
		



																  const BYTE * const matchlimit, const BYTE ** matchpos) 
{
	
	
	
	
	
	
	
		 // HC4 match finder
		 LZ4HC_Insert(hc4, ip);
	
	
		
		
		
			
				
				
				
				
					
					
					
						ipt += STEPSIZE;
						reft += STEPSIZE;
						continue;
					}
					
					
					
				
					if ((ipt < (matchlimit - 3)) && (A32(reft) == A32(ipt))) {
						ipt += 4;
						reft += 4;
					}
				
					ipt += 2;
					reft += 2;
				}
				
					ipt++;
			 
					ml = ipt - ip;
					*matchpos = ref;
				}
				
		
		
	



																  const BYTE * matchlimit, int longest, const BYTE ** matchpos,
																  const BYTE ** startpos) 
{
	
	
	
	
	
	
	
		 // First Match
		 LZ4HC_Insert(hc4, ip);
	
	
		
		
		
			
				
				
				
				
				
					
					
					
						ipt += STEPSIZE;
						reft += STEPSIZE;
						continue;
					}
					
					
					
				
					if ((ipt < (matchlimit - 3)) && (A32(reft) == A32(ipt))) {
						ipt += 4;
						reft += 4;
					}
				
					ipt += 2;
					reft += 2;
				}
				
					ipt++;
			 
				
					startt--;
					reft--;
				}
				
					
					
					
					
					
				
		
		
	



{
	
	
	
		 // Encode Literal length
		 length = *ip - *anchor;
	
	
		*token = (RUN_MASK << ML_BITS);
		len = length - RUN_MASK;
		for (; len > 254; len -= 255)
			*(*op)++ = 255;
		*(*op)++ = (BYTE) len;
	}
	
	else
		*token = (length << ML_BITS);
	
		 // Copy Literals
		 LZ4_BLINDCOPY(*anchor, *op, length);
	
		 // Encode Offset
		 LZ4_WRITE_LITTLEENDIAN_16(*op, *ip - ref);
	
		 // Encode MatchLength
		 len = (int)(ml - MINMATCH);
	
		*token += ML_MASK;
		len -= ML_MASK;
		for (; len > 509; len -= 510) {
			*(*op)++ = 255;
			*(*op)++ = 255;
		}
		if (len > 254) {
			len -= 255;
			*(*op)++ = 255;
		}
		*(*op)++ = (BYTE) len;
	}
	
	else
		*token += len;
	
		 // Prepare next loop
		 *ip += ml;
	
	



//****************************
// Compression CODE
//****************************

{
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
		 // Main Loop
		 while (ip < mflimit)
		
		
		
			ip++;
			continue;
		}
		
			 // saved, in case we would skip too much
			 start0 = ip;
		
		
	 
			
		
		else
			ml2 = ml;
		
		{
			
			
		
		
			
			
			{
				
				
				
			
			
		
			 // Here, start0==ip
			 if ((start2 - ip) < 3)	// First Match too small : removed
		{
			
			
			
			
		
	 
			 // Currently we have :
			 // ml2 > ml1, and
			 // ip1+3 <= ip2 (usually < ip1+ml1)
			 if ((start2 - ip) < OPTIMAL_ML)
			
			
			
			
				new_ml = OPTIMAL_ML;
			
				new_ml = start2 - ip + ml2 - MINMATCH;
			
			
				
				
				
				
				
			
		
			 // Now, we have start2 = ip+new_ml, with new_ml=min(ml, OPTIMAL_ML=18)
			 
			
		
		else
			ml3 = ml2;
		
		{
			
				 // ip & ref are known; Now for ml
				 if (start2 < ip + ml)
				
				
					
					
					
						ml = OPTIMAL_ML;
					
						ml = start2 - ip + ml2 - MINMATCH;
					
					
						
						
						
						
						
					
				
				else
					
					
					
				
			
				 // Now, encode 2 sequences
				 LZ4_encodeSequence(&ip, &op, &anchor, ml, ref);
			
			
			
		
		
		{
			
			{
				
					
					
					
					
					
					
						
						
						
						
						
					
				
				
				
				
				
				
				
				
			
			
			
			
			
		
		
			 // OK, now we have 3 ascending matches; let's write at least the first one
			 // ip & ref are known; Now for ml
			 if (start2 < ip + ml)
			
			
				
				
				
					ml = OPTIMAL_ML;
				
					ml = start2 - ip + ml2 - MINMATCH;
				
				
					
					
					
					
					
				
			
			else
				
				
				
			
		
		
		
		
		
		
		
		
		
	
		 // Encode Last Literals
	{
		
		
			*op++ = (RUN_MASK << ML_BITS);
			lastRun -= RUN_MASK;
			for (; lastRun > 254; lastRun -= 255)
				*op++ = 255;
			*op++ = (BYTE) lastRun;
		}
		
		else
			*op++ = (lastRun << ML_BITS);
		
		
	
	
		 // End
		 return (int)(((char *)op) - dest);

{
	
	
	
	


