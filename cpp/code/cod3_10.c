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

#include <float.h>
#include <math.h>

#define L_CODE    55
#define NB_PULSE  3
#define STEP      7
#define NB_TRACK  3


/*-------------------------------------------------------------------*
 * routine   cod3_10.c                                               *
 *           ~~~~~~~~~~                                              *
 * Algebraic codebook; 10 bits: 3 pulses in a frame of 55 samples.   *
 *-------------------------------------------------------------------*
 * The code length is 55, containing 3 nonzero pulses: i0...i2.      *
 * All pulses have a fixed amplitudes: +1 for i0 and i2, -1 for i1.  *
 * 1 bit is used to change sign of all pulses.                       *
 * Each pulse can have 8 possible positions:                         *
 *                                                                   *
 * track0 (i0):  0, 7, .. 49                                         *
 * track1 (i1):  2, 9, .. 51                                         *
 * track2 (i2):  4, 11, .. 53                                        *
 *-------------------------------------------------------------------*
 * Bruno Bessette,   august 1995                                     *
 * Claude Laflamme,  august 1995                                     *
 *-------------------------------------------------------------------*/

void cod3_10(
  float dn[],     /* input : correlation between target and h[]            */
  float cn[],     /* input : residual after long term prediction           */
  int   l_subfr,  /* input : lenght of subframe                            */
  float h[],      /* input : impulse response of weighted synthesis filter */
  float cod[],    /* output: algebraic (fixed) codebook excitation         */
  float y[],      /* output: filtered fixed codebook excitation            */
  int   *indices, /* output: indices of 3 pulses (10 bits, 1 word)         */
  float *gain_code
)
{
  int i0, i1, i2;
  int i, j, k, pos, dec, index;
  int codvec[NB_PULSE];
  float psk=-1, ps1, ps2, sq2, sqk;
  float alpk, alp1, alp2;
  float s, *ptr;
  float rr[L_CODE][L_CODE];

  /* Put h[i]= dn[i] = cn[i] = 0; for i>l_subfr */

  for(i=l_subfr; i<L_CODE; i++)
    h[i] = dn[i] = cn[i] = 0.0;


 /*----------------------------------------------------------------*
  *        BUILD "rr[][]", MATRIX OF AUTOCORRELATION.              *
  *----------------------------------------------------------------*/

  s = 0.0;
  i = l_subfr-1;
  for (k=0; k<l_subfr; k++, i--)
  {
    s += h[k]*h[k];
    rr[i][i] = s;
  }

  for (dec=1; dec<l_subfr; dec++)
  {
    s = 0.0;
    j = l_subfr - 1;
    i = j - dec;
    for (k=0; k<(l_subfr-dec); k++, i--, j--)
    {
      s += h[k]*h[k+dec];
      rr[i][j] = rr[j][i] = s;
    }
  }

  /* Put rr[i][j] = 0; for i,j>l_subfr */

  for(i=l_subfr; i<L_CODE; i++)
    for(j=0; j<L_CODE; j++)
      rr[i][j] = rr[j][i] = 0.0;


 /*----------------------------------------------------------------*
  *              SEARCH THE BEST CODEVECTOR.                       *
  *----------------------------------------------------------------*/

  sqk = -1.0;
  alpk = 1.0;

   /*----------------------------------------------------------------*
    * i0, i1 and i2 loop: 8x8x8 positions = 512                      *
    *----------------------------------------------------------------*/

    for (i0=0; i0<L_CODE; i0+=STEP)
    {
      for (i1=2; i1<L_CODE; i1+=STEP)
      {
	ps1 = dn[i0] - dn[i1];
	alp1 = rr[i0][i0] + rr[i1][i1] - 2.0 * rr[i0][i1];

	for (i2=4; i2<L_CODE; i2+=STEP)
	{
	  ps2 = ps1 + dn[i2];
	  alp2 = alp1 + rr[i2][i2] + 2.0 * (rr[i0][i2] - rr[i1][i2]);

	  sq2 = (ps2 * ps2);

	  if ((alpk*sq2) > (sqk*alp2))
	  {
	    sqk = sq2;
	    alpk = alp2;
	    psk = ps2;
	    codvec[0] = i0;
	    codvec[1] = i1;
	    codvec[2] = i2;
	  }
	}
      }
    }


 /*--------------------------------------------------------------------*
  * BUILD THE CODEWORD, THE FILTERED CODEWORD AND INDEX OF CODEVECTOR. *
  *--------------------------------------------------------------------*/

  for (i=0; i<L_CODE; i++) cod[i] = 0.0;
  for (i=0; i<L_CODE; i++) y[i] = 0.0;
  for (i=0; i<NB_PULSE; i++) indices[i] = -1;

  *gain_code = psk / alpk;

  index = 0;
  if (*gain_code < 0) index = 1;       /* global sign */

  for (k=0; k<NB_PULSE; k++)
  {
    pos = codvec[k];

    if (k == 1) s = -1.0;
    else s = 1.0;

    cod[pos] += s;

    ptr = h;
    for (i=pos; i<L_CODE; i++) y[i] += s * (*ptr++);

    index <<= 3;
    i = pos / STEP;
    index += i;
  }

  *indices = index;

  return;
}

