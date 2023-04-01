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
/*  Module:     a2lsp.c                                                 */
/*----------------------------------------------------------------------*/
/*  Memory Usage:														*/
/*		ROM: 				4											*/
/*		Static/Global RAM: 	0											*/
/*		Stack/Local RAM: 	21											*/
/*----------------------------------------------------------------------*/

/*
 * pctolsp - convert pc to lsp
 *
 * NOTES: 1. This routine is hardwired for 10th order
 *        2. The routine uses 3 stage uniform grid quantization of lsp.
 */
#include "macro.h"

#define STEPSNUM 4

void    a2lsp(float *freq, float *a)
{
	static float STEPS[4] =
	{0.00635, 0.003175, 0.0015875, 0.00079375};	/* rom storage */
	int     lspnumber;
	int     root, notlast;
	float   temp, temp0, temp1, temp2;
	float   q[10];
	float   prev[2];
	int     offset;
	int     iswitch;
	float   frequency, LastFreq;
	float   STEP;
	int     STEPindex;

	LastFreq = 0;

	/* calculate q[z] and p[z] , they are all stored in q*/

	q[0] = a[0] + a[9] - 1.0;
	q[1] = a[1] + a[8] - q[0];
	q[2] = a[2] + a[7] - q[1];
	q[3] = a[3] + a[6] - q[2];
	q[4] = a[4] + a[5] - q[3];
	q[4] = q[4] / 2;

	offset = 5;
	q[0 + offset] = a[0] - a[9] + 1.0;
	q[1 + offset] = a[1] - a[8] + q[0 + offset];
	q[2 + offset] = a[2] - a[7] + q[1 + offset];
	q[3 + offset] = a[3] - a[6] + q[2 + offset];
	q[4 + offset] = a[4] - a[5] + q[3 + offset];
	q[4 + offset] = q[4 + offset] / 2;

	prev[0] = 9e9;
	prev[1] = 9e9;
	lspnumber = 0;
	notlast = TRUE;
	iswitch = 0;
	frequency = 0;

	while (notlast)
	{

		root = TRUE;
		offset = iswitch * 5;
		STEPindex = 0;	/* Start with low resolution grid */
		STEP = STEPS[STEPindex];

		while (root)
		{

			temp = cos(frequency * 6.2832);
			temp1 = 2.0 * temp + q[0 + offset];
			temp2 = 2.0 * temp * temp1 - 1.0 + q[1 + offset];
			temp0 = 2.0 * temp * temp2 - temp1 + q[2 + offset];
			temp1 = 2.0 * temp * temp0 - temp2 + q[3 + offset];
			temp2 = temp * temp1 - temp0 + q[4 + offset];

			if ((temp2 * prev[iswitch]) <= 0.0 || frequency >= 0.5)
			{
				if (STEPindex == STEPSNUM - 1)
				{
					if (fabs(temp2) < fabs(prev[iswitch]))
					{
						freq[lspnumber] = frequency;
					}
					else
					{
						freq[lspnumber] = frequency - STEP;
					}

					if ((prev[iswitch]) < 0.0)
					{
						prev[iswitch] = 9e9;
					}
					else
					{
						prev[iswitch] = -9e9;
					}
					root = FALSE;
					frequency = LastFreq;
					STEPindex = 0;
				}
				else
				{
					if (STEPindex == 0)
					{
						LastFreq = frequency;
					}
					frequency -= STEPS[++STEPindex];	/* Go back one grid step */
					STEP = STEPS[STEPindex];
				}
			}
			else
			{
				prev[iswitch] = temp2;
				frequency += STEP;
			}
		}

		lspnumber++;
		if (lspnumber > ORDER - 1)
			notlast = FALSE;
		iswitch = 1 - iswitch;
	}
}
