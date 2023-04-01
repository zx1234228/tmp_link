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
#define NB_PULSE  8
#define STEP      5
#define NB_TRACK  5

#define NB_ITER   4

/*-------------------------------------------------------------------*
 * routine   cod8_35.c                                               *
 *           ~~~~~~~~~~                                              *
 * Algebraic codebook; 35 bits: 8 pulses in a frame of 55 samples.   *
 *-------------------------------------------------------------------*
 * The code length is 55, containing 8 nonzero pulses: i0...i7.      *
 * All pulses can have two possible amplitudes: +1 or -1.            *
 * Each pulse can have 11 possible positions.                        *
 * The 8 pulses can be on the following 5 tracks:                    *
 *                                                                   *
 * track0 (i0,i5):  0, 5, .. 50                                      *
 * track1 (i1,i6):  1, 6, .. 51                                      *
 * track2 (i2,i7):  2, 7, .. 52                                      *
 * track3 (i3,i8):  3, 8, .. 53                                      *
 * track4 (i4,i9):  4, 9, .. 54                                      *
 *-------------------------------------------------------------------*
 * Bruno Bessette, August 1995                                       *
 * Claude Laflamme, August 1995                                      *
 *-------------------------------------------------------------------*/

void cod8_35(
  float dn[],     /* input : correlation between target and h[]            */
  float cn[],     /* input : residual after long term prediction           */
  int   l_subfr,  /* input : lenght of subframe                            */
  float h[],      /* input : impulse response of weighted synthesis filter */
  float cod[],    /* output: algebraic (fixed) codebook excitation         */
  float y[],      /* output: filtered fixed codebook excitation            */
  int   *indx,    /* output: indices of 8 pulses (8+8+8+11 = 35 bits)      */
  float *gain_code
)
{

  int i0, i1, i2, i3, i4, i5, i6, i7, ia, ib;
  int i, j, k, pos, dec, index;
  int ipos[NB_TRACK*2], codvec[NB_PULSE+1], indices[NB_TRACK];
  float psk, ps, ps0, ps1, ps2, sq, sq2, sqk;
  float alpk, alp, alp0, alp1, alp2;
  float s, *ptr;
  float dn2[L_CODE], sign[L_CODE], rrv[L_CODE], dn6[L_CODE];
  float rr[L_CODE][L_CODE];

  /* Put h[i]= dn[i] = cn[i] = 0; for i>l_subfr */

  for(i=l_subfr; i<L_CODE; i++)
    h[i] = dn[i] = cn[i] = 0.0;


 /*----------------------------------------------------------------*
  *     BUILD "dn2[]", "sign[]" AND "dn6[]" VECTORS.               *
  *----------------------------------------------------------------*/

  ps1 = 1e-20;
  ps2 = 1e-20;
  for (i=0; i<L_CODE; i++) ps1 += (cn[i]*cn[i]);
  for (i=0; i<L_CODE; i++) ps2 += (dn[i]*dn[i]);
  s = sqrt(ps2/ps1);


  for (i=0; i<NB_TRACK; i++) {

    for (j=i; j<L_CODE; j+=STEP) {
      ps1 = dn[j];
      ps2 = (s*cn[j]) + (2.0*ps1);

      if (ps2 >= 0.0) sign[j] = 1.0;
      else {
	sign[j] = -1.0;
	ps1 = -ps1;
	ps2 = -ps2;
      }
      dn2[j] = ps1;
      dn6[j] = ps2;
    }

    for (k=0; k<5; k++) {
      ps = FLT_MAX;
      index = i;
      for (j=i; j<L_CODE; j+=STEP) {
	if ((dn6[j] < ps) && (dn6[j] >= 0)) {
	  ps = dn6[j];
	  index = j;
	}
      }
      dn6[index] = -1;
    }
  }

 /*----------------------------------------------------------------*
  *     Set staring position of each pulse.                        *
  *----------------------------------------------------------------*/

  for(i=0; i<NB_TRACK; i++)
  {
    ipos[i] = i;
    ipos[i+5] = i;
  }


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
      rr[i][j] = rr[j][i] = (s * sign[i] * sign[j]);
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
  for(i=0;i<NB_PULSE+1;++i)
	codvec[i]=i;

  ia  = ipos[0];
  ib  = ipos[1];
  ps  = 0;
  psk = 0;

 /*------------------------------------------------------------------*
  * i loop: try for nb of iterations specified.                      *
  *------------------------------------------------------------------*/

  for (i=0; i<NB_ITER; i++)
  {

   /*----------------------------------------------------------------*
    * i0 and i1 loop: 6 positions x 11 positions = 66                *
    *----------------------------------------------------------------*/

    sq = -1.0;
    alp = 1.0;

    for (i0=ipos[0]; i0<L_CODE; i0+=STEP)
    {
    if (dn6[i0] >= 0)
    {

      for (i1=ipos[1]; i1<L_CODE; i1+=STEP)
      {
        ps2 = dn2[i0] + dn2[i1];
        alp2 = rr[i0][i0] + rr[i1][i1] + 2.0 * rr[i0][i1];

        sq2 = (ps2 * ps2);

        if ((alp*sq2) > (sq*alp2))
        {
          sq = sq2;
          ps = ps2;
          alp = alp2;
          ia = i0;
          ib = i1;
        }
      }
    }
    }
    i0 = ia;
    i1 = ib;

   /*----------------------------------------------------------------*
    * i2 and i3 loop: 6 positions x 11 positions = 66                *
    *----------------------------------------------------------------*/

    for (i3=ipos[3]; i3<L_CODE; i3+=STEP)
      rrv[i3] = rr[i3][i3] + 2.0 * (rr[i0][i3] + rr[i1][i3]);

    ps0 = ps;
    alp0 = alp;
    sq = -1.0;
    alp = 1.0;

    for (i2=ipos[2]; i2<L_CODE; i2+=STEP)
    {
    if (dn6[i2] >= 0)
    {
      ps1 = ps0 + dn2[i2];
      alp1 = alp0 + rr[i2][i2] + 2.0 * (rr[i0][i2] + rr[i1][i2]);

      for (i3=ipos[3]; i3<L_CODE; i3+=STEP)
      {
        ps2 = ps1 + dn2[i3];
        alp2 = alp1 + rrv[i3] + 2.0 * rr[i2][i3];

        sq2 = (ps2 * ps2);

        if ((alp*sq2) > (sq*alp2))
        {
          sq = sq2;
          ps = ps2;
          alp = alp2;
          ia = i2;
          ib = i3;
        }
      }
    }
    }
    i2 = ia;
    i3 = ib;


   /*----------------------------------------------------------------*
    * i4 and i5 loop: 6 positions x 11 positions = 66                *
    *----------------------------------------------------------------*/

    for (i5=ipos[5]; i5<L_CODE; i5+=STEP)
      rrv[i5] = rr[i5][i5] + 2.0 * (rr[i0][i5] + rr[i1][i5]
                + rr[i2][i5] + rr[i3][i5]);

    ps0 = ps;
    alp0 = alp;
    sq = -1.0;
    alp = 1.0;

    for (i4=ipos[4]; i4<L_CODE; i4+=STEP)
    {
    if (dn6[i4] >= 0)
    {
      ps1 = ps0 + dn2[i4];
      alp1 = alp0 + rr[i4][i4] + 2.0 * (rr[i0][i4] + rr[i1][i4]
                  + rr[i2][i4] + rr[i3][i4]);

      for (i5=ipos[5]; i5<L_CODE; i5+=STEP)
      {
        ps2 = ps1 + dn2[i5];
        alp2 = alp1 + rrv[i5] + 2.0 * rr[i4][i5];

        sq2 = (ps2 * ps2);

        if ((alp*sq2) > (sq*alp2))
        {
          sq = sq2;
          ps = ps2;
          alp = alp2;
          ia = i4;
          ib = i5;
        }
      }
    }
    }
    i4 = ia;
    i5 = ib;

   /*----------------------------------------------------------------*
    * i6 and i7 loop: 6 positions x 11 positions = 66                *
    *----------------------------------------------------------------*/

    for (i7=ipos[7]; i7<L_CODE; i7+=STEP)
      rrv[i7] = rr[i7][i7] + 2.0 * (rr[i0][i7] + rr[i1][i7] + rr[i2][i7]
                + rr[i3][i7] + rr[i4][i7] + rr[i5][i7]);

    ps0 = ps;
    alp0 = alp;
    sq = -1.0;
    alp = 1.0;

    for (i6=ipos[6]; i6<L_CODE; i6+=STEP)
    {
    if (dn6[i6] >= 0)
    {
      ps1 = ps0 + dn2[i6];
      alp1 = alp0 + rr[i6][i6] + 2.0 * (rr[i0][i6] + rr[i1][i6] + rr[i2][i6]
                  + rr[i3][i6] + rr[i4][i6] + rr[i5][i6]);

      for (i7=ipos[7]; i7<L_CODE; i7+=STEP)
      {
        ps2 = ps1 + dn2[i7];
        alp2 = alp1 + rrv[i7] + 2.0 * rr[i6][i7];

        sq2 = (ps2 * ps2);

        if ((alp*sq2) > (sq*alp2))
        {
          sq = sq2;
          ps = ps2;
          alp = alp2;
          ia = i6;
          ib = i7;
        }
      }
    }
    }

   /*----------------------------------------------------------------*
    * memorise codevector if this one is better than the last one.   *
    *----------------------------------------------------------------*/

    if ((alpk*sq) > (sqk*alp))
    {
      sqk  = sq;
      alpk = alp;
      psk  = ps;
      codvec[0] = i0;
      codvec[1] = i1;
      codvec[2] = i2;
      codvec[3] = i3;
      codvec[4] = i4;
      codvec[5] = i5;
      codvec[6] = ia;
      codvec[7] = ib;
      codvec[8] = i;
    }


   /*----------------------------------------------------------------*
    * Cyclic permutation of i0 - i7                                  *
    *----------------------------------------------------------------*/

    pos = ipos[0];
    for (j=0; j<(NB_TRACK*2)-1; j++)
      ipos[j] = ipos[j+1];
    ipos[(NB_TRACK*2)-1] = pos;

  }


 /*--------------------------------------------------------------------*
  * BUILD THE CODEWORD, THE FILTERED CODEWORD AND INDEX OF CODEVECTOR. *
  *--------------------------------------------------------------------*/

  for (i=0; i<L_CODE; i++) cod[i] = 0.0;
  for (i=0; i<L_CODE; i++) y[i] = 0.0;
  for (i=0; i<NB_TRACK; i++) indices[i] = -1;

  *gain_code = psk / alpk;

  for (k=0; k<NB_PULSE; k++)
  {
    pos = codvec[k];
    s = sign[pos];

    cod[pos] += s;

    ptr = h;
    for (i=pos; i<L_CODE; i++) y[i] += s * (*ptr++);


    j = pos%NB_TRACK;
    index = pos/NB_TRACK;
    if (s < 0) index += 16;

    if (indices[j] == -1) indices[j] = index;
    else {
      if (((index ^ indices[j]) & 16) == 0) {

	if (indices[j] <= index) {
	  indices[j] = ((indices[j] & 16) << 3)
	     + ((indices[j] & 15)*11) + (index & 15);
	}
	else {
	  indices[j] = ((indices[j] & 16) << 3)
	     + ((index & 15)*11) + (indices[j] & 15);
	}
      }
      else {

	if ((indices[j] & 15) <= (index & 15)) {
	  indices[j] = ((index & 16) << 3)
	     + ((index & 15)*11) + (indices[j] & 15);
	}
	else {
	  indices[j] = ((indices[j] & 16) << 3)
	     + ((indices[j] & 15)*11) + (index & 15);
	}
      }
    }
  }


  if (codvec[8] == 0) {
    indx[0] = indices[0];
    indx[1] = indices[1];
    indx[2] = indices[2];
    indx[3] = ((indices[3] & 16) << 4) + ((indices[4] & 16) << 3)
	    + ((indices[3] & 15)*11) + (indices[4] & 15);
  }
  else if (codvec[8] == 1) {
    indx[0] = indices[1];
    indx[1] = indices[2];
    indx[2] = indices[3];
    indx[3] = (1 << 9)
	    + ((indices[4] & 16) << 4) + ((indices[0] & 16) << 3)
	    + ((indices[4] & 15)*11) + (indices[0] & 15);
  }
  else if (codvec[8] == 2) {
    indx[0] = indices[2];
    indx[1] = indices[3];
    indx[2] = indices[4];
    indx[3] = (2 << 9)
	    + ((indices[0] & 16) << 4) + ((indices[1] & 16) << 3)
	    + ((indices[0] & 15)*11) + (indices[1] & 15);
  }
  else if (codvec[8] == 3) {
    indx[0] = indices[3];
    indx[1] = indices[4];
    indx[2] = indices[0];
    indx[3] = (3 << 9)
	    + ((indices[1] & 16) << 4) + ((indices[2] & 16) << 3)
	    + ((indices[1] & 15)*11) + (indices[2] & 15);
  }

  return;
}

