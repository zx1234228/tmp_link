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
/*  Module:     lspmaq.c                                                */
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*  Memory Usage:														*/
/*		ROM: 				1											*/
/*		Static/Global RAM: 	0											*/
/*		Stack/Local RAM: 	527											*/
/*----------------------------------------------------------------------*/
/************************************************************************
*            *****   LSP matrix quantization  ************              *
*                                                                       *
* Input Arguments                                                       *
* ***************                                                       *
* x		Input LSP matrix  x(ndimx,kdim), i.e., kdim rows of             *
*		ndimx elements.                                                 *
*		LSP vectors row by row.                                         *
* ndim		Actual vector dimension (LSP order).  ndim <= ndimx         *
* kdim		Number of LSP vectors, i.e., number of rows in the main matrix.*
* many		Number of sub-matrices.                                     *
* nsub		Array containing "many" sub-matrices sizes. nsub(i) is the size *
*		of the rows in the ith sub-matrix. Sum(i)[ nsub(i) ] = ndim     *
* book		(1D). Contains "many" codebooks in series. The ith codebook *
*		contains matices of size kdim rows by nsub(i) columns.          *
*		i.e., kdim vectors of size nsub(i) in series.                   *
* alp		Weight adjustment factor.  alp=0  no weight                 *
*                                          alp >0 input is weighted     *
* nsiz		Contain "many" size values. nsiz(i) is the number of matrices *
*		in codebook i.                                                  *
*                                                                       *
* Output arguments                                                      *
* ****************                                                      *
* y		Output matrix. The quantized version of x (same size).          *
* index		Contains "many" indices of the best matrix from each codebook *
* d		Aux. array, at least the size of max( nsiz(i) )                 *
*                                                                       *
*************************************************************************/
#include "macro.h"
#include "rom.h"

void    lspmaq(float *x, short ndim, short kdim, short many, short *nsub,
	       short *nsiz, float alp, float *y, short *index, short br, float *lsptab)
{
	register int i, j, k, m, im, jp, km, jm, jk, jpm, jj, jpmk, kn;
	float   dm;
        static float df = MIN_LSP_SEP;
	float   d[2048];

	jj = 0;

	/* Find weights */
	for (k = 0; k < kdim; k++)
	{
		kn = k * ndim;
		y[kn] = x[kn + 1] - x[kn];
		y[kn + ndim - 1] = x[kn + ndim - 1] - x[kn + ndim - 2];
		for (i = 1; i < ndim - 1; i++)
		{
			y[i + kn] = Min(x[i + kn] - x[i - 1 + kn], x[i + 1 + kn] - x[i + kn]);
		}
		for (i = 0; i < ndim; i++)
		{
			if (y[i + kn] != 0)
				y[i + kn] = alp / y[i + kn] * OneDivPI2 + 1.0;
			else
				y[i + kn] = 50.0 * OneDivPI2 + 1.0;
		}
	}

	/* Code all sub-matrices */
	im = 0;
	jp = 0;
	for (m = 0; m < many; m++)
	{
		for (j = 0; j < nsiz[m]; j++)
			d[j] = 0;
		km = nsub[m] * kdim;
		for (j = 0; j < nsiz[m]; j++)
		{
			jm = km * j;
			for (k = 0; k < kdim; k++)
			{
				jk = k * nsub[m];
				kn = k * ndim;
				jpm = jp + jm + jk;
				for (i = 0; i < nsub[m]; i++)
					d[j] = d[j] + y[i + im + kn] * (x[i + im + kn] - lsptab[jpm + i]) * (x[i + im + kn] - lsptab[jpm + i]);
			}
		}

		if (im > 0)
		{
			for (k = 0; k < kdim; k++)
			{
				jk = k * nsub[m];
				kn = k * ndim;
				for (j = 0; j < nsiz[m]; j++)
				{
					jm = km * j;
					jpm = jp + jm + jk;
					if (lsptab[jpm] <= y[im - 1 + kn] + df)
						d[j] = 1.0e38;
				}
			}
		}

		dm = 1.0e38;
		for (j = 0; j < nsiz[m]; j++)
		{
			if (d[j] < dm)
			{
				dm = d[j];
				jj = j;
			}
		}

		index[m] = jj;

		jpm = jp + km * index[m];
		for (k = 0; k < kdim; k++)
		{
			jpmk = jpm + k * nsub[m];
			kn = k * ndim;
			for (i = 0; i < nsub[m]; i++)
				y[im + i + kn] = lsptab[jpmk + i];
		}

		im += nsub[m];
		jp += km * nsiz[m];
	}

}

/***************************************************************************
* Routine name: lspmaq_dec                                                 *
* Function: VQ of line spectral frequencies - Decoder part.                *
* Inputs:                                                                  *
*         ndim	- Actual vector dimension (LSP order).                     *
*         kdim	- Number of LSP vectors, i.e., number of rows in the main  *
*                matrix.                                                   *
*         many	- Number of sub-matrices.                                  *
*         nsub	- Array containing "many" sub-matrices sizes. nsub(i) is   *
*                the size of the rows in the ith sub-matrix.               *
*                Sum(i)[ nsub(i) ] = ndim.                                 *
*         book	- (1D). Contains "many" codebooks in series. The ith       *
*                codebook contains matices of size kdim rows by nsub(i)    *
*                columns. i.e., kdim vectors of size nsub(i) in series.    *
*         alp	- Weight adjustment factor. alp=0 no weight. alp >0 input  *
*                is weighted.                                              *
*         nsiz	- Contain "many" size values. nsiz(i) is the number of     *
*                matrices in codebook i.                                   *
*                                                                          *
* Outputs: y	- Output matrix. The quantized version of x (same size).   *
*          index - Contains "many" indices of the best matrix from each    *
*                  codebook.                                               *
*                                                                          *
*                                                                          *
*  Program written by Dror Nahumi, Dep. 45370H                             *
***************************************************************************/
void    lspmaq_dec(short ndim, short kdim, short many, short *nsub,
		   short *nsiz, float *y, short *index, short br, float *lsptab)
{
	register int i, k, m, im, jp, km, jpm, jpmk, kn;

	/* Code all sub-matrices */
	im = 0;
	jp = 0;
	for (m = 0; m < many; m++)
	{
		km = nsub[m] * kdim;
		jpm = jp + km * index[m];
		for (k = 0; k < kdim; k++)
		{
			jpmk = jpm + k * nsub[m];
			kn = k * ndim;
			for (i = 0; i < nsub[m]; i++)
				y[im + i + kn] = lsptab[jpmk + i];
		}
		im += nsub[m];
		jp += km * nsiz[m];
	}
}
