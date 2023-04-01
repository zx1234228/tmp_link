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
/*  Module:     mdfyorig.c                                              */
/*----------------------------------------------------------------------*/
/*  Memory Usage:														*/
/*		ROM: 				0											*/
/*		Static/Global RAM: 	24											*/
/*		Stack/Local RAM: 	141											*/
/*----------------------------------------------------------------------*/
/* Modify the original residual to match the TARGET residual buffer */
#include "e_globs.h"
#include "macro.h"

void    modifyorig(float *residualm, float *accshift, float beta, short *dpm,
		   short shiftrange, short resolution, float *TARGET, float *residual,
		   short dp, short sfend)
{
	static short FirstTime = 1;
	short   best;
	float   y, tmp;
	float   ex0y[(RSHIFT * 2 + 2) * RRESOLUTION + 1];	/* Fraction sampled correlation function */
	float   ex0y2[RSHIFT * 2 + 2];	/* Integer sampled correlation function */
	float   Residual[2 * SubFrameSize + 2 * RSHIFT + 1];
	static float a1[RRESOLUTION];
	static float a2[RRESOLUTION];
	static float a3[RRESOLUTION];
	register short i, j, k, n;
	short   sfstart, length, shiftrangel, shiftranger;
	float   e01, e02;

	/****** INITIALIZATION *****/
	if (FirstTime)
	{
		FirstTime = 0;
		/* Calculate interpolation coefficients */
		tmp = 1.0 / (float) resolution;
		y = -(float) (resolution / 2) * tmp;
		for (i = 0; i < resolution; i++)
		{
			a1[i] = (y * y - y) / 2.0;
			a2[i] = (1.0 - y * y);
			a3[i] = (y * y + y) / 2.0;
			y += tmp;
		}
	}

	/********************
	* CORRELATION MATCH *
	********************/
	length = sfend - dp;
	sfstart = dp;

	if (shiftrange != 0)
	{
		/* Limit the search range to control accshift */
		shiftrangel = shiftranger = shiftrange;
		if (*accshift < 0)
			shiftrangel += 1;
		if (*accshift > 0)
			shiftranger += 1;
		/* For non-periodic signals */
		if ((beta < 0.2 && fabs(*accshift) > 15.0) || (beta < 0.3 && fabs(*accshift) > 30.0))
        {
			if (*accshift < 0)
				shiftranger = 1;
			else
				shiftrangel = 1;
        }

		if ((shiftrangel + (short) *accshift) > GUARD - BLPRECISION)
		{
			shiftrangel = GUARD - BLPRECISION - (short) *accshift;
			fprintf(stderr, "mdfyorig:*** Buffer limit. shiftrangel is:%d\n", shiftrangel);
		}
		if ((shiftranger - (short) *accshift) > (GUARD - BLPRECISION))
		{
			shiftranger = (GUARD - BLPRECISION) + (short) *accshift;
			fprintf(stderr, "mdfyorig:*** Buffer limit. shiftranger is:%d\n", shiftranger);
		}

		/* Create a buffer of modify residual for match at low cut-off frequency */
		for (i = 0; i <= length + shiftrangel + shiftranger; i++)
			bl_intrp(Residual + i, residual + dp + i, *accshift + shiftrangel, 0.5, 3);

		/* Search for all integer delays of residual */
		for (n = 0; n <= shiftrangel + shiftranger; n++)
		{
			ex0y2[n] = 0.0;
			for (i = 0; i < length; i++)
			{
				ex0y2[n] += Residual[n + i] * TARGET[sfstart + i];
			}
		}

		/* Do quadratic interpolation of ex0y */
		for (n = 1, k = 0; n < shiftrangel + shiftranger; n++)
		{
			for (j = 0; j < resolution; j++)
			{
				ex0y[k++] = a1[j] * ex0y2[n - 1] + a2[j] * ex0y2[n] + a3[j] * ex0y2[n + 1];
			}
		}

		/* Find maximum with positive correlation */
		y = 0.0;
		best = resolution * shiftrangel - RRESOLUTION / 2;
		for (n = 0; n < k; n++)
		{
			if (ex0y[n] > y)
			{
				y = ex0y[n];
				best = n;
			}
		}

		/* Calculate energy in selected shift index */
		e01 = e02 = 0.0;
		for (i = shiftrangel; i < length + shiftrangel; i++)
			e01 += Residual[i] * Residual[i];
		for (i = 0; i < length; i++)
			e02 += TARGET[i + sfstart] * TARGET[i + sfstart];
		if (e01 * e02 == 0)
			y = 0.0;
		else
			y = y / sqrt(e02 * e01);
		if (y > 0.7)
			*accshift -= (float) (best - resolution * shiftrangel + RRESOLUTION / 2) / (float) resolution;
	}

	for (k = 0; k < length; k++)
		bl_intrp(residualm + dp + k, residual + dp + k, *accshift, BLFREQ, BLPRECISION);

	*dpm = dp + length;
}
