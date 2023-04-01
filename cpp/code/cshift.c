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
/*  Module:     cshift.c                                                */
/*----------------------------------------------------------------------*/
/*  Memory Usage:														*/
/*		ROM: 				0											*/
/*		Static/Global RAM: 	0											*/
/*		Stack/Local RAM: 	7											*/
/*----------------------------------------------------------------------*/
/* Find residual pulse frame for shifting */
#include "macro.h"
#include "ansi.h"

#define EWL 2			/* energy window search */

void    cshiftframe(short *sfstart, short *sfend, float *maxshift2, short dpm, float *residual,
		    short guard, float accshift, float maxshift, float delay, short subframel,
		    short extra)
{
	float   maxener;
	int     offset;
	int     iacshift;
	int     length;
	short   loc, loc2, g;
#if ANSI_EVRC_BOTTOM_GUARD
	short bg;

    bg = guard - 2*GUARD - FrameSize + EWL;
#endif

	if (delay < 0)
	{
		iacshift = (short) (-accshift + 0.5);
		iacshift = -iacshift;
	}
	else
		iacshift = (short) (-accshift + 0.5);

	/* determine first a pitch pulse somewhere near dpm */
	length = (short) (1.5 * delay);
	offset = dpm + iacshift - (short) (0.25 * delay);
#if ANSI_EVRC_BOTTOM_GUARD
    if (offset < bg)
    {
        offset = bg;
        printf("cshift(): ANSI_EVRC_BOTTOM_GUARD\n");
    }
#endif

	g = guard - offset - EWL;	/* Maximum allowed search size */
	if (length > g)
		length = g;
	maxeloc(&loc, &maxener, residual, offset, length, EWL);
	loc -= iacshift;

	/* now find the first pitch pulse for sure */
	if (loc < dpm)
	{
		offset = loc + iacshift + (short) (0.75 * delay + 0.5);
#if ANSI_EVRC_BOTTOM_GUARD
		if (offset < bg)
        {
			offset = bg;
            printf("cshift(): ANSI_EVRC_BOTTOM_GUARD\n");
        }
#endif
		length = (short) (0.5 * delay);
		g = guard - offset - EWL;	/* Maximum allowed search size */
		if (length > g)
			length = g;
		maxeloc(&loc, &maxener, residual, offset, length, EWL);
		loc -= iacshift;
	}
	if (loc > dpm + delay)
	{
		offset = loc + iacshift - (short) (1.25 * delay + 0.5);
#if ANSI_EVRC_BOTTOM_GUARD
		if (offset < bg)
        {
			offset = bg;
            printf("cshift(): ANSI_EVRC_BOTTOM_GUARD\n");
        }
#endif
		length = (short) (0.5 * delay);
		g = guard - offset - EWL;	/* Maximum allowed search size */
		if (length > g)
			length = g;
		maxeloc(&loc2, &maxener, residual, offset, length, EWL);
		loc2 -= iacshift;
		if (loc2 >= dpm)
			loc = loc2;
	}

	*sfstart = dpm;
	*sfend = loc + extra;
	*maxshift2 = maxshift;
	if (loc < subframel - extra / 2 && *sfend > subframel)
		*sfend = subframel;
	if (loc < subframel + extra / 2 && *sfend > subframel + extra)
		*sfend = subframel + extra;
	if (loc >= subframel + extra / 2)
		*sfend = subframel;

	if (loc >= *sfend || loc < *sfstart)
		*maxshift2 = 0;
}
