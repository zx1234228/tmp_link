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
/*  Module:     apf.c                                                   */
/*----------------------------------------------------------------------*/
/*  Memory Usage:														*/
/*		ROM: 				0											*/
/*		Static/Global RAM: 	203											*/
/*		Stack/Local RAM: 	156											*/
/*----------------------------------------------------------------------*/
/* Adaptive post filter */
#include "macro.h"

void    apf(float *in, float *coeff, float *out, float delayi, float alpha,
	    float beta, float u, float agc, float ltgain, short order, short length, short br)
{
	static int FirstTime = 1;

	static float FIRmem[ORDER];	/* FIR filter memory */
	static float IIRmem[ORDER];	/* IIR filter memory */
	static float last;
	static float Residual[ACBMemSize + SubFrameSize];	/* local residual */

	float   wcoef1[ORDER];
	float   wcoef2[ORDER];
	float   scratch[SubFrameSize];
	float   temp[SubFrameSize];
	float   mem[ORDER];
	float   sum1, sum2, gamma, APFgain;
	short   i, j, n, best;

	/* initialization -- should be done in init routine for implementation */
	if (FirstTime)
	{
		FirstTime = 0;
		for (i = 0; i < ORDER; i++)
			FIRmem[i] = 0.0;
		for (i = 0; i < ORDER; i++)
			IIRmem[i] = 0.0;
		for (i = 0; i < ACBMemSize; i++)
			Residual[i] = 0.0;
		last = 0;
	}

	/* Compute weighted LPC coefficients */
	weight(wcoef1, coeff, alpha, order);
	weight(wcoef2, coeff, beta, order);

	/* Tilt speech  */

	/*...no tilt in non-voiced regions...*/
	for (i = 0, sum2 = 0.0; i < length - 1; i++)
		sum2 += in[i] * in[i + 1];	/*...crosscorr...*/
	if ((sum2) < 0.0)
		u = 0.0;	/*...no tilt...*/

	for (i = 0; i < length; i++)
	{
		scratch[i] = in[i] - u * last;
		last = in[i];
	}

	/* Compute  residual */
	fir(Residual + ACBMemSize, scratch, wcoef1, FIRmem, order, length);

	/* long term filtering */
	/* Find best integer delay around delayi */
	j = (short) (delayi + 0.5);
	sum1 = 0;
	best = j;
	for (i = Max(DMIN, j - 3); i <= Min(DMAX, j + 3); i++)
	{
		for (n = ACBMemSize, sum2 = 0; n < ACBMemSize + length; n++)
			sum2 += Residual[n] * Residual[n - i];
		if (sum2 > sum1)
		{
			sum1 = sum2;
			best = i;
		}
	}

	/* Get beta for delayi */
	for (i = ACBMemSize, sum1 = 0; i < ACBMemSize + length; i++)
		sum1 += Residual[i - best] * Residual[i - best];
	for (i = ACBMemSize, sum2 = 0; i < ACBMemSize + length; i++)
		sum2 += Residual[i] * Residual[i - best];

	if (sum2 * sum1 == 0 || br == 1)
		for (i = 0; i < length; i++)
			temp[i] = Residual[i + ACBMemSize];
	else
	{
		gamma = sum2 / sum1;
		if (gamma < 0.5)
			for (i = 0; i < length; i++)
				temp[i] = Residual[i + ACBMemSize];
		else
		{
			if (gamma > 1.0)
				gamma = 1.0;	/* Clip gamma at 1.0 */

			/* Do actual filtering */
			for (i = 0; i < length; i++)
			{
				temp[i] = Residual[ACBMemSize + i] +
				    gamma * ltgain * Residual[ACBMemSize + i - best];
			}
		}
	}

	/* iir short term filter - first run */
	for (i = 0; i < length; i++)
		scratch[i] = temp[i];
	for (i = 0; i < order; i++)
		mem[i] = IIRmem[i];
	iir(scratch, scratch, wcoef2, mem, order, length);

	/* Get filter gain */
	for (i = 0, sum1 = 0, sum2 = 0; i < length; i++)
	{
		sum1 += in[i] * in[i];
		sum2 += scratch[i] * scratch[i];
	}
	if (sum2 != 0)
		APFgain = sqrt(sum1 / sum2);
	else
		APFgain = 1.0;

	/* Re-normalize the speech signal */
	for (i = 0; i < length; i++)
		temp[i] *= APFgain;

	/* iir short term filter - second run */
	iir(out, temp, wcoef2, IIRmem, order, length);

	/* Update residual buffer */
	for (i = 0; i < ACBMemSize; i++)
		Residual[i] = Residual[i + length];
}
