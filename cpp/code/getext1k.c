/**********************************************************************
Each of the companies; Lucent, Motorola, Nokia, and Qualcomm (hereinafter 
referred to individually as "Source" or collectively as "Sources") do 
hereby state:

To the extent to which the Source(s) may legally and freely do so, the 
Source(s), upon submission of a Contribution, grant(s) a free, 
irrevocable, non-exclusive, license to the Third Generation Partnership 
Project 2 (3GPP2) and its Organizational Partners: ARIB, CCSA, TIA, TTA, 
and TTC, under the Source's copyright or copyright license rights in the 
Contribution, to, in whole or in part, copy, make derivative works, 
perform, display and distribute the Contribution and derivative works 
thereof consistent with 3GPP2's and each Organizational Partner's 
policies and procedures, with the right to (i) sublicense the foregoing 
rights consistent with 3GPP2's and each Organizational Partner's  policies 
and procedures and (ii) copyright and sell, if applicable) in 3GPP2's name 
or each Organizational Partner's name any 3GPP2 or transposed Publication 
even though this Publication may contain the Contribution or a derivative 
work thereof.  The Contribution shall disclose any known limitations on 
the Source's rights to license as herein provided.

When a Contribution is submitted by the Source(s) to assist the 
formulating groups of 3GPP2 or any of its Organizational Partners, it 
is proposed to the Committee as a basis for discussion and is not to 
be construed as a binding proposal on the Source(s).  The Source(s) 
specifically reserve(s) the right to amend or modify the material 
contained in the Contribution. Nothing contained in the Contribution 
shall, except as herein expressly provided, be construed as conferring 
by implication, estoppel or otherwise, any license or right under (i) 
any existing or later issuing patent, whether or not the use of 
information in the document necessarily employs an invention of any 
existing or later issued patent, (ii) any copyright, (iii) any 
trademark, or (iv) any other intellectual property right.

With respect to the Software necessary for the practice of any or 
all Normative portions of the Enhanced Variable Rate Codec (EVRC) as 
it exists on the date of submittal of this form, should the EVRC be 
approved as a Specification or Report by 3GPP2, or as a transposed 
Standard by any of the 3GPP2's Organizational Partners, the Source(s) 
state(s) that a worldwide license to reproduce, use and distribute the 
Software, the license rights to which are held by the Source(s), will 
be made available to applicants under terms and conditions that are 
reasonable and non-discriminatory, which may include monetary compensation, 
and only to the extent necessary for the practice of any or all of the 
Normative portions of the EVRC or the field of use of practice of the 
EVRC Specification, Report, or Standard.  The statement contained above 
is irrevocable and shall be binding upon the Source(s).  In the event 
the rights of the Source(s) in and to copyright or copyright license 
rights subject to such commitment are assigned or transferred, the 
Source(s) shall notify the assignee or transferee of the existence of 
such commitments.
*******************************************************************/
 
/*======================================================================*/
/*     Enhanced Variable Rate Codec - Master C code Specification       */
/*     Copyright (C) 1997-1998 Telecommunications Industry Association. */
/*     All rights reserved.                                             */
/*----------------------------------------------------------------------*/
/* Note:  Reproduction and use of this software for the design and      */
/*     development of North American Wideband CDMA Digital              */
/*     Cellular Telephony Standards is authorized by the TIA.           */
/*     The TIA does not authorize the use of this software for any      */
/*     other purpose.                                                   */
/*                                                                      */
/*     The availability of this software does not provide any license   */
/*     by implication, estoppel, or otherwise under any patent rights   */
/*     of TIA member companies or others covering any use of the        */
/*     contents herein.                                                 */
/*                                                                      */
/*     Any copies of this software or derivative works must include     */
/*     this and all other proprietary notices.                          */
/*======================================================================*/
/*  Module:     getext1k.c                                              */
/*----------------------------------------------------------------------*/
/*  Memory Usage:														*/
/*		ROM: 				0											*/
/*		Static/Global RAM: 	9											*/
/*		Stack/Local RAM: 	4											*/
/*----------------------------------------------------------------------*/

/****************************************************************************
* Routine name: GetExc800bps.                                               *
* Function: Energy quantization of the residual signal.                     *
* Inputs: input - signal array.                                             *
*         length - size of signal array.                                    *
* Output: output - quantized signal.                                        *
****************************************************************************/
#include "macro.h"
#include "rom.h"

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK1 23148373

float   ran0(long *seed0)
{
	long    k;
	float   ans;

	*seed0 ^= MASK1;
	k = (*seed0) / IQ;
	*seed0 = IA * (*seed0 - k * IQ) - IR * k;
	if (*seed0 < 0)
		*seed0 += IM;
	ans = AM * (*seed0);
	*seed0 ^= MASK1;
	return (ans);
}

float   ran_g(long *seed0)
{
	static int iset = 0;
	static float gset;
	float   fac, rsq, v1, v2;

	if (iset == 0)
	{
		do
		{
			v1 = 2.0 * ran0(seed0) - 1.0;
			v2 = 2.0 * ran0(seed0) - 1.0;
			rsq = v1 * v1 + v2 * v2;
		}
		while (rsq >= 1.0 || rsq == 0.0);
		fac = sqrt(-2.0 * log(rsq) / rsq);
		gset = v1 * fac;
		iset = 1;
		return (v2 * fac);
	}
	else
	{
		iset = 0;
		return (gset);
	}
}

void    GetExc800bps(float *output, short *best, float scale, float *input, short length, short flag, short n)
{
	short   i, j;
	float   sum, tmp, D, *ptr;
	static long Seed;
	static float Sum[NoOfSubFrames];

	if (!flag)
		Seed = 1234;

	/* Get energy of next sub frame */
	for (i = 0, sum = 0; i < length; i++)
		sum += fabs(input[i]);

	sum = sum / (float) SubFrameSize;
	if (sum < 1)
		sum = 1;
	sum = log10(sum / scale);	/*...fix for noise puffs...*/

	Sum[n] = sum;

	/* Quantize if last frame */
	if (n == NoOfSubFrames - 1)
	{
		/* Quantize to 8 bits */
		for (i = 0, sum = 1e30, ptr = Logqtbl; i < 256; i++)
		{
			for (j = 0, tmp = 0; j < 3; j++)
			{
				D = Sum[j] - *ptr++;
				tmp += D * D;
			}
			if (tmp < sum)
			{
				sum = tmp;
				*best = i;
			}
		}

		for (j = 0; j < 3; j++)
			Sum[j] = Logqtbl[*best * 3 + j];

		/* Convert to linear domain */
		for (j = 0; j < 3; j++)
			Sum[j] = pow(10, Sum[j]);

		/* Get excitation */
		j = FrameSize - ACBMemSize;
		for (i = 0; i < FrameSize - 1; i++)
		{
			tmp = ran_g(&Seed);
			if (i >= j)
				output[i - j] = Sum[i / (length - 1)] * tmp;
		}
		output[i - j] = Sum[2] * ran_g(&Seed);	/* last excitation */
	}

}

void    GetExc800bps_dec(float *output, short length, short best, short flag, short n, short fer_flag)
{
	short   i, j;
	float   sum;
	static long Seed;
	static float Sum[NoOfSubFrames];
	static short PrevBest = 0;

	if (!flag && !n)
		Seed = 1234;

	if (n == 0)
	{
		/* De-quantize */
		if (fer_flag == 0)
		{
			for (j = 0; j < 3; j++)
				Sum[j] = Logqtbl[best * 3 + j];
			PrevBest = best;
		}
		else
		{
			for (j = 0, sum = 0; j < 3; j++)
				sum += Logqtbl[PrevBest * 3 + j];
			sum /= 3.0;
			for (j = 0; j < 3; j++)
				Sum[j] = sum;
		}
	}

	/* Convert to linear domain */
	sum = pow(10, Sum[n]);

	for (i = 0; i < length; i++)
	{
		output[i] = sum * ran_g(&Seed);
	}

}
