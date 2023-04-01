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
/*  Module:     decode.c                                                */
/*----------------------------------------------------------------------*/
/*  EVRC Decoder                                                        */
/*======================================================================*/
/*         ..Includes.                                                  */
/*----------------------------------------------------------------------*/
#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>

#include  "d_globs.h"
#include  "fer.h"
#include  "globs.h"
#include  "macro.h"
#include  "proto.h"
#include  "rom.h"
#include  "acelp_co.h"	/* for ACELP */
#include  "typedef.h"
#include  "tty.h"

#if ANSI_EVRC_LSP_EXPANSION
float spread_lsp[ORDER] = { /* same as initial LSP values */
    1.0*0.48/ORDER,
    2.0*0.48/ORDER,
    3.0*0.48/ORDER,
    4.0*0.48/ORDER,
    5.0*0.48/ORDER,
    6.0*0.48/ORDER,
    7.0*0.48/ORDER,
    8.0*0.48/ORDER,
    9.0*0.48/ORDER,
   10.0*0.48/ORDER
};
#endif

/*======================================================================*/
/*         ..Reset RCELP decode rate.                                   */
/*----------------------------------------------------------------------*/
void    SetDecodeRate(short  rate)
{

	/*....execute....*/
	bit_rate = rate;

	switch (rate)
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
/*         ..Reset RCELP decoder parameters.                            */
/*----------------------------------------------------------------------*/
void    InitDecoder()
{
	/*....(local) variables....*/
	int     j;
	float   ftmp;

	/*....execute....*/
	SetDecodeRate(4);
	last_valid_rate = 1;	/* reset last_valid_rate */
	last_fer_flag = 0;		/* reset last_fer_flag */
	decode_fcnt = 0;

	for (j = 0; j < ORDER; j++)
		SynMemory[j] = 0.0;

	ftmp = 0.48 / (float) ORDER;
	OldlspD[0] = ftmp;
	for (j = 1; j < ORDER; j++)
		OldlspD[j] = OldlspD[j - 1] + ftmp;
	for (j = 0; j < ACBMemSize; j++)
		PitchMemoryD[j] = PitchMemoryD_back[j] = 0.0;
	pdelayD = 40.0;
	ave_acb_gain = ave_fcb_gain = 0.0;
	FadeScale = 1.0;
	fer_counter = 0;

	erasureFlag = 0;
	errorFlag = 0;
}

/*======================================================================*/
/*         ..Decode bitstream data.                                     */
/*----------------------------------------------------------------------*/
void  decode (
	short*  codeBuf,
	short   rate,
	short   post_filter,
	float*  outFbuf
)
{
	/*....(local) variables....*/
	register int i, j, n;
	register float *foutP;
	float   delayi[3];
	short   subframesize;
	float   sum1;
#if ANSI_EVRC_SEED
    static long Seed = 0;
#else
	long    Seed;
#endif
	float   sum_acb;
	short   local_rate;
    short   *p_buf16;

	/*....execute....*/

	local_rate = rate;
	FrameErrorHandler(&local_rate, codeBuf);	/* check frame for some errors */

	SetDecodeRate(local_rate);

    p_buf16 = codeBuf;

Lable:
	fer_flag = erasureFlag | errorFlag;

	for (i = 0; i < PACKWDSNUM; i++)
		PackedWords[i] = codeBuf[i];

	/* Re-initialize PackWdsPtr */
	PackWdsPtr[0] = 16;
	PackWdsPtr[1] = 0;

	if (!fer_flag)
	{
		fer_counter -= 1;
		if (fer_counter < 0)
			fer_counter = 0;
	}
	else
	{
		if (fer_counter == 3)
			ave_acb_gain *= 0.75;
#if ANSI_EVRC_SEED
        else
        {
                Seed = (long) (lsp[ORDER - 1] * 65536.0);   /* A random number */
                printf("d_fer(): ANSI_EVRC_SEED\n");
        }
#endif

		fer_counter = 3;

#if ANSI_EVRC_LSP_EXPANSION
        /* Frame Erasure - Use old LSPs with bandwidth expansion */
        for (j = 0; j < ORDER; j++)
        {
            lsp[j] = 0.875*OldlspD[j] + 0.125*spread_lsp[j];
        }
        printf("d_fer(): ANSI_EVRC_LSP_EXPANSION\n");
#endif
		if (bit_rate == 4)
		{
			for (i = 0; i < ACBMemSize; i++)
				PitchMemoryD_back[i] = PitchMemoryD[i];
		}
	}

	if (bit_rate == 1)
	{
        /*************************************************************
        *
        *           Eighth Rate Processing
        *
        **************************************************************/

		delayi[0] = (float) (DMIN);
		delayi[1] = (float) (DMIN);
		delayi[2] = (float) (DMIN);
		/* Bit-unpack the quantization indices */


		for (i = 0; i < 2; i++)
			BitUnpack(&SScratch[i], (unsigned short *) PackedWords, lognsize8[i], PackWdsPtr);
#if ANSI_EVRC_LSP_EXPANSION
        if (!fer_flag)
#else
		if (fer_flag)
		{		/* if frame is erased - use old lsp */
			for (j = 0; j < ORDER; j++)
				lsp[j] = OldlspD[j];
		}
		else
#endif
		{
			lspmaq_dec(ORDER, 1, 2, nsub8, nsize8, lsp, SScratch, 1, lsptab8);

                        /* Check for monotonic LSPs */
			for (j = 1; j < ORDER; j++)
                        {
                            if (lsp[j] <= lsp[j - 1])
                            {
                                errorFlag = 1;
                                goto Lable; 
                            }
                        }

                        /* Check for minimum LSP separation at splits */
                        if( lsp[5] <= (lsp[4]+MIN_LSP_SEP) )
                        {
                            errorFlag = 1;
                            goto Lable; 
                        }
		}

		BitUnpack(&idxcbg, (unsigned short *) PackedWords, 8, PackWdsPtr);

		foutP = outFbuf;

		for (i = 0; i < NoOfSubFrames; i++)
		{
			if (i < 2)
				subframesize = SubFrameSize - 1;
			else
				subframesize = SubFrameSize;

			if (lastrateD != 1 && i == 0 && decode_fcnt == 0)
				j = 0;	/* Reset seed */
			else
				j = 1;

#if ANSI_EVRC_ALL_ONES
            if (ones_dec_cnt>2)
            {
                idxcbg=64;
            }
#endif

			GetExc800bps_dec(PitchMemoryD + ACBMemSize, subframesize, idxcbg, j, i, fer_flag);

			for (j = 0; j < ACBMemSize; j++)
				PitchMemoryD[j] = PitchMemoryD[j + subframesize];

			/* Linear interpolation of lsp */
			Interpol(lspi, OldlspD, lsp, i, ORDER);

			/* Convert lsp to PC */
			lsp2a(pci, lspi);

			/* Synthesis of decoder output signal and postfilter output signal */
			SynthesisFilter(DECspeech, PitchMemoryD + ACBMemSize, pci, SynMemory, ORDER, subframesize);
			if (post_filter)
				apf(DECspeech, pci, DECspeechPF, (delayi[0] + delayi[1]) / 2.0, ALPHA, ALPHA, 0, AGC, 0, ORDER, subframesize, bit_rate);
			else
				V_copy(DECspeech, DECspeechPF, subframesize);

            /*
            *  Always send TTY_EIGHTH_RATE to tty_dec during 1/8 rate and
            *  force the ACB gain != 0 so that it goes into NON_TTY.
            *  This helps prevent false alarms during very high
            *  frame error rates.
            */
            if( tty_option == TTY_NO_GAIN )
            {
                tty_dec_header = TTY_EIGHTH_RATE;
                tty_dec_char = 0;
                tty_dec_flag = tty_dec( p_buf16,
                                        1,
                                        tty_dec_header,
                                        tty_dec_char,
                                        tty_dec_baud_rate,
                                        erasureFlag,
                                        i,
                                        NoOfSubFrames,
                                        subframesize);
                if( tty_dec_flag == 1 )
                    p_buf16 += subframesize;
            }

            if( tty_option == TTY_DISABLED
                || (tty_option == TTY_NO_GAIN && tty_dec_flag == 0)
              )
            {

                /* Convert to short and write to output buffer */
                for (j = 0; j < subframesize; j++)
                {
                    if (DECspeechPF[j] > 32767.0)
                        *p_buf16++ = (short) 32767;
                    else
                    {
                        if (DECspeechPF[j] < -32768)
                            *p_buf16++ = (short) -32767;
                        else
                            *p_buf16++ = (short) DECspeechPF[j];
                    }
                }
            }

        } /* end for(subframes) */

    } /* end if(rate==1) */
	else
    {          
        /*************************************************************
        *
        *           Half and Full Rate Processing
        *
        **************************************************************/

		if (bit_rate == 4)
			BitUnpack(&LPCflag, (unsigned short *) PackedWords, 1, PackWdsPtr);
		else
			LPCflag = 0;

		/* Bit-unpack the quantization indices */
		for (i = 0; i < knum; i++)
			BitUnpack(&SScratch[i], (unsigned short *) PackedWords, 
				lognsize[i], PackWdsPtr);

		if (!fer_flag)
		{
			lspmaq_dec(ORDER, 1, knum, nsub, nsize, lsp, SScratch, bit_rate, lsptab);

                        /* Check for monotonic LSPs */
			for (j = 1; j < ORDER; j++)
                        {
				if (lsp[j] <= lsp[j - 1])
				{
					errorFlag = 1;
					goto Lable; 
				}
                        }

                        /* Check for minimum separation of LSPs at the splits */
                        if( bit_rate == 3 )     /* Check Half Rate splits */
                        {
                            if(   (lsp[3] <= (lsp[2]+MIN_LSP_SEP))
                               || (lsp[6] <= (lsp[5]+MIN_LSP_SEP)))
                            {
                                    errorFlag = 1;
                                    goto Lable; 
                            }
                        }
                        else                    
                        {                      /* Check Full Rate splits */
                            if(   (lsp[2] <= (lsp[1]+MIN_LSP_SEP))
                               || (lsp[4] <= (lsp[3]+MIN_LSP_SEP))
                               || (lsp[7] <= (lsp[6]+MIN_LSP_SEP)))
                            {
                                    errorFlag = 1;
                                    goto Lable; 
                            }
                        }
		}
#if !ANSI_EVRC_LSP_EXPANSION
		else
		{
			/* Frame Erasure - Use old LSPs */		
			for (j = 0; j < ORDER; j++)
				lsp[j] = OldlspD[j];
		}
#endif


        BitUnpack(&idxppg, (unsigned short *) PackedWords, 7, PackWdsPtr);
        tty_dec_header = (idxppg >> 5) & 0x03;
        tty_dec_char = idxppg & 0x01F;
		if (bit_rate == 4)
        {
            /* Get baud rate from bit 171 */
            tty_dec_baud_rate = (PackedWords[10] & 0x0020) >> 5; 
        }
        idxppg += DMIN;

		/* Check in delay is possible */
		if (!fer_flag && idxppg > DMAX)
		{
			errorFlag = 1;
			goto Lable;
		}

		delay = (float) idxppg;

		if (bit_rate == 4)
        {
			BitUnpack(&idxppg, (unsigned short *) PackedWords, 5, PackWdsPtr);

#if ANSI_EVRC_DDELAY_CHECK
            /* check for out-of-range previous delay before Fix_delay_contour */
            if (!fer_flag && idxppg != 0)
            {
                /* prev_delay = delay - delta_delta + 16 */
                n = delay - (idxppg - 16);

                if (n < DMIN)
                {
                    printf("decode(): ANSI_EVRC_DDELAY_CHECK\n");
                    fprintf(stdout,"d_no_fer():  prev_delay < DMIN failure in frame #%d, erasing frame.\n",(int)decode_fcnt + 1);
                    errorFlag = 1;
                    goto Lable; 
                }
                if (n > DMAX)
                {
                    printf("decode(): ANSI_EVRC_DDELAY_CHECK\n");
                    fprintf(stdout,"d_no_fer():  prev_delay > DMAX failure in frame #%d, erasing frame.\n",(int)decode_fcnt + 1);
                    errorFlag = 1;
                    goto Lable; 
                }
            }
#endif
        }

		if (fer_flag)
			delay = pdelayD;
		else
		{
			if (bit_rate == 4)
			{
				/* Fix delay countour of previous erased frame */
				if (fer_counter == 2 && idxppg != 0)
				{
					j = idxppg - 16;
					pdelayD_back = pdelayD;
					pdelayD = delay - (float) j;
					if (fabs(pdelayD - pdelayD_back) > 15)
						pdelayD_back = pdelayD;

					for (i = 0; i < ACBMemSize; i++)
						PitchMemoryD[i] = PitchMemoryD_back[i];

					for (i = 0; i < NoOfSubFrames; i++)
					{
						if (i < 2)
							subframesize = SubFrameSize - 1;
						else
							subframesize = SubFrameSize;

						/* Interpolate delay */
						Interpol_delay(delayi, &pdelayD_back, &pdelayD, i);

						/* Compute adaptive codebook contribution */
						acb_excitation(PitchMemoryD + ACBMemSize, ave_acb_gain, delayi,
							       PitchMemoryD, subframesize);

						for (j = 0; j < ACBMemSize; j++)
							PitchMemoryD[j] = PitchMemoryD[j + subframesize];
					}
				}
			}
		}

		/* Smooth interpolation if the difference between delays is too big */
		if (fabs(delay - pdelayD) > 15)
			pdelayD = delay;

		/* Update fer coefficients */
		if (!fer_flag)
			ave_acb_gain = ave_fcb_gain = 0.0;

		foutP = outFbuf;
		for (i = 0; i < NoOfSubFrames; i++)
		{
			if (i < 2)
				subframesize = SubFrameSize - 1;
			else
				subframesize = SubFrameSize;

			Interpol(lspi, OldlspD, lsp, i, ORDER);

			/* Convert lsp to PC */
			lsp2a(pci, lspi);

			/* Bandwidth expansion after frame erasure only if LPCflag is set */
			if (bit_rate == 4)
			{
				if (LPCflag && fer_counter == 2)
					weight(pci, pci, 0.75, ORDER);
			}

			Interpol_delay(delayi, &pdelayD, &delay, i);

			if (fer_flag && ave_acb_gain < 0.3)
			{
				delayi[0] = rnd_delay[i];
				delayi[1] = rnd_delay[i + 1];
				delayi[2] = rnd_delay[i + 2];
			}

			/* Un-Pack bits */
			/* ACB delay gain */
			BitUnpack(&idxppg, (unsigned short *) PackedWords, 3, PackWdsPtr);

			/* FCB shape index 1 */
			if (bit_rate == 4)
			{
				BitUnpack(SScratch, (unsigned short *) PackedWords, 8, PackWdsPtr);
				fcbIndexVector[0] = *SScratch;
				BitUnpack(SScratch, (unsigned short *) PackedWords, 8, PackWdsPtr);
				fcbIndexVector[1] = *SScratch;
				BitUnpack(SScratch, (unsigned short *) PackedWords, 8, PackWdsPtr);
				fcbIndexVector[2] = *SScratch;
				BitUnpack(SScratch, (unsigned short *) PackedWords, 11, PackWdsPtr);
				fcbIndexVector[3] = *SScratch;
				/* FCB gain index */
				BitUnpack(&idxcbg, (unsigned short *) PackedWords, 5, PackWdsPtr);
			}
			else
			{
				BitUnpack(&idxcb, (unsigned short *) PackedWords, 10, PackWdsPtr);
				/* FCB gain index */
				BitUnpack(&idxcbg, (unsigned short *) PackedWords, 4, PackWdsPtr);
			}

			/* Compute adaptive codebook contribution */
			if (fer_flag)
				sum_acb = ave_acb_gain;
			else
			{
				sum_acb = ppvq[idxppg];
				ave_acb_gain += sum_acb / NoOfSubFrames;
			}

			acb_excitation(PitchMemoryD + ACBMemSize, sum_acb, delayi,
				       PitchMemoryD, subframesize);

			/* Compute fixed codebook contribution */
			if (!fer_flag)
				ave_fcb_gain += gnvq[idxcbg] / NoOfSubFrames;

			/* Compute fixed codebook contribution */
			if ((bit_rate == 4) && !fer_flag)	/* full rate */
				dec8_35(fcbIndexVector, Scratch);

			if ((bit_rate==3) && !fer_flag) /* half rate */
				dec3_10(idxcb, Scratch);

			if (!fer_flag)
			{
				if (sum_acb > 0.9)
					sum_acb = 0.9;
				if (sum_acb < 0.2)
					sum_acb = 0.2;

				/* get intrpolated delay for this subframe */
				n = (short) ((delayi[1] + delayi[0]) / 2.0 + 0.5);
				if (n > subframesize)
					n = 200;

				pit_shrp(Scratch, (int) (n), sum_acb, subframesize);

				sum1 = gnvq[idxcbg];
				for (j = 0; j < subframesize; j++)
					PitchMemoryD[j + ACBMemSize] += sum1 * Scratch[j];
			}

			/* Add fading */
			if (fer_flag)
			{
				for (j = 0; j < subframesize; j++)
					PitchMemoryD[ACBMemSize + j] *= FadeScale;
				FadeScale -= 0.05;
				if (FadeScale < 0.0)
					FadeScale = 0.0;
			}
			else
			{
				FadeScale += 0.2;
				if (FadeScale > 1.0)
					FadeScale = 1.0;
			}

			for (j = 0; j < ACBMemSize; j++)
				PitchMemoryD[j] = PitchMemoryD[j + subframesize];

			if (fer_flag)
			{
				/* Use gaussian noise */
				if (ave_acb_gain < 0.4)
				{
#if !ANSI_EVRC_SEED
					Seed = (long) (lsp[ORDER - 1] * 65536.0);	/* A random number */
#endif
					for (j = 0; j < subframesize; j++)
						PitchMemoryD[ACBMemSize + j] += 0.1 * ave_fcb_gain * ran_g(&Seed);
				}
			}

			/* Synthesis of decoder output signal and postfilter output signal */
			SynthesisFilter(DECspeech, PitchMemoryD + ACBMemSize, pci, SynMemory, ORDER, subframesize);


			/* Postfilter */
            if( post_filter )
            {
                if (bit_rate == 4)
                    apf(DECspeech, pci, DECspeechPF, (delayi[0] + delayi[1]) / 2.0, ALPHA, BETA, U, AGC, LTGAIN, ORDER, subframesize, bit_rate);
                else
                    apf(DECspeech, pci, DECspeechPF, (delayi[0] + delayi[1]) / 2.0, HALF_ALPHA, BETA, HALF_U, AGC, LTGAIN, ORDER, subframesize, bit_rate);
            }
			else
            {
				V_copy(DECspeech,DECspeechPF,subframesize);
            }

            if( tty_option == TTY_NO_GAIN )
            {
                tty_dec_flag = tty_dec( p_buf16,
                                        idxppg,
                                        tty_dec_header,
                                        tty_dec_char,
                                        tty_dec_baud_rate,
                                        erasureFlag,
                                        i,
                                        NoOfSubFrames,
                                        subframesize);

                /* Update pointer if tty was generated */
                if( tty_dec_flag == 1)
                {
                    p_buf16 += subframesize;
                }
            }
                                         

            if( tty_option == TTY_DISABLED
                || (tty_option == TTY_NO_GAIN && tty_dec_flag == 0)
              )
            {

                /* Convert to short and write to output buffer */
                for (j = 0; j < subframesize; j++)
                {
                    if (DECspeechPF[j] > 32767.0)
                        *p_buf16++ = (short) 32767;
                    else
                    {
                        if (DECspeechPF[j] < -32768)
                            *p_buf16++ = (short) -32767;
                        else
                            *p_buf16++ = (short) DECspeechPF[j];
                    }
                }
            }

        } /* end for(subframes) */

		pdelayD = delay;

    } /* Ends case for rate=4kbps or 8kbps */


    /* update decoder variables */
	for (i = 0; i < ORDER; i++)
		OldlspD[i] = lsp[i];
	lastrateD = bit_rate;

	decode_fcnt++;
	last_fer_flag=fer_flag;
}
