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
/*  Module:     e_globs.c                                               */
/*----------------------------------------------------------------------*/
/*  Memory Usage:														*/
/*		ROM: 				0											*/
/*		Static/Global RAM: 	1202										*/
/*		Stack/Local RAM: 	0											*/
/*----------------------------------------------------------------------*/
#include  "macro.h"
#include  "globs.h"

/*======================================================================*/
/*         ..Globals (encoder).                                         */
/*----------------------------------------------------------------------*/
float  *ExconvH = Scratch;	/* Excitation of adaptive codebook convolved with H (uses Scratch)*/

long    encode_fcnt;		/* Frame counter */

float  *HPspeech;		/* orig. speech signal - memory overlap with residual */
float   ConstHPspeech[GUARD];	/* temporary buffer to store HPspech  */

float   OldlspE[ORDER];		/* Last frame quantized lsp                */

float   ppvq_mid[ACBGainSize - 1];	/* Intermidiate values in ppvq */
float   lsp_nq[ORDER];		/* Correlation coefficients                */
float   Oldlsp_nq[ORDER];	/* Last frame quantized lsp                */
float   lspi_nq[ORDER];		/* Interpolation of correlation coeff      */
float   pci_nq[ORDER];		/* Interpolated prediction coefficients    */
float   wpci[ORDER];		/* Interpolated weighted prediction coefficients */

float   Excitation[ACBMemSize + SubFrameSize + EXTRA];

float   H[Hlength+1];		/* Impulse response [Hlength]              */
float   HtH[Hlength+1];		/* Impulse response ^2 [Hlength]           */
float   SynMemoryM[ORDER];	/* weighted speech synthesis filter memory */

float   TARGET[SubFrameSize+1];	/* Residual - Zero input response       */
float	TARGETw[SubFrameSize+1];	/* weighted target */

float   WFmemFIR[ORDER];	/* Weighting filter memory                 */
float   WFmemIIR[ORDER];	/* Weighting filter memory                 */

float   zir[SubFrameSize];	/* Zero Input Response (can share memory w/ HtH)*/

float   residual[2 * GUARD + FrameSize];
float   residualm[SubFrameSize + EXTRA];

float   origm[SubFrameSize];
float  *worigm = origm;		/* shared weighted original memory */

float   accshift;
float   delay1;
float   pdelay;
float   beta, beta1;
short   dpm;

float   LPCgain;		/* used for frame erasures */
short   shiftSTATE;
short   lastrateE;		/* last bitrate used for encoder*/

float   fcbGain;                /* ACELP fixed codebook gain */
float   y2[55];                 /* Filtered innovative vector (debug only) */


