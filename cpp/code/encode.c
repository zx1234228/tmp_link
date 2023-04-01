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
/*  Module:     encode.c                                               	*/
/*----------------------------------------------------------------------*/
/* EVRC Encoder */
/*======================================================================*/
/*         ..Includes.                                                  */
/*----------------------------------------------------------------------*/
#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>

#include  "e_globs.h"
#include  "globs.h"
#include  "macro.h"
#include  "proto.h"
#include  "rom.h"
#include  "acelp_co.h"	/* for ACELP fixed codebook */
#include  "typedef.h"
#include  "tty.h"

/*======================================================================*/
/*         ..Reset RCELP encode rate.                                   */
/*----------------------------------------------------------------------*/
void
        SetEncodeRate(short rate)
{
	/*....execute....*/
	bit_rate = rate;

	switch (bit_rate)
	{
	case 1:
		FCBGainSize = 16;	/*...use half-rate...*/
		gnvq = gnvq_4;
		break;
	case 3:
		nsub = nsub22;
		nsize = nsize22;
		lognsize = lognsize22;
		lsptab = lsptab22;
		knum = 3;
		FCBGainSize = 16;
		gnvq = gnvq_4;
		break;
	case 4:
		nsub = nsub28;
		nsize = nsize28;
		lognsize = lognsize28;
		lsptab = lsptab28;
		knum = 4;
		FCBGainSize = 32;
		gnvq = gnvq_8;
		break;
	}
}

/*======================================================================*/
/*         ..Reset RCELP encoder parameters.                            */
/*----------------------------------------------------------------------*/
void
        InitEncoder()
{
	/*....(local) variables....*/
	int     j;
	float   ftmp;
	float   sum1, sum2;
	/****************************************************/
	/*         Algorithm (one time) initializations     */
	/****************************************************/

	/* Calculate ROM tables */

	/* FCB gains (4k) */
	sum1 = sum2 = 8.0 / 16.0;	/* Spacing  8.3/16.0 ?*/
	gnvq_4[0] = exp(sum1);
	for (j = 1; j < 16; j++)
	{
		sum2 += sum1;
		gnvq_4[j] = exp(sum2);
	}

	/* FCB gains (8k) */
	sum1 = sum2 = 8.0 / 32.0;	/* Spacing */
	gnvq_8[0] = exp(sum1);
	for (j = 1; j < 32; j++)
	{
		sum2 += sum1;
		gnvq_8[j] = exp(sum2);
	}

	/* More initialization is done in the routines using FirstTime */
	/* static variable to indicate algorithmic reset.              */

	/*....execute....*/
	SetEncodeRate(4);

	encode_fcnt = 0;

	/* ACB gains */
	for (j = 0; j < ACBGainSize - 1; j++)
		ppvq_mid[j] = (ppvq[j] + ppvq[j + 1]) / 2.0;

	for (j = 0; j < ORDER; j++)
		SynMemoryM[j] = 0.0;

	ftmp = 0.48 / (float) ORDER;
	OldlspE[0] = Oldlsp_nq[0] = ftmp;
	for (j = 1; j < ORDER; j++)
	{
		OldlspE[j] = OldlspE[j - 1] + ftmp;
		Oldlsp_nq[j] = Oldlsp_nq[j - 1] + ftmp;
	}

	HPspeech = residual;

	for (j = 0; j < GUARD; j++)
		ConstHPspeech[j] = 0.0;

	for (j = 0; j < ORDER; j++)
		WFmemIIR[j] = WFmemFIR[j] = 0.0;

	for (j = 0; j < ACBMemSize; j++)
		Excitation[j] = 0.0;
	accshift = 0.0;
	shiftSTATE = 0;
	dpm = 0;
	pdelay = 40.0;

	LPCgain = 1.0;
}

/*======================================================================*/
/*         ..Get beta, et al.                                           */
/*----------------------------------------------------------------------*/
float  pre_encode (
	float*  inFbuf,
	float*  Rs
)
{
	/*....(local) variables....*/
	register int i, j;	/* counters */
	float   sum1;		/* tmp accumulator */

	/*....execute....*/

	/* Re-initialize PackWdsPtr */
	PackWdsPtr[0] = 16;
	PackWdsPtr[1] = 0;
	for (i = 0; i < PACKWDSNUM; i++)
		PackedWords[i] = 0;

	/* Process Data */
	for (j = 0; j < FrameSize + 80; j++)
		HPspeech[j + GUARD] = inFbuf[j];

	/* Make sure HPspeech - HPspeech+2*GUARD have the right memory */
	for (j = 0; j < GUARD; j++)
		HPspeech[j] = ConstHPspeech[j];

	/* Update ConstHPspeech for next frame */
	for (j = 0; j < GUARD; j++)
		ConstHPspeech[j] = HPspeech[j + FrameSize];

	/* Calculate prediction coefficients */
	/* reflection coef. not needed-returned in Scratch */
	lpcanalys(pci, Scratch, HPspeech + 2 * GUARD, ORDER, FrameSize, Rs);

	/* Calculate impulse response of 1/A(z) Note: only one IIR filter */
	ImpulseRzp(H, pci, pci, 1.0, 1.0, ORDER, Hlength);

	/* Get energy of H */
	for (j = 0, sum1 = 0; j < Hlength; j++)
		sum1 += H[j] * H[j];

	/* determine spectral transistion degree (set flag if large) -- for frame erasures */
	if (sum1 / LPCgain > 10.0)
		LPCflag = 1;
	else
		LPCflag = 0;

	LPCgain = sum1;

	/* Bandwidth expansion */
	weight(pci, pci, _Gamma_4, ORDER);

	/* Convert prediction coefficients to lsp */
	a2lsp(lsp_nq, pci);

	/* Get residual signal */
	for (j = 0; j < ORDER; j++)
		Scratch[j] = 0;	/* Scratch is used as filter memory */

	lsp2a(pci_nq, Oldlsp_nq);

	GetResidual(residual, HPspeech, pci_nq, Scratch, ORDER, GUARD);

	for (i = 0; i < NoOfSubFrames; i++)
	{
		Interpol(lspi_nq, Oldlsp_nq, lsp_nq, i, ORDER);
		lsp2a(pci_nq, lspi_nq);
		if (i < 2)
			GetResidual(residual + i * (SubFrameSize - 1) + GUARD,
				    HPspeech + i * (SubFrameSize - 1) + GUARD, pci_nq, Scratch, ORDER,
				    SubFrameSize - 1);
		else
			GetResidual(residual + i * (SubFrameSize - 1) + GUARD,
				    HPspeech + i * (SubFrameSize - 1) + GUARD, pci_nq, Scratch, ORDER,
				    SubFrameSize);
	}

	GetResidual(residual + FrameSize + GUARD, HPspeech + FrameSize + GUARD, pci,
		    Scratch, ORDER, GUARD);

    if( tty_option == TTY_NO_GAIN )
    {
        /* Calculate pitch period at the end of the frame, use n.q. lpc coef. */
        fndppf(&delay1, &beta1, residual + GUARD, DMIN, DMAX-1, FrameSize);
        fndppf(&delay, &beta, residual + 2 * GUARD, DMIN, DMAX-1, FrameSize);
    }
    else
    {
        /* Calculate pitch period at the end of the frame, use n.q. lpc coef. */
        fndppf(&delay1, &beta1, residual + GUARD, DMIN, DMAX, FrameSize);
        fndppf(&delay, &beta, residual + 2 * GUARD, DMIN, DMAX, FrameSize);
    }

	if (beta1 > beta + 0.4)
	{
		if (fabs(delay - delay1) > 15)
		{
			beta = beta1;
			delay = delay1;
		}
		else
		{
			beta = (beta + beta1) / 2.0;
			delay = (delay + delay1) / 2.0;
		}
	}

	return (beta);
}

/*======================================================================*/
/*         ..Encode speech data.                                        */
/*----------------------------------------------------------------------*/
void  encode (
	short   rate,
	short*  codeBuf
)
{
	/*....(local) variables....*/
	register int i, j, n;
	float   delayi[3];
	short   subframesize;
	short   Aveidxppg;
	float   sum1;

	/*....execute....*/
	SetEncodeRate(rate);

	/* Handle 800bps seperatly */
	if (bit_rate == 1)
	{
		/* Reset accumulated shift */
		accshift = 0;
		dpm = 0;

		lspmaq(lsp_nq, ORDER, 1, 2, nsub8, nsize8, 0.5, lsp, SScratch, bit_rate, lsptab8);

		/* Bit-pack the quantization indices */
		for (i = 0; i < 2; i++)
			Bitpack(SScratch[i], (unsigned short *) PackedWords, lognsize8[i], PackWdsPtr);

		for (i = 0; i < NoOfSubFrames; i++)
		{
			if (i < 2)
				subframesize = SubFrameSize - 1;
			else
				subframesize = SubFrameSize;

			/* interpolate lsp */
			Interpol(lspi, OldlspE, lsp, i, ORDER);

			/* Convert lsp to PC */
			lsp2a(pci, lspi);

			/* Get lpc gain */
			/* Calculate impulse response of 1/A(z) */
			ImpulseRzp(H, pci, pci, 1.0, 1.0, ORDER, Hlength);
			/* Get energy of H */
			sum1 = 0.0;
			for (j = 0; j < subframesize; j++)
				sum1 += H[j] * H[j];
			sum1 = sqrt(sum1);

			if (lastrateE != 1 && i == 0 && encode_fcnt == 0)
				j = 0;	/* Reset seed */
			else
				j = 1;
			GetExc800bps(Excitation, &idxcbg, sum1, residual + GUARD + i * (SubFrameSize - 1), subframesize, j, i);

			/*...another puff fix...*/
			ZeroInput(zir, pci, pci, Excitation + ACBMemSize - subframesize, GAMMA1, GAMMA2, ORDER, subframesize, 1);
		}

		/* TJ Added 3/8/96 -- Trap for all ones rate 1/8 packet */      
		if ((SScratch[0]&SScratch[1]&0xf)==0xf && (idxcbg==0xff))
		{
			/* Clear Frame Energy Gain MSB if output packet == all ones */
			idxcbg=0x7f;  
		}

		Bitpack(idxcbg, (unsigned short *) PackedWords, 8, PackWdsPtr);
	}
	else
	{			/* bit rates 4 or 8kbps */
		/* Send indication on the spectrum transition */
		if (bit_rate == 4)
			Bitpack(LPCflag, (unsigned short *) PackedWords, 1, PackWdsPtr);

		/* Quantize the LSPs */
		lspmaq(lsp_nq, ORDER, 1, knum, nsub, nsize, 0.5, lsp, SScratch, bit_rate, lsptab);

		/* Bit-pack the quantization indices */
		for (i = 0; i < knum; i++)
			Bitpack(SScratch[i], (unsigned short *) PackedWords, 
				lognsize[i], PackWdsPtr);

		/* Update shiftSTATE with hysteresis */
		if (beta < 0.1)
		{
			accshift = 0;
			dpm = 0;
			shiftSTATE = 0;
		}

		if (accshift > 20)
			shiftSTATE = -1;
		if (accshift < -20)
			shiftSTATE = 1;
		if (accshift <= 10 && shiftSTATE == -1)
			shiftSTATE = 0;
		if (accshift >= -10 && shiftSTATE == 1)
			shiftSTATE = 0;

		/* Control accshift */
		if (shiftSTATE == 1 && beta < 0.4)
			delay += 1.0;
		else if (shiftSTATE == -1 && beta < 0.4)
			delay -= 1.0;
		if (delay > DMAX)
			delay = DMAX;
		if (delay < DMIN)
			delay = DMIN;

        if( tty_option == TTY_NO_GAIN && tty_enc_flag != 0 )
        {
            /* Pack TTY information */
            Bitpack((short) tty_enc_header, (unsigned short *) PackedWords, 2, PackWdsPtr);
            Bitpack((short) tty_enc_char, (unsigned short *) PackedWords, 5, PackWdsPtr);
        }
        else
        {
            /* Pack pitch */
            Bitpack((short) (delay - DMIN), (unsigned short *) PackedWords, 7, PackWdsPtr);
        }

		if (bit_rate == 4)
		{
			j = (short) (delay - pdelay);
			if (abs(j) > 15)
				j = 0;
			else
				j = j + 16;
			Bitpack(j, (unsigned short *) PackedWords, 5, PackWdsPtr);
		}

		/* Smooth interpolation if the difference between delays is too big */
		if (fabs(delay - pdelay) > 15)
			pdelay = delay;

		Aveidxppg = 0;
		/*********************************
        * CELP codebook search procedure *
		*********************************/
		for (i = 0; i < NoOfSubFrames; i++)
		{
			if (i < 2)
				subframesize = SubFrameSize - 1;
			else
				subframesize = SubFrameSize;

			/* interpolate lsp */
			Interpol(lspi, OldlspE, lsp, i, ORDER);
			Interpol(lspi_nq, Oldlsp_nq, lsp_nq, i, ORDER);

			/* Convert lsp to PC */
			lsp2a(pci, lspi);
			lsp2a(pci_nq, lspi_nq);

			/* Get zir */
			ZeroInput(zir, pci_nq, pci, Excitation + ACBMemSize, GAMMA1, GAMMA2, ORDER, subframesize, 0);

			/* Calculate impulse response of 1/A(z) * A(z/g1) / A(z/g2) */
			ImpulseRzp(H, pci_nq, pci, GAMMA1, GAMMA2, ORDER, Hlength);

			/* Interpolate delay */
			Interpol_delay(delayi, &pdelay, &delay, i);

			ComputeACB(residualm, Excitation + ACBMemSize, delayi,
				   residual + GUARD + i * (SubFrameSize - 1),
				   FrameSize + GUARD - i * (SubFrameSize - 1), &dpm,
				   &accshift, beta, subframesize, RSHIFT);

			/* Get weighted speech */
			/* ORIGM */
			SynthesisFilter(origm, residualm, pci_nq, SynMemoryM, ORDER, subframesize);

			/* Weighting filter */
			weight(wpci, pci_nq, GAMMA1, ORDER);
			fir(Scratch, origm, wpci, WFmemFIR, ORDER, subframesize);
			weight(wpci, pci_nq, GAMMA2, ORDER);
			iir(worigm, Scratch, wpci, WFmemIIR, ORDER, subframesize);

			/* Remove Zero input response from weighted speech */
			for (j = 0; j < subframesize; j++)
				worigm[j] -= zir[j];

			/* Calculate closed loop gain */
			getgain(Excitation + ACBMemSize, &sum1, H, &idxppg, ppvq, ppvq_mid, ACBGainSize, 1, worigm, subframesize, Hlength);

			Aveidxppg += idxppg;

			/* Get TARGET for fixed c.b. */
			/* Convolve excitation with H */
			/* ExconvH stored in Scratch memory */
			ConvolveImpulseR(ExconvH, Excitation + ACBMemSize, H, Hlength, subframesize);
			for (j = 0; j < subframesize; j++)
				TARGETw[j] = worigm[j] - ExconvH[j];

			/* Convert TARGET from weighted domain to residual domain */
			Weight2Res(TARGET, TARGETw, pci_nq, pci, GAMMA1, GAMMA2, ORDER, SubFrameSize);

			if (subframesize < SubFrameSize)
				TARGETw[subframesize] = TARGET[subframesize] = Scratch[subframesize] = 0.0;

			/* get delay for current subframe */
			n = (short) ((delayi[1] + delayi[0]) / 2.0 + 0.5);
			/* Compute fixed codebook contribution */
			if (n > subframesize)
				n = 200;

			/* ACELP fixed codebook search */
			if (bit_rate == 4)
			{
				ACELP_Code(TARGETw, TARGET, H, (int) n, sum1, subframesize, Scratch,
					   &fcbGain, y2, fcbIndexVector, 1);
				/* constrain fcb gain */
				fcbGain *= (1.0 - ppvq[idxppg] * 0.15);
			}
			else
			{
				/** Processing half rate case  **/
				ACELP_Code(TARGETw, TARGET, H, (int) n, sum1, subframesize, Scratch,
					   &fcbGain, y2, fcbIndexVector, 0);
				/* Constrain fcb gain */
				fcbGain *= (0.9 - ppvq[idxppg] * 0.1);
			}

			/* quantize fcb gain */
			for (idxcbg = 0, j = 1; j < FCBGainSize; ++j)
			{
				if (fcbGain > (gnvq[j] + gnvq[j - 1]) / 2.0)
					idxcbg = j;
			}
			fcbGain = gnvq[idxcbg];

			/* Add to total excitation */
			for (j = 0; j < subframesize; j++)
				Excitation[j + ACBMemSize] += Scratch[j] * fcbGain;

			/* Update filters memory */
			ZeroInput(zir, pci_nq, pci, Excitation + ACBMemSize, GAMMA1, GAMMA2, ORDER, subframesize, 1);

			/* Update residualm */
			for (j = 0; j < dpm; j++)
				residualm[j] = residualm[j + subframesize];

			/* Update excitation */
			for (j = 0; j < ACBMemSize; j++)
				Excitation[j] = Excitation[j + subframesize];

			/* Pack bits */
			/* ACB gain index */
			Bitpack(idxppg, (unsigned short *) PackedWords, 3, PackWdsPtr);

			if (bit_rate == 4)
			{
				idxcb = fcbIndexVector[0];
				Bitpack(idxcb, (unsigned short *) PackedWords, 8, PackWdsPtr);
				idxcb = fcbIndexVector[1];
				Bitpack(idxcb, (unsigned short *) PackedWords, 8, PackWdsPtr);
				idxcb = fcbIndexVector[2];
				Bitpack(idxcb, (unsigned short *) PackedWords, 8, PackWdsPtr);
				idxcb = fcbIndexVector[3];
				Bitpack(idxcb, (unsigned short *) PackedWords, 11, PackWdsPtr);
				/* FCB gain index */
				Bitpack(idxcbg, (unsigned short *) PackedWords, 5, PackWdsPtr);
			}
			else
			{
				/** Packing half-rate bits **/
				idxcb = fcbIndexVector[0];
				/* FCB shape index  10 bits */
				Bitpack(idxcb, (unsigned short *) PackedWords, 10, PackWdsPtr);
				/* FCB gain index */
				Bitpack(idxcbg, (unsigned short *) PackedWords, 4, PackWdsPtr);
			}
		}
		/* prevent overflow */
		if (Aveidxppg <= 1 && idxppg != 1)
		{
			accshift = 0;
			dpm = 0;
		}

		/* Update encoder variables */
		pdelay = delay;

        if( tty_option == TTY_NO_GAIN && tty_enc_flag != 0 )
        {
            if( bit_rate == 4 )
            {
                /* Zero the delta delay when TTY is in the lag bits */
                PackedWords[2] = PackedWords[2] & ~0x0F80;

                /* Pack TTY Baud Rate information */
                Bitpack((short) tty_enc_baud_rate, (unsigned short *) PackedWords, 1, PackWdsPtr);
            }
        }

	}			/* Ends if for rate of 4 or 8kbps */

#if 0   /*...this was put in post_encode() to allow quick avg_rate calculation...*/
	for (i = 0; i < ORDER; i++)
	{
		OldlspE[i] = lsp[i];
		Oldlsp_nq[i] = lsp_nq[i];
	}
#endif
	lastrateE = bit_rate;

	for (i = 0; i < PACKWDSNUM; i++)
		codeBuf[i] = PackedWords[i];

	encode_fcnt++;
}

/*======================================================================*/
/*         ..Save LSPs.                                                 */
/*----------------------------------------------------------------------*/
void  post_encode ()
{
	/*....(local) variables....*/
	register int  j;

	/*....execute....*/
	for (j = 0; j < ORDER; j++)
	{
		OldlspE[j] = lsp[j];
		Oldlsp_nq[j] = lsp_nq[j];
	}
}
