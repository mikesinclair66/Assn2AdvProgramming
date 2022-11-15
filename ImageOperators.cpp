#include "ImageOperators.h"
#include <emmintrin.h>
#include <iostream>

#pragma warning(disable:4018) // signed/unsigned mismatch

#define EMMX_BLEND(comp) \
	t = _mm_loadu_si128((__m128i *) pDst[(comp)]);                      \
	d0 = _mm_unpacklo_epi8(t, zero);                                    \
	d1 = _mm_unpackhi_epi8(t, zero);                                    \
	t = _mm_loadu_si128((__m128i *) pSrc[(comp)]);                      \
	_mm_storeu_si128((__m128i *) pDst[(comp)], _mm_packus_epi16(_mm_srli_epi16(_mm_add_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(t, zero), a0), _mm_mullo_epi16(_mm_sub_epi16(ff, a0), d0)), 8), _mm_srli_epi16(_mm_add_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(t, zero), a1), _mm_mullo_epi16(_mm_sub_epi16(ff, a1), d1)), 8)))                      

//src - bubble image, dst - background, dstXOffset - x coord, dstYOffset - y coord
void blitBlend( UCImg &src, UCImg &dst, unsigned int dstXOffset, unsigned int dstYOffset, SimdMode simdMode)
{
	if (src.spectrum() != 4) throw cimg_library::CImgException("blitBlend: Src image is missing ALPHA channel");

	// calcualte our SIMD blend area (defined by X0, Y0 to X1, Y1). Take into account alignment restrictions;

	//X1: bubble image width + offset; basically the end x coordinate of the bubble
	unsigned int X1 = src.width() + dstXOffset;
	if (X1 > dst.width()) X1 = dst.width();
	//X0: start x coordinate of the bubble
	unsigned int X0 = dstXOffset;

	//Y1: endY of the bubble
	unsigned int Y1 = src.height() + dstYOffset;
	if (Y1 > dst.height()) Y1 = dst.height();

	//Y0: startY of the bubble
	unsigned int Y0 = dstYOffset;
	// TODO: Y0 & Y1 need to be aligned.


	for (unsigned int y = Y0, srcLine = 0; y < Y1; y++, srcLine++) {
		unsigned char *pSrc[4];
		pSrc[0] = src.data(0, srcLine, 0, 0);
		pSrc[1] = src.data(0, srcLine, 0, 1);
		pSrc[2] = src.data(0, srcLine, 0, 2);
		pSrc[3] = src.data(0, srcLine, 0, 3);

		unsigned char *pDst[4];
		pDst[0] = dst.data(X0, y, 0, 0);
		pDst[1] = dst.data(X0, y, 0, 1);
		pDst[2] = dst.data(X0, y, 0, 2);

		short ffconst[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
		//*** __m128i maps to 128-bit XMM registers
		__m128i ff = _mm_loadu_si128((__m128i *)ffconst);

		//populates the window with pure assembly; fastest.
		if (simdMode == SIMD_EMMX) {
			for (unsigned x = X0; x < X1; x += 16) {
				__asm {
					/*
					Some notes:
					-movdqu: assign double quad word (128bit) to destination
					regardless of operand location size.
					-movdqa: assign double quad word (128bit) to destination
					where both operands are of equal location size
					-Since each location stores a byte, it takes 4 index traversals
					to get from pSrc[2] to pSrc[3] since each element of pSrc is
					32-bit (dword). Hence [pSrc + 12] = pSrc[3]
					*/
					pxor xmm0, xmm0 // xmm0 <- 0
					// assign memory location of 32-bit (double-word)
					// value pointed to is pSrc[3]
					mov eax, dword ptr [pSrc + 12]
					movdqu xmm1, [eax]; // xmm1 <- *pSrc[3]
					movdqa xmm2, xmm1; // xmm2 <- pSrc[3]
					//note that xmm0 == 0; some sort of conversion below to 16-bit
					punpcklbw xmm2, xmm0; // xmm2 <- a0, 16bit
					movdqa xmm3, xmm1; // xmm3 <- pSrc[3]
					punpckhbw xmm3, xmm0; // xxm3 <- a1, 16bit

					// blending the red;
					// load d0
					//assign memory location of 32-bit (double-word)
					//value pointed to is pSrc[3]
					mov eax, dword ptr[pDst + 0]; 
					movdqu xmm1, [eax]; // xmm1 = pDst[0]
					movdqa xmm6, xmm1; // xmm6 = pDst[0]
					punpcklbw xmm6, xmm0; // xmm6 <- pDst[0] low 16bit
					movdqa xmm7, xmm1; // xmm7 - pDst[0]
					punpckhbw xmm7, xmm0; // xmm7 <- pDst[0] high, 16 bit
					// load the ff constant
					movdqu xmm4, [ffconst]; // xmm4 <- ff
					movdqa xmm5, xmm4; // xmm5 = ff
					psubw  xmm5, xmm2; // xmm5 = ff - a0
					pmullw xmm6, xmm5; // xmm6 = (ff - a0) * d0;
					// now for the upper bits
					movdqa xmm5, xmm4; // xmm5 = ff
					psubw  xmm5, xmm3; // xmm5 = ff - a1
					pmullw xmm7, xmm5; // xmm7 = (ff - a1) * d1;
					// load the source;
					//assign memory location of 32-bit (double-word)
					//value pointed to is pSrc[0]
					mov eax, dword ptr[pSrc + 0];
					movdqu xmm1, [eax]; // xmm1 = pSrc[0]
					// low bits of pSrc[0]
					movdqa xmm5, xmm1; // xmm5 = pSrc[0]
					punpcklbw xmm5, xmm0; // xmm5 = pSrc[0], low, 16 bit;
					pmullw xmm5, xmm2; // xmm5 = s0 * a0;
					paddw xmm6, xmm5; // xmm6 = s0 * a0 + (ff - a0) * d0;
					// high bits of pSrc[0]
					movdqa xmm5, xmm1; // xmm5 = xmm1
					punpckhbw xmm5, xmm0; // xmm5 <- pDst[0] high, 16 bit
					pmullw xmm5, xmm3; // xmm5 = s1 * a1
					paddw xmm7, xmm5; // xmm7 = s1 * a1 + (ff - a1) * d1;
					// shift the results;
					psrlw xmm6, 8;//slide 8 bits to the right on xmm6 data
					psrlw xmm7, 8;//slide 8 bits to the right on xmm7 data
					// pack back
					packuswb xmm6, xmm7; // xmm6 <- xmm6{}xmm7 low bits;
					mov eax, dword ptr [pDst + 0]; // assign 32-bit word pDst[0] to eax
					movdqu [eax], xmm6; // done for this component;

					// blending the green;
					// load d0
					mov eax, dword ptr[pDst + 4]; // eax = 32-bit location with value pDst[1]
					movdqu xmm1, [eax]; // xmm1 = pDst[0]
					movdqa xmm6, xmm1; // xmm6 = pDst[0]
					punpcklbw xmm6, xmm0; // xmm6 <- pDst[0] low 16bit
					movdqa xmm7, xmm1; // xmm7 = pDst[0]
					punpckhbw xmm7, xmm0; // xmm7 <- pDst[0] high, 16 bit
					// load the ff constant
					movdqu xmm4, [ffconst]; // xmm4 <- ff
					movdqa xmm5, xmm4; // xmm5 = ff
					psubw  xmm5, xmm2; // xmm5 = ff - a0
					pmullw xmm6, xmm5; // xmm6 = (ff - a0) * d0;
					// now for the upper bits
					movdqa xmm5, xmm4; // xmm5 = ff
					psubw  xmm5, xmm3; // xmm5 = ff - a1
					pmullw xmm7, xmm5; // xmm7 = (ff - a1) * d1;
					// load the source;
					mov eax, dword ptr[pSrc + 4];
					movdqu xmm1, [eax]; // xmm1 = pSrc[0]
					// low bits of pSrc[0]
					movdqa xmm5, xmm1;
					punpcklbw xmm5, xmm0; // xmm5 = pSrc[0], low, 16 bit;
					pmullw xmm5, xmm2; // xmm5 = s0 * a0;
					paddw xmm6, xmm5; // xmm6 = s0 * a0 + (ff - a0) * d0;
					// high bits of pSrc[0]
					movdqa xmm5, xmm1;
					punpckhbw xmm5, xmm0;
					pmullw xmm5, xmm3; // xmm5 = s1 * a1
					paddw xmm7, xmm5; // xmm7 = s1 * a1 + (ff - a1) * d1;
					// shift the results;
					psrlw xmm6, 8;
					psrlw xmm7, 8;
					// pack back
					packuswb xmm6, xmm7; // xmm6 <- xmm6{}xmm7 low bits;
					mov eax, dword ptr [pDst + 4];
					movdqu [eax], xmm6; // done for this component;

					// blending the blue;
					// load d0
					mov eax, dword ptr[pDst + 8]; 
					movdqu xmm1, [eax]; // xmm1 = pDst[0]
					movdqa xmm6, xmm1;
					punpcklbw xmm6, xmm0; // xmm6 <- pDst[0] low 16bit
					movdqa xmm7, xmm1;
					punpckhbw xmm7, xmm0; // xmm7 <- pDst[0] high, 16 bit
					// load the ff constant
					movdqu xmm4, [ffconst]; // xmm4 <- ff
					movdqa xmm5, xmm4; 
					psubw  xmm5, xmm2; // xmm5 = ff - a0
					pmullw xmm6, xmm5; // xmm6 = (ff - a0) * d0;
					// now for the upper bits
					movdqa xmm5, xmm4;
					psubw  xmm5, xmm3; // xmm5 = ff - a1
					pmullw xmm7, xmm5; // xmm7 = (ff - a1) * d1;
					// load the source;
					mov eax, dword ptr[pSrc + 8];
					movdqu xmm1, [eax]; // xmm1 = pSrc[0]
					// low bits of pSrc[0]
					movdqa xmm5, xmm1; // xmm5 = pSrc[0]
					punpcklbw xmm5, xmm0; // xmm5 = pSrc[0], low, 16 bit;
					pmullw xmm5, xmm2; // xmm5 = s0 * a0;
					paddw xmm6, xmm5; // xmm6 = s0 * a0 + (ff - a0) * d0;
					// high bits of pSrc[0]
					movdqa xmm5, xmm1;
					punpckhbw xmm5, xmm0;
					pmullw xmm5, xmm3; // xmm5 = s1 * a1
					paddw xmm7, xmm5; // xmm7 = s1 * a1 + (ff - a1) * d1;
					// shift the results;
					psrlw xmm6, 8;
					psrlw xmm7, 8;
					// pack back
					packuswb xmm6, xmm7; // xmm6 <- xmm6{}xmm7 low bits;
					mov eax, dword ptr [pDst + 8];
					movdqu [eax], xmm6; // done for this component;
				};
				pSrc[0] += 16;
				pSrc[1] += 16;
				pSrc[2] += 16;
				pSrc[3] += 16;

				pDst[0] += 16;
				pDst[1] += 16;
				pDst[2] += 16;
			}
		//populates the window without use of assembly.
		} else if (simdMode == SIMD_NONE) {
			for (unsigned int x = X0; x < X1; x++) {
				short diff;
				short tmp;

				diff = *pSrc[0] - *pDst[0];
				tmp = short(*pSrc[3] * diff) >> 8;
				*pDst[0] = tmp + *pDst[0];

				diff = *pSrc[1] - *pDst[1];
				tmp = short(*pSrc[3] * diff) >> 8;
				*pDst[1] = tmp + *pDst[1];

				diff = *pSrc[2] - *pDst[2];
				tmp = short(*pSrc[3] * diff) >> 8;
				*pDst[2] = tmp + *pDst[2];

				pSrc[0] += 1;
				pSrc[1] += 1;
				pSrc[2] += 1;
				pSrc[3] += 1;

				pDst[0] += 1;
				pDst[1] += 1;
				pDst[2] += 1;
			}
		//populates the window with a c++ based assembly.
		} else if (simdMode == SIMD_EMMX_INTRINSICS) {

			for (unsigned x = X0; x < X1; x += 16) {
				register __m128i s0, s1, d0, d1, a0, a1, r0, r1, zero;
				register __m128i diff0, tmp0, diff1, tmp1, t;
				zero = _mm_setzero_si128();
				// load alpha
				t = _mm_loadu_si128((__m128i *) pSrc[3]);
				a0 = _mm_unpacklo_epi8(t, zero);
				a1 = _mm_unpackhi_epi8(t, zero);

			    EMMX_BLEND(0);
				EMMX_BLEND(1);
				EMMX_BLEND(2);

				pSrc[0] += 16;
				pSrc[1] += 16;
				pSrc[2] += 16;
				pSrc[3] += 16;

				pDst[0] += 16;
				pDst[1] += 16;
				pDst[2] += 16;
			}
		}

	}
}
