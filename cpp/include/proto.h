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
/*  Module:     proto.h                                                 */
/*----------------------------------------------------------------------*/
#ifndef  _PROTO_H_
#define  _PROTO_H_

/*======================================================================*/
/*         ..Includes.                                                  */
/*----------------------------------------------------------------------*/
#include  "macro.h"

/*======================================================================*/
/*         ..CELP Function definitions.                                 */
/*----------------------------------------------------------------------*/
extern void lpcanalys(float *pc, float *rc, float *input, short order, short len,float*);
extern void weight(float *awght, float *a, float gammma, short order);
extern void ZeroInput(float *output, float *coef_uq, float *coef, float *in, float gamma1, float gamma2, short order, short length, short type);
extern void ImpulseR(float *output, float *coefw, float *IIRmemory, short order, short length);
extern void ImpulseRzp(float *output, float *coef_uq, float *coef, float gamma1, float gamma2, short order, short length);
extern void ComputeHtH(float *HtH, float *H, short length);
extern void FCBsearch(float *Ex2, float *inTARGET, float *HtH, short length, short hlength, short spacing, float gopt);
extern void FCBsearch_best(float *Ex2, short *idxcb, short *idxcbg, float *inTARGET, float *HtH, float *gcb, short gcb_size, short length, short hlength, short gainq, short delay);
extern void SynthesisFilter(float *output, float *input, float *coef, float *memory, short order, short length);
extern void autocorrelation(float *r, float *winput, short len, short order);
extern void durbin(float *r, float *rc, float *pc, short order);
extern void a2lsp(float *freq, float *a);
extern void lsp2a(float *a, float *freq);
extern void Interpol(float *Lar, float *last, float *current, short SubNum, short order);
extern void Interpol_delay(float *out, float *last, float *current, short SubNum);
extern void acb_excitation(float *Ex1, float gain, float *delay3, float *PitchMemory, short length);
extern void Bitpack(short in, unsigned short *TrWords, short NoOfBits, short *ptr);
extern void BitUnpack(short *out, unsigned short *RecWords, short NoOfBits, short *ptr);

/*======================================================================*/
/*         ..RCELP routines.                                            */
/*----------------------------------------------------------------------*/
extern void GetResidual(float *residual, float *input, float *coef, float *mem, short order, short length);
extern void fndppf(float *delay, float *beta, float *buf, short dmin, short dmax, short length);
extern void ComputeACB(float *residualm, float *excitation, float *delay, float *residual, short guard, short *dpm, float *accshift, float beta, short length, short rshift);
extern void putacbc(float *exctation, float *input, short dpl, short subframel, short extra, float *delay3, float freq, short prec);
extern void mod(float *residualm, float *accshift, float beta, short shiftr, short resolution, float *exctation, float *Dresidual, float *residual, short guard, short *dpm, float delay, short subframel, short extra);
extern void cshiftframe(short *sfstart, short *sfend, float *maxshift2, short dpm, float *residual, short guard, float accshift, float maxshift, float delay, short subframel, short extra);
extern void maxeloc(short *maxloc, float *maxener, float *signal, short dp, short length, short ewl);
extern void modifyorig(float *residualm, float *accshift, float beta, short *dpm, short shiftrange, short resolution, float *TARGET, float *residual, short dp, short sfend);
extern void bl_intrp(float *output, float *input, float delay, float factor, short fl);
extern void getgain(float *exctation, float *lambda, float *H, short *idxcb, float *gcb, float *gcb_mid, short gcb_size, short Quantize, float *mresidual, short subframel, short hlength);
extern void ConvolveWithH(float *out, float *in, float *coefw, short order, short length);
extern void ConvolveImpulseR(float *out, float *in, float *H, short hlength, short length);
extern void PickToAverage(float *res, float *signal, short length);
extern void lspmaq_dec(short ndim, short kdim, short many, short *nsub, short *nsiz, float *y, short *index, short br, float *);
extern void lspmaq(float *x, short ndim, short kdim, short many, short *nsub, short *nsiz, float alp, float *y, short *index, short br, float *);
extern void Weight2Res(float *, float *, float *, float *, float, float, short, short);
extern void iir(float *output, float *input, float *coef, float *IIRmemory, short order, short length);
extern void fir(float *output, float *input, float *coef, float *FIRmemory, short order, short length);
extern void apf(float *, float *, float *, float, float, float, float, float, float, short, short, short);
extern void SetRate(int);
extern void GetExc800bps(float *output, short *best, float scale, float *input, short length, short flag, short n);
extern void GetExc800bps_dec(float *output, short length, short best, short flag, short n, short fer_flag);
extern float ran_g(long *seed0);
extern void V_copy (float *, float *, long);
#endif
