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
/*  Module:     main.c                                                  */
/*----------------------------------------------------------------------*/
/*======================================================================*/
/*         ..Includes.                                                  */
/*----------------------------------------------------------------------*/
//#include <getopt.h>  /* Not needed for DOS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h> 
//#include <getopt.h>
#include "getopt.h"

#include "macro.h"
#include "bqiir.h"
#include "decode.h"
#include "encode.h"
#include "ns127.h"
#include "rda.h"
#include "typedef.h"
#include "tty.h"
#include "tty_dbg.h"

extern char     *trans_fname;
extern int      fer_sim_seed;

extern void     fer_sim( short *rate );

/*======================================================================*/
/*         ..Typedefs.                                                  */
/*----------------------------------------------------------------------*/
typedef long   int32;
typedef short  int16;

/*======================================================================*/
/*         ..Defines.                                                   */
/*----------------------------------------------------------------------*/
/* Version 3.1  4 Dec 2000:
*    Contains MIN_TTY_ONSET_FIX so that the TTY encoder sends a
*    minimum of 4 TTY_SILENCE message to the decoder when Baudot tones
*    are first detected.
*
* Version 4.0  21 Oct 2002
*    ANSI-127 fixes:
*      -Bottom guard for offset in cshift.c
*      -Erasing all-zeros Packets
*      -Muting after multiple all-ones packets
*      -Fixed seed for frame erasures
*      -Add delta delay sanity check
*      -LSP bandwidth expansion for frame erasures
*    TTY for 45.45 baud and 50 baud
*    Minimum input energy threshold in TTY detector
*
* Version 4.1 15 Oct 2003
*    -Changed function round() to round32()
*
* Version 4.2 25 jan 2004
*    -Added -s <filename> option for signaling file to support dim &
*     burst simulation.
*/
    
#define PROGRAM_VERSION "4.2"
#define PROGRAM_DATE    "25 Jan 2004"

#define  SPEECH_BUFFER_LEN        160
#define  LOOKAHEAD_LEN             80
#define  BITSTREAM_BUFFER_LEN      12       /*  data + 1 word for rate  */

/*======================================================================*/
/*         ..Structs.                                                   */
/*----------------------------------------------------------------------*/
typedef struct {
	char*   input_filename;
	char*   output_filename;
	int16   encode_only;
	int16   decode_only;
	int16   max_rate;             int16  max_rate_default;
	int16   min_rate;             int16  min_rate_default;
	int16   post_filter;          int16  post_filter_default;
	int16   noise_suppression;    int16  noise_suppression_default;
	int16   ibuf_len;
	int16   obuf_len;
	char	*signaling_filename;
	FILE	*fp_signaling;
} EvrcArgs;


/*======================================================================*/
/*         ..Print banner.                                              */
/*----------------------------------------------------------------------*/
void  banner (
	FILE* fp
)
{
	/*....execute....*/
	fprintf(fp,"\n\
---------------------------------------------------------------------\n\
>>>>>>>>>>>>>>>>> CDMA Enhanced Variable-Rate Codec <<<<<<<<<<<<<<<<<\n\
<<<<<<<<<<<<<<<<<<<< Floating-point C simulation >>>>>>>>>>>>>>>>>>>>\n\
>>>>>>>>>>>>>>>>>>>>>>>  - TTY/TDD Edition -  <<<<<<<<<<<<<<<<<<<<<<<\n\
<<<<<<<<<<<<<<<<<<<<<< Version %s %s <<<<<<<<<<<<<<<<<<<<<<\n\
---------------------------------------------------------------------\n",
PROGRAM_VERSION, PROGRAM_DATE);
}

/*======================================================================*/
/*         ..Usage.                                                     */
/*----------------------------------------------------------------------*/
void  usage (
	FILE*  fp,
	char*  prog_name
)
{
	/*....execute....*/
	banner(fp);
	fprintf(fp,"\
 usage:\n\
        %s  <required_args>  [optional_args]\n\
\n\
   required_args:\n\
        -i <filename>   Input filename.\n\
        -o <filename>   Output filename.\n\
\n\
   optional_args:\n\
        -e              Encode only (infile=speech, outfile = bitstream).\n\
        -d              Decode only (infile=bitstream, outfile = speech).\n\
        -h <rate>       Maximum rate, 1(eighth),3(half), or 4(full); (default=4).\n\
        -l <rate>       Minimum rate, 1(eighth),3(half), or 4(full); (default=1).\n\
        -p <1 or 0>     Enable (1)/disable (0) post filter, (default=1).\n\
        -n <1 or 0>     Enable (1)/disable (0) noise suppression, (default=1).\n\
        -t <tty_option> Enable TTY processing\n\
                            0 = TTY handling OFF [default]\n\
                            1 = NO GAIN solution for TTY\n\
        -f <trans_file> FER transition probability file (for simulating FERs).\n\
        -S <seed>       Initial seed for FER simulator\n\
        -s <filename>   Signaling filename\n\
\n",
		prog_name
	);
}

/*======================================================================*/
/*         ..Print command line arguments.                              */
/*----------------------------------------------------------------------*/
void  print_eargs (
	FILE*      fp,
	EvrcArgs*  e
)
{
	/*....execute....*/
	banner(fp);
	if (e->encode_only && !e->decode_only) {
		e->ibuf_len = SPEECH_BUFFER_LEN;
		e->obuf_len = BITSTREAM_BUFFER_LEN - 1;
		fprintf(fp,"          input_speechfile == \"%s\"\n",e->input_filename);
		fprintf(fp,"          output_bitstream => \"%s\"\n",e->output_filename);
	} else if (e->decode_only && !e->encode_only) {
		e->ibuf_len = BITSTREAM_BUFFER_LEN - 1;
		e->obuf_len = SPEECH_BUFFER_LEN;
		fprintf(fp,"           input_bitstream => \"%s\"\n",e->input_filename);
		fprintf(fp,"         output_speechfile == \"%s\"\n",e->output_filename);
	} else if (!e->encode_only && !e->decode_only) {
		e->ibuf_len = SPEECH_BUFFER_LEN;
		e->obuf_len = SPEECH_BUFFER_LEN;
		fprintf(fp,"          input_speechfile == \"%s\"\n",e->input_filename);
		fprintf(fp,"         output_speechfile == \"%s\"\n",e->output_filename);
	}
	if (e->signaling_filename != NULL) {
		fprintf(fp,"            signaling file == \"%s\"\n",e->signaling_filename);
	}
	if (e->max_rate_default) {
		fprintf(fp,"                  max_rate == %d\n",    e->max_rate);
	} else {
		fprintf(fp,"                  max_rate => %d\n",    e->max_rate);
	}
	if (e->min_rate_default) {
		fprintf(fp,"                  min_rate == %d\n",    e->min_rate);
	} else {
		fprintf(fp,"                  min_rate => %d\n",    e->min_rate);
	}
	if (e->post_filter) {
		if (e->post_filter_default) {
			fprintf(fp,"               post_filter == ON\n");
		} else {
			fprintf(fp,"               post_filter => ON\n");
		}
	} else {
		if (e->post_filter_default) {
			fprintf(fp,"               post_filter == OFF\n");
		} else {
			fprintf(fp,"               post_filter => OFF\n");
		}
	}
	if (e->noise_suppression) {
		if (e->noise_suppression_default) {
			fprintf(fp,"         noise_suppression == ON\n");
		} else {
			fprintf(fp,"         noise_suppression => ON\n");
		}
	} else {
		if (e->noise_suppression_default) {
			fprintf(fp,"         noise_suppression == OFF\n");
		} else {
			fprintf(fp,"         noise_suppression => OFF\n");
		}
	}
    switch( tty_option )
    {
        case 1:
            fprintf(fp,"                       tty => NO GAIN\n");
            break;
        default:
            tty_option = 0;
            fprintf(fp,"                       tty => OFF\n");
            break;
    }
    if( trans_fname != NULL )
    {
        fprintf(fp,"  FER Transition Prob File => %s\n", trans_fname);
        if( fer_sim_seed != 0 )
        {
           fprintf(fp,"                  FER seed => %d\n", fer_sim_seed);
        }
    }
	fprintf(fp,"---------------------------------------------------------------------\n");
}

/*======================================================================*/
/*         ..Get command line arguments.                                */
/*----------------------------------------------------------------------*/
int16 get_eargs (
    int         argc,
    char*       argv[],
    EvrcArgs*   eargs
)
{
	/*....(local) variables....*/
	int16      option;

	/*....execute....*/
	if (argc < 5)
    {
        return(0);
	}
    else
    {
		eargs->input_filename     = NULL;
		eargs->output_filename    = NULL;
		eargs->encode_only        = 0;
		eargs->decode_only        = 0;
		eargs->max_rate           = 4;    eargs->max_rate_default          = 1;
		eargs->min_rate           = 1;    eargs->min_rate_default          = 1;
        eargs->post_filter        = 1;    eargs->post_filter_default       = 1;
		eargs->noise_suppression  = 1;    eargs->noise_suppression_default = 1;
		eargs->ibuf_len           = SPEECH_BUFFER_LEN;
		eargs->obuf_len           = SPEECH_BUFFER_LEN;
        tty_option                = 1;
		eargs->signaling_filename = NULL;
		eargs->fp_signaling       = NULL;

        while ((option=getopt(argc,argv,"i:o:redh:l:p:n:t:D:f:S:s:")) != EOF) {
			switch (option) {
			case 'i':
				eargs->input_filename = argv[optind-1];
				break;
			case 'o':
				eargs->output_filename = argv[optind-1];
				break;
			case 'e':
				eargs->encode_only = 1;
				if (eargs->decode_only) {
					fprintf(stderr,"%s:  ERROR - Encode with decode.\n",argv[0]);
					exit(-1);
				}
				break;
			case 'd':
				eargs->decode_only = 1;
				if (eargs->encode_only) {
					fprintf(stderr,"%s:  ERROR - Decode with encode.\n",argv[0]);
					exit(-1);
				}
				break;
			case 'h':
				eargs->max_rate = (int16)atoi(argv[optind-1]);
				if (eargs->max_rate > 4) eargs->max_rate = 4;
				if (eargs->max_rate < 1) eargs->max_rate = 1;
				eargs->max_rate_default = 0;
				break;
			case 'l':
				eargs->min_rate = (int16)atoi(argv[optind-1]);
				if (eargs->min_rate > 4) eargs->min_rate = 4;
				if (eargs->min_rate < 1) eargs->min_rate = 1;
				eargs->min_rate_default = 0;
				break;
			case 'p':
				eargs->post_filter = (int16)atoi(argv[optind-1]);
				if (eargs->post_filter != 0) eargs->post_filter = 1;
				eargs->post_filter_default = 0;
				break;
			case 'n':
				eargs->noise_suppression = (int16)atoi(argv[optind-1]);
				if (eargs->noise_suppression != 0) eargs->noise_suppression = 1;
				eargs->noise_suppression_default = 0;
				break;
            case 't':
                tty_option = (int16)atoi(argv[optind-1]);
				break;
            case 'D':
                tty_debug_print_flag = (short) atoi(argv[optind-1]) & TTY_DEBUG_PRINT;
                tty_debug_flag = (short) atoi(argv[optind-1]) & TTY_DEBUG_DUMP;
				break;
            case 'f':
                trans_fname = argv[optind-1];
				break;
            case 'S':
                fer_sim_seed = (int16)atoi(argv[optind-1]);
				break;
            case 's':
				eargs->signaling_filename = argv[optind-1];
                eargs->fp_signaling = fopen(argv[optind-1],"rb");
				if( eargs->fp_signaling == NULL )
				{
					fprintf(stderr,"Error opening signaling file %s\n",argv[optind-1]);
					exit(-1);
				}
				break;
			}
		}
		if (eargs->min_rate > eargs->max_rate) {
			eargs->min_rate = eargs->max_rate;
		}
	}
    return(1);
}

/*======================================================================*/
/*         ..Returns number of frames in a binary file.                 */
/*----------------------------------------------------------------------*/
unsigned long GetNumFrames (
	FILE*  fp,
	int16  blocksize
)
{
	/*....(local) variables....*/
    unsigned long   position;
    unsigned long   numFrames;

	/*....execute....*/
	position = ftell(fp);
	fseek(fp,0L,2);
	numFrames = ftell(fp) / blocksize;
	fseek(fp,position,0);
	return(numFrames);
}

/*======================================================================*/
/*         ..Hi-pass filter with 120 Hz iir 3-stage biquad.             */
/*----------------------------------------------------------------------*/
void  HiPassFilter (
	float*  buffer
)
{
	/*....execute....*/
	bqiir(buffer);
}

/*======================================================================*/
/*         ..Noise suppression on 10ms buffer.                          */
/*----------------------------------------------------------------------*/
void  SuppressNoise (
	float*  buffer
)
{
	/*....execute....*/
	noise_suprs(buffer);
}

/*======================================================================*/
/*         ..Rate determination.                                        */
/*----------------------------------------------------------------------*/
int16  RDA (
	float*  ibuffer,
	short   max_rate,
	short   min_rate,
	float   beta
)
{
	/*....(local) variables....*/
	int16  rate;

	/*....execute....*/
	rate = select_rate(ibuffer,max_rate,min_rate,beta);
	return(rate);
}

/*======================================================================*/
/*         ..Pre-encoder, return beta, et. al....                       */
/*----------------------------------------------------------------------*/
float  PreEncode(
	float*     ibuffer,
	float*     R,
	EvrcArgs*  eargs
)
{
	/*....(static) variables....*/
	static float   buf[SPEECH_BUFFER_LEN + LOOKAHEAD_LEN];

	/*....(local) variables....*/
	float*  sP;
	float*  dP;
	int16   j;
	float   beta;

	/*....execute....*/
	for (sP=ibuffer,dP=buf+LOOKAHEAD_LEN,j=0; j<SPEECH_BUFFER_LEN; j++,sP++,dP++) {
		*dP = *sP;
	}
	beta = pre_encode(buf,R);
	for (sP=buf+SPEECH_BUFFER_LEN,dP=buf,j=0; j<LOOKAHEAD_LEN; j++,sP++,dP++) {
		*dP = *sP;
	}
	return(beta);
}

/*======================================================================*/
/*         ..Speech encoder.                                            */
/*----------------------------------------------------------------------*/
void  Encode(
	int16   rate,
	int16*  obuffer
)
{
	/*....execute....*/
	encode(rate,obuffer);
	post_encode();
}

/*======================================================================*/
/*         ..Speech decoder.                                            */
/*----------------------------------------------------------------------*/
void  Decode(
	int16*  ibuffer,
	int16   rate,
	int16   post_filter,
	float*  obuffer
)
{
	/*....execute....*/
	decode(ibuffer,rate,post_filter,obuffer);
}

/*======================================================================*/
/*         ..Main.                                                      */
/*----------------------------------------------------------------------*/
int main (int argc,char* argv[])
{
	/*....(local) variables....*/

	FILE*      ifileP;
	FILE*      ofileP;
    EvrcArgs   evrc_args;
    EvrcArgs*  eargs;

    int16      buf16[SPEECH_BUFFER_LEN];
	int16*     buf16P;
	float      buf[SPEECH_BUFFER_LEN];
	float*     bufP;

    unsigned long    buf_count;
	int16      ibuf_len;
	int16      obuf_len;
    unsigned long j;
	int16      k;
	int16      rate;
	float      beta;
	float      rate_sum;
	float      avg_rate;
	float      R[17];
    int16      min_rate;
	int16	   max_rate;
	char       signaling;

    unsigned long   fer_count = 0;  /* For computing FER */

	/*....execute....*/

	/*...get arguments and check usage...*/
    eargs = &evrc_args;     /* SAB: added to eliminate malloc in get_eargs() */
    if ( get_eargs(argc,argv,eargs) == 0) {
		usage(stdout,argv[0]);
		exit(-1);
	}
	print_eargs(stdout,eargs);

	/*...open files...*/
	if ((ifileP=fopen(eargs->input_filename,"rb")) == NULL) {
		fprintf(stderr,"%s:  ERROR - Unable to open input file \"%s\".\n",
			argv[0],
			eargs->input_filename
		);
		exit(-1);
	}
	if ((ofileP=fopen(eargs->output_filename,"wb")) == NULL) {
		fprintf(stderr,"%s:  ERROR - Unable to open output file \"%s\".\n",
			argv[0],
			eargs->output_filename
		);
		exit(-1);
	}

	/*...loop counter max...*/
	if (eargs->decode_only) {
		buf_count = GetNumFrames(ifileP,sizeof(int16)*BITSTREAM_BUFFER_LEN);
	} else {
		buf_count = GetNumFrames(ifileP,sizeof(int16)*SPEECH_BUFFER_LEN);
	}

	/*...processing loop...*/
	InitEncoder();
	InitDecoder();

    if( tty_debug_flag )
        tty_debug();

    if( tty_option == TTY_NO_GAIN )
    {
        init_tty_enc( &tty_enc_char, &tty_enc_header, &tty_enc_baud_rate);
        init_tty_dec();
    }

	rate_sum = 0.0;
	avg_rate = 0.0;

	ibuf_len = eargs->ibuf_len;
	obuf_len = eargs->obuf_len;
	j = 1;
	if (!eargs->decode_only) {
		while ((fread(buf16,sizeof(int16),ibuf_len,ifileP)) == ibuf_len) {

			for (buf16P=buf16,bufP=buf,k=0; k<ibuf_len; k++,buf16P++,bufP++) {
				*bufP = (float)*buf16P;
			}

			HiPassFilter(buf);

            if( tty_option == TTY_NO_GAIN )
            {
                tty_enc_flag = tty_enc( &tty_enc_char,
                                        &tty_enc_header,
                                        &tty_enc_baud_rate,
                                        buf,
                                        FrameSize );
            }

            if( tty_option == TTY_DISABLED || tty_enc_flag == 0 )
            {
                if (eargs->noise_suppression)
                {
                    SuppressNoise(buf);
                    SuppressNoise(buf+ibuf_len/2);
                }
            }

			beta = PreEncode(buf,R,eargs);

            if( tty_option == TTY_NO_GAIN && tty_enc_flag != 0 )
            {
                /* Force to full rate for TTY packets */
                min_rate = MIN(4,eargs->max_rate);
            }
            else
            {
                min_rate = eargs->min_rate;
            }
            
			max_rate = eargs->max_rate;
			if( eargs->fp_signaling != NULL )
			{
				k = fread(&signaling, sizeof(char), 1, eargs->fp_signaling);
				if( k == 0 )
				{
					fseek(eargs->fp_signaling,0,0);
					k = fread(&signaling, sizeof(char), 1, eargs->fp_signaling);
					if( k == 0 )
					{
						fprintf(stderr,"Error reading signaling file %s: File Empty!\n",eargs->signaling_filename);
						exit(-1);
					}
				}
				if( signaling )
				{
					max_rate = MIN(3,eargs->max_rate);
				}
			}

            rate = RDA(R,max_rate,min_rate,beta);

            /* Force rate if option set in command line */
			if (rate > eargs->max_rate) rate = eargs->max_rate;
			if (rate < eargs->min_rate) rate = eargs->min_rate;

			switch (rate) {
			case 1:
				rate_sum += 1;
				break;
			case 3:
				rate_sum += 4;
				break;
			default:
				rate_sum += 8;
				break;
			}
			avg_rate = (rate_sum / (float)j) * 1.2;

			Encode(rate,buf16);

            if( trans_fname != NULL )
            {
                fer_sim( &rate );
            }

            if( rate == ERASURE )
            {
                fer_count++;
            }

			if (eargs->encode_only)
            {
				fwrite(&rate,sizeof(int16),1,ofileP);
				fwrite(buf16,sizeof(int16),obuf_len,ofileP);
                fprintf(stderr,"Encoding %lu of %lu  avg_rate= %6.2f  FER= %4.2f%%\r",
                    j,buf_count,avg_rate, 100.0*fer_count/j);
			} else {

				Decode(buf16,rate,eargs->post_filter,buf);

				fwrite(buf16,sizeof(int16),obuf_len,ofileP);
                fprintf(stderr,"Encoding/Decoding %lu of %lu   avg_rate= %6.2f  FER= %4.2f%%\r",
                    j,buf_count,avg_rate, 100.0*fer_count/j);
				}
			j++;
		}
	} else {
		while ((fread(&rate,sizeof(int16),1,ifileP)) == 1)
        {
			if ((fread(buf16,sizeof(int16),ibuf_len,ifileP)) == ibuf_len)
            {

                if( trans_fname != NULL )
                {
                    fer_sim( &rate );
                }

                if( rate == ERASURE )
                {
                    fer_count++;
                }

				Decode(buf16,rate,eargs->post_filter,buf);

                fprintf(stderr,"Decoding %lu of %lu  FER= %4.2f%%\r",
                    j,buf_count, 100.0*fer_count/j);
				fwrite(buf16,sizeof(int16),obuf_len,ofileP);
				j++;
			}
		}
	}

	if (eargs->fp_signaling != NULL)
	   fclose(eargs->fp_signaling);

	fprintf(stderr,"\n");
    exit(0);
}


//#include "../microwo_evrc_lib.h"
//
//void myRead( char* buf , int size , FILE* fp )
//{
//	int len = 0 ;
//	len = fread( buf , size , 1 , fp);
//	if ( len < 0 )
//	{
//		exit(0);
//	}
//}
//
//void myWrite( short* buf , int size )
//{
//	FILE* fp = fopen("./VoiceData.pcm" , "ab");
//
//	if ( NULL == fp )
//	{
//		return ;
//	}
//	fwrite( buf , sizeof(short) * size , 1 , fp);
//	fclose(fp);
//}
//
//int main()
//{
//	char inBuf[116];
//	short outBuf[2048];
//	short inBufLen = 0 ;
//	short outBufLen = 0 ;
//	struct VoiceBuf* buf ;
//	int len = 0 ;
//	int i = 0 ;
//	FILE* fp = fopen("./EvrcData.log" , "rb");
//	
//	
//	if ( NULL == fp )
//	{
//		return 0;
//	}
//	while ((fread((char*)&len, 4, 1, fp)) == 1)
//	{
//		inBufLen = len;
//
//		myRead( (char*)inBuf , inBufLen , fp );
//
//		parse_mw_evrc_packet(inBuf , inBufLen , outBuf , &outBufLen);
//
//
//		myWrite(outBuf , outBufLen );
//		i ++ ;
//		printf("%d\n" , i );
//	}
//	
//	printf("end \n");
//	getchar();
//
//	return 0 ;
//}


