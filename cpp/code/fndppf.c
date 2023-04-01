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
/*  Module:     fndppf.c                                                */
/*----------------------------------------------------------------------*/
/*  Memory Usage:														*/
/*		ROM: 				4											*/
/*		Static/Global RAM: 	45											*/
/*		Stack/Local RAM: 	12											*/
/*----------------------------------------------------------------------*/
/************************************************************************
* Routine name: fndppf                                                  *
* Function: forward pitch predictor.		                        *
* Inputs:   buf    - data buffer.                                       *
*           dmin   - minimum delay value.                               *
*           dmax   - maximum delay value.                               *
*           prevdelay - previous frame delay value.                     *
*           length - size of pitch window.                              *
* Outputs:  delay  - predicted delay.                                   *
*           beta   - gain value.                                        *
*                                                                       *
************************************************************************/
#include "e_globs.h"
#include "macro.h"

void    fndppf(float *delay, float *beta, float *buf, short dmin, short dmax, short length)
{
	static float b = -0.312;/* rom storage */
	static float a[3] =
	{-2.2875, 1.956, -0.5959};	/* rom storage */

	short   dnew=DMIN;
	float   sum;
	register int m, i, n;
	static float DECbuf[FrameSize / 4];
	float   corrmax, cmax, tap1, tmp;
	short   M1, M2, dnewtmp=DMIN;
	static short lastgoodpitch = 0;
	static float lastbeta = 0.0;
	static float memory[3];
	static int FirstTime = 1;

	/* init static variables (should be in init routine for implementation) */
	if (FirstTime)
	{
		FirstTime = 0;
		for (i = 0; i < FrameSize / 4; i++)
			DECbuf[i] = 0.0;
		memory[0] = memory[1] = memory[2] = 0.0;
	}

	/* Shift memory of DECbuf */
	for (i = 0; i < length / 8; i++)
		DECbuf[i] = DECbuf[i + length / 8];

	/* filter signal and decimate */
	for (i = 0, n = length / 8; i < length / 2; i++)
	{
		sum = buf[i + length / 2] - a[0] * memory[0] - a[1] * memory[1] - a[2] * memory[2];
		if ((i + 1) % 4 == 0)
			DECbuf[n++] = sum + memory[0] * b + memory[1] * b + memory[2];
		memory[2] = memory[1];
		memory[1] = memory[0];
		memory[0] = sum;
	}

	/* perform first search for best delay value in decimated domain */
	corrmax = -1.0e30;
	for (m = dmin / 4; m <= dmax / 4; m++)
	{
		for (i = 0, sum = 0.0; i < length / 4 - m; i++)
			sum += DECbuf[i] * DECbuf[i + m];
		if (sum > corrmax)
		{		/* Gives some bias to low delays */
			corrmax = sum;
			dnew = m;
		}
	}

	/* Compare against lastgoodpitch */
	if (lastgoodpitch != 0 && abs(lastgoodpitch - dnew * 4) > 2)
	{
		M1 = lastgoodpitch / 4 - 2;
		if (M1 < dmin / 4)
			M1 = dmin / 4;
		M2 = M1 + 4;
		if (M2 > dmax / 4)
			M2 = dmax / 4;

		cmax = -1.0e30;
		for (m = M1; m <= M2; m++)
		{
			for (i = 0, sum = 0.0; i < length / 4 - m; i++)
				sum += DECbuf[i] * DECbuf[i + m];
			if (sum > cmax)
			{	/* Gives some bias to low delays */
				cmax = sum;
				dnewtmp = m;
			}
		}
		if (cmax > 0.835 * corrmax)
			dnew = dnewtmp;
	}

	/* perform first search for best delay value in non-decimated buffer */
	M1 = Max(4 * dnew - 3, dmin);
	if (M1 < dmin)
		M1 = dmin;
	M2 = Min(4 * dnew + 3, dmax);
	if (M2 > dmax)
		M2 = dmax;
	corrmax = -1.0e30;
	for (m = M1; m <= M2; m++)
	{
		for (i = 0, sum = 0.0; i < length - m; i++)
			sum += buf[i] * buf[i + m];
		if (sum > corrmax)
		{
			corrmax = sum;
			dnew = m;
		}
	}

	for (i = 0, sum = 0.0; i < length - dnew; i++)
		sum += buf[i + dnew] * buf[i + dnew];
	for (i = 0, cmax = 0.0; i < length - dnew; i++)
		cmax += buf[i] * buf[i];
	sum = sqrt(sum * cmax);

	if (sum == 0)
		*beta = 0;
	else
		*beta = corrmax / sum;
	if (*beta > 1.0)
		*beta = 1.0;
	if (*beta < 0)
		*beta = 0.0;

	/* perform search for best delay value in around old pitch delay */
	if (lastgoodpitch != 0)
	{
		M1 = lastgoodpitch - 6;
		M2 = lastgoodpitch + 6;

		if (M1 < dmin)
			M1 = dmin;
		if (M2 > dmax)
			M2 = dmax;

		if (dnew > M2 || dnew < M1)
		{
			cmax = -1.0e30;
			for (m = M1; m <= M2; m++)
			{
				for (i = 0, sum = 0.0; i < length - m; i++)
					sum += buf[i] * buf[i + m];
				if (sum > cmax)
				{
					cmax = sum;
					dnewtmp = m;
				}
			}

			for (i = 0, tmp = 0.0; i < length - dnewtmp; i++)
				tmp += buf[i + dnewtmp] * buf[i + dnewtmp];
			for (i = 0, sum = 0.0; i < length - dnewtmp; i++)
				sum += buf[i] * buf[i];
			tmp = sqrt(sum * tmp);

			if (tmp == 0)
				tap1 = 0.0;
			else
				tap1 = cmax / tmp;
			if (tap1 > 1.0)
				tap1 = 1.0;
			if (tap1 < 0)
				tap1 = 0.0;

			/* Replace dnew with dnewtmp if tap1 is large enough */
			if ((dnew > M2 && tap1 > 0.6 * *beta) ||
			    (dnew < M1 && tap1 > 1.2 * *beta))
			{
				dnew = dnewtmp;
				*beta = tap1;
			}
		}
	}

	*delay = (float) dnew;
	if (*beta > 0.4)
	{
		lastgoodpitch = dnew;
		lastbeta = *beta;
	}
	else
	{
		lastbeta *= 0.75;
		if (lastbeta < 0.3)
			lastgoodpitch = 0;
	}
}
