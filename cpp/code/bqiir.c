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
/*  Module:     bqiir.c                                                 */
/*----------------------------------------------------------------------*/
/*         ..Complexity (not including main() buffers).                 */
/*           total data ROM        84 bytes                             */
/*           total static RAM     144 bytes                             */
/*           total dynamic RAM    736 bytes                             */
/*----------------------------------------------------------------------*/
/*======================================================================*/
/*         ..Include files.                                             */
/*----------------------------------------------------------------------*/
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>

/*======================================================================*/
/*        ..Typedefs.                                                   */
/*----------------------------------------------------------------------*/
typedef long   int32;
typedef short  int16;

/*======================================================================*/
/*        ..Defines.                                                    */
/*             data ROM       12 bytes                                  */
/*----------------------------------------------------------------------*/
#define  BQ_MAIN         0

#define  BQ_N_DATA     160
#define  BQ_N_FILTERS    3
#define  BQ_N_ORDER      2
#define  BQ_N_SAVE       (BQ_N_FILTERS * BQ_N_ORDER)
#define  BQ_N_BUFFER     (BQ_N_SAVE + BQ_N_DATA)
#define  BQ_N_W          (2 * BQ_N_FILTERS * (BQ_N_ORDER + 1))

/*======================================================================*/
/*        ..Globals.                                                    */
/*             data ROM       72 bytes                                  */
/*----------------------------------------------------------------------*/
float  bq_w[BQ_N_W] = {
	/*   n2           n1            n0          d2             d1           d0   */
	1.000125737, -2.000125721, 1.000000000, 0.952444269, -1.943779252, 1.000000000,
	0.999873585, -1.999873569, 1.000000000, 0.875214548, -1.866892280, 1.000000000,
	0.833470028, -1.666939491, 0.833469450, 0.833345838, -1.825209384, 1.000000000
};

/*======================================================================*/
/*        ..Floating-point vector copy.                                 */
/*----------------------------------------------------------------------*/
void  V_copy (float* a,float* b,int32 n)
{
	/*....execute....*/
	while (n--)  *b++ = *a++;
}

/*======================================================================*/
/*        ..Three-stage bi-quad hi-pass IIR filter.                     */
/*          This function processes 160 point (20 ms) blocks).          */
/*             static RAM     144 bytes                                 */
/*             dynamic RAM    736 bytes                                 */
/*----------------------------------------------------------------------*/
void  bqiir (float* buf)
{
	/*....(static) variables....*/
	static float  bq_xsave[BQ_N_FILTERS*BQ_N_SAVE];
	static float  bq_ysave[BQ_N_FILTERS*BQ_N_SAVE];

	/*....(local) variables....*/
	register int16   i;
	register int16   j;
	register int16   k;
	register int16   fi;
	register float*  tmpP;
	register float*  wP;
	register float*  wpP;
	register float*  xP;
	register float*  xpP;
	register float*  xsP;
	register float*  yP;
	register float*  ypP;
	register float*  ysP;
	register float   sum;
	float  bq_x[BQ_N_BUFFER];
	float  bq_y[BQ_N_BUFFER];

	/*....execute....*/
	V_copy(buf,bq_x+BQ_N_SAVE,BQ_N_DATA);

	for (xsP=bq_xsave,ysP=bq_ysave,xP=bq_x,yP=bq_y+BQ_N_ORDER,wP=bq_w,fi=0,i=BQ_N_FILTERS-1; i>=0; i--,xP+=BQ_N_ORDER,yP+=BQ_N_ORDER,wP+=(2*(BQ_N_ORDER+1)),fi++,xsP+=BQ_N_SAVE,ysP+=BQ_N_SAVE) {
		V_copy(xsP,bq_x,BQ_N_SAVE);
		V_copy(ysP,bq_y,BQ_N_SAVE);
	
		for (xpP=xP,ypP=yP,j=0; j<BQ_N_DATA+i*BQ_N_ORDER; j++,xpP++,ypP++) {
			sum = 0.0;
			for (tmpP=xpP,wpP=wP,    k=0; k<BQ_N_ORDER+1; k++,wpP++,tmpP++) {
				sum += *wpP * *tmpP;
			}
			for (tmpP=ypP-BQ_N_ORDER,k=0; k<BQ_N_ORDER;   k++,wpP++,tmpP++) {
				sum -= *wpP * *tmpP;
			}
			*ypP = sum;
		}
		V_copy(bq_x+BQ_N_BUFFER-BQ_N_SAVE,xsP,BQ_N_SAVE);
		V_copy(bq_y+BQ_N_BUFFER-BQ_N_SAVE,ysP,BQ_N_SAVE);

		V_copy(bq_y,bq_x,BQ_N_BUFFER);
	}
	V_copy(bq_y+BQ_N_SAVE,buf,BQ_N_DATA);
}

#if BQ_MAIN
/*===============================================================*/
/*        ..Main.                                                */
/*---------------------------------------------------------------*/
main (int argc,char* argv[])
{
	/*....(local) variables....*/
	FILE*   ifileP;
	FILE*   ofileP;
	int16   sx[BQ_N_DATA];
	float   x[BQ_N_DATA];
	int32   j;

	/*....execute....*/
	if (argc < 3) {
		fprintf(stderr,"usage:  %s  INFILE  OUTFILE\n",argv[0]);
		exit(-1);
	}

	/*...open data files...*/
	if ((ifileP = fopen(argv[1],"rb")) == NULL) {
		fprintf(stderr,"ERROR:  Cannot open input file \"%s\"\n",argv[1]);
		exit(-1);
	}
	if ((ofileP = fopen(argv[2],"wb")) == NULL) {
		fprintf(stderr,"ERROR:  Cannot open output file \"%s\"\n",argv[2]);
		exit(-1);
	}

	/*...filter loop...*/
	j = 1;
	while (fread(sx,sizeof(int16),BQ_N_DATA,ifileP) == BQ_N_DATA) {
		V_copy(sx,x,BQ_N_DATA);       /*...int16 to float copy...*/
		bqiir(x);
		V_copy(x,sx,BQ_N_DATA);       /*...float to int16 copy...*/
		fwrite(sx,sizeof(int16),BQ_N_DATA,ofileP);
		fprintf(stdout,"%s:  frame %ld\r",argv[0],j++);
	}
	fprintf(stdout,"\n");
}
#endif
