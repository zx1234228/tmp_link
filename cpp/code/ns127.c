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
/* ns127.c */

/*****************************************************************
*
* EVRC Noise Suppression
*
* Input:  The input to the function is a float pointer to the
*         array of data to be noise suppressed.
*
* Output: There is no return value.  The input array is replaced
*         with the noise suppressed values.
*
*
*****************************************************************/

/* Includes */

#include		<math.h>


/* Defines */

#define			FRM_LEN			80
#define			DELAY			24
#define			FFT_LEN			128

#define			NUM_CHAN		16
#define			LO_CHAN			0
#define			MID_CHAN		5
#define			HI_CHAN			15

#define			TRUE			1
#define			FALSE			0

#define			UPDATE_THLD		35
#define			METRIC_THLD		45
#define			INDEX_THLD		12
#define			SETBACK_THLD		12
#define			SNR_THLD		6
#define			INDEX_CNT_THLD		5
#define			UPDATE_CNT_THLD		50

#define			NORM_ENRG		(1.0)	/* use (32768.0 * 32768.0) for fractional */
#define			NOISE_FLOOR		(1.0 / NORM_ENRG)
#define			MIN_CHAN_ENRG		(0.0625 / NORM_ENRG)
#define			INE			(16.0 / NORM_ENRG)

#define			MIN_GAIN		(-13.0)
#define			GAIN_SLOPE		0.39

#define			CNE_SM_FAC		0.1
#define			CEE_SM_FAC		0.55
#define			PRE_EMP_FAC		(-0.8)
#define			DE_EMP_FAC		0.8

#define			HYSTER_CNT_THLD		6		/* forced update constants... */
#define			HIGH_TCE_DB		(50.)		/* 50 - 10log10(NORM_ENRG) */
#define			LOW_TCE_DB		(30.)		/* 30 - 10log10(NORM_ENRG) */
#define			TCE_RANGE		(HIGH_TCE_DB - LOW_TCE_DB)
#define			HIGH_ALPHA		0.99
#define			LOW_ALPHA		0.50
#define			ALPHA_RANGE		(HIGH_ALPHA - LOW_ALPHA)
#define			DEV_THLD		28.0


/* Macros */

#define			min(a,b)		((a)<(b)?(a):(b))
#define			max(a,b)		((a)>(b)?(a):(b))
#define			square(a)		((a)*(a))


/* The noise supression function */

void		noise_suprs (float * farray_ptr)

{

/* Static variables */

/* The channel table is defined below.  In this table, the
lower and higher frequency coefficients for each of the 16
channels are specified.  The table excludes the coefficients
with numbers 0 (DC), 1, and 64 (Foldover frequency).  For
these coefficients, the gain is always set at 1.0 (0 dB). */

static int	ch_tbl [NUM_CHAN][2] = {

			{ 2,  3},
			{ 4,  5},	
			{ 6,  7},
			{ 8,  9},
			{10, 11},
			{12, 13},
			{14, 16},
			{17, 19},
			{20, 22},
			{23, 26},
			{27, 30},
			{31, 35},
			{36, 41},
			{42, 48},
			{49, 55},
			{56, 63}
				      };

/* The voice metric table is defined below.  It is a non-
linear table with a deadband near zero.  It maps the SNR
index (quantized SNR value) to a number that is a measure
of voice quality. */ 

static int	vm_tbl [90] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7,
		8, 8, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 15,
		15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 23, 24,
		24, 25, 26, 27, 28, 28, 29, 30, 31, 32, 33, 34,
		35, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44, 45,
		46, 47, 48, 49, 50, 50, 50, 50, 50, 50, 50, 50,
		50, 50};

static int	first = TRUE;

static float	pre_emp_mem = 0.0, de_emp_mem = 0.0;

static float	ch_enrg [NUM_CHAN];

static float	ch_noise [NUM_CHAN];

static float	overlap [FFT_LEN-FRM_LEN];	/* initialized to 0.0 automatically */

static float	ch_gain [FFT_LEN/2];

static int	update_cnt = 0; 

static float	window [DELAY+FRM_LEN];
static float	window_overlap [DELAY];

static int	hyster_cnt;		/* forced update statics... */
static int	last_update_cnt;
static float	ch_enrg_long_db [NUM_CHAN];

static unsigned long frame_cnt;
				


/* Automatic variables */

float		data_buffer [FFT_LEN], enrg, snr;
float		tne, tce, gain, ftmp1, ftmp2;
int		ch_snr [NUM_CHAN];
int		i, j, j1, j2;
int		vm_sum;
int		update_flag, modify_flag, index_cnt;

float		ch_enrg_dev;		/* for forced update... */
float		ch_enrg_db [NUM_CHAN];
float		alpha;



/* Functions */

void		r_fft (float *, int);
void		init_window (float *, int, float);


/* Init the window function, channel gains one time */

if (first == TRUE) {

    init_window(window,
		DELAY+FRM_LEN,
		(float)DELAY/(DELAY+FRM_LEN));

    ch_gain [0] = (ch_gain [1] =  1.0);

}


/* Increment frame counter */
frame_cnt++;

/* Preemphasize the input data and store in the data buffer with
appropriate delay */

  for (i = 0; i < DELAY; i++)
    data_buffer [i] = window_overlap [i];

  data_buffer [DELAY] = *farray_ptr + PRE_EMP_FAC * pre_emp_mem;

  for (i = DELAY + 1, j = 1; i < DELAY + FRM_LEN; i++, j++) 
    data_buffer [i] = *(farray_ptr + j) + PRE_EMP_FAC * 
                      *(farray_ptr + j - 1); 

  pre_emp_mem = *(farray_ptr + FRM_LEN - 1);

  for (i = DELAY + FRM_LEN; i < FFT_LEN; i++)
    data_buffer [i] = 0.0;


/* update window_overlap buffer */

  for (i = 0, j = FRM_LEN; i < DELAY; i++, j++) 
    window_overlap [i] = data_buffer [j];


/* Apply window to frame prior to FFT */

  for (i = 0; i < FRM_LEN+DELAY; i++)
    data_buffer [i] *= window [i];


/* Perform FFT on the data buffer */

  r_fft (data_buffer, +1);


/* Estimate the energy in each channel */

  alpha = (first == TRUE) ? 1.0 : CEE_SM_FAC;

  for (i = LO_CHAN; i <= HI_CHAN; i++) {

    enrg = 0.0;
    j1 = ch_tbl [i][0], j2 = ch_tbl [i][1];
    for (j = j1; j <= j2; j++)
      enrg += square(data_buffer [2*j]) + square(data_buffer [2*j+1]); 
    enrg /= (float) (j2 - j1 + 1);
    ch_enrg [i] = (1 - alpha) * ch_enrg [i] + alpha * enrg;
    if (ch_enrg [i] < MIN_CHAN_ENRG) ch_enrg [i] = MIN_CHAN_ENRG;

  }


/* Initialize channel noise estimate to channel energy of first four frames */

  if (frame_cnt <= 4)
    for (i = LO_CHAN; i <= HI_CHAN; i++)
      ch_noise [i] = max(ch_enrg [i], INE);


/* Compute the channel SNR indices */

  for (i = LO_CHAN; i <= HI_CHAN; i++) {

    snr = 10.0 * log10 ((double) (ch_enrg [i] / ch_noise [i]));
    if (snr < 0.0) snr = 0.0;
    ch_snr [i] = (snr + 0.1875) / 0.375;

  }


/* Compute the sum of voice metrics */

  vm_sum = 0;
  for (i = LO_CHAN; i <= HI_CHAN; i++) {

    j = min(ch_snr[i],89);
    vm_sum += vm_tbl [j];

  }


/* Compute the total noise estimate (tne) and total channel energy estimate (tce) */

  tne = tce = 0.0;

  for (i = LO_CHAN; i <= HI_CHAN; i++) {

    tne += ch_noise [i];
    tce += ch_enrg [i];

  }


/* Calculate log spectral deviation */

  for (i = LO_CHAN; i <= HI_CHAN; i++)
    ch_enrg_db [i] = 10.*log10( ch_enrg [i] );

  if (first == TRUE)
    for (i = LO_CHAN; i <= HI_CHAN; i++)
      ch_enrg_long_db [i] = ch_enrg_db [i];

  ch_enrg_dev = 0.;
  for (i = LO_CHAN; i <= HI_CHAN; i++)
    ch_enrg_dev += fabs( ch_enrg_long_db [i] - ch_enrg_db [i] );


/* Calculate long term integration constant as a function of total channel energy (tce) */
/* (i.e., high tce (-40 dB) -> slow integration (alpha = 0.99),
          low tce (-60 dB) -> fast integration (alpha = 0.50) */

  alpha = HIGH_ALPHA - (ALPHA_RANGE / TCE_RANGE) * (HIGH_TCE_DB - 10.*log10(tce));
  if ( alpha > HIGH_ALPHA )
    alpha = HIGH_ALPHA;
  else if ( alpha < LOW_ALPHA )
    alpha = LOW_ALPHA;


/* Calc long term log spectral energy */

  for (i = LO_CHAN; i <= HI_CHAN; i++) {
    ch_enrg_long_db [i] = alpha*ch_enrg_long_db [i] + (1.-alpha)*ch_enrg_db [i];
  }


/* Set or reset the update flag */

  update_flag = FALSE;

  if (vm_sum <= UPDATE_THLD) {

    update_flag = TRUE;
    update_cnt = 0;

  }

  else if (tce > NOISE_FLOOR && ch_enrg_dev < DEV_THLD) {
    update_cnt++;
    if (update_cnt >= UPDATE_CNT_THLD)
      update_flag = TRUE;
  }

  if ( update_cnt == last_update_cnt )
    hyster_cnt++;
  else
    hyster_cnt = 0;
  last_update_cnt = update_cnt;

  if ( hyster_cnt > HYSTER_CNT_THLD )
    update_cnt = 0;


/* Set or reset modify flag */

  index_cnt = 0;

  for (i = MID_CHAN; i <= HI_CHAN; i++)
    if (ch_snr [i] >= INDEX_THLD)
      index_cnt++;

  modify_flag = (index_cnt < INDEX_CNT_THLD)? TRUE : FALSE;
 

/* Modify the SNR indices */

  if (modify_flag == TRUE)
    for (i = LO_CHAN; i <= HI_CHAN; i++)
      if ((vm_sum <= METRIC_THLD) || (ch_snr [i] <= SETBACK_THLD))
        ch_snr [i] = 1;


/* Compute the channel gains */

  ftmp1 = 10.0 * log10 ((double) (NOISE_FLOOR / tne));
  ftmp1 = max(ftmp1,MIN_GAIN);

  for (i = LO_CHAN; i <= HI_CHAN; i++) {

    if (ch_snr [i] <= SNR_THLD)
      ch_snr [i] = SNR_THLD;

    gain = (ch_snr [i] - SNR_THLD) * GAIN_SLOPE + ftmp1;

    gain = min(0.0,gain);

    ftmp2 = pow (10.0, (double) (gain / 20.0));

    j1 = ch_tbl [i][0], j2 = ch_tbl [i][1];

    for (j = j1; j <= j2; j++)
      ch_gain [j] = ftmp2; 

  } 


/* Update the channel noise estimates */

  if (update_flag == TRUE)
    for (i = LO_CHAN; i <= HI_CHAN; i++) {

      ch_noise [i] = (1.0 - CNE_SM_FAC) * ch_noise [i] +
                      CNE_SM_FAC * ch_enrg [i];

      if (ch_noise [i] < MIN_CHAN_ENRG) ch_noise [i] = MIN_CHAN_ENRG;

    }


/* Filter the input data in the frequency domain and perform IFFT */

  for (i = 0; i < FFT_LEN / 2; i++) {

    data_buffer [2*i] *= ch_gain [i];
    data_buffer [2*i+1] *= ch_gain [i];

  }

  r_fft (data_buffer, -1);


/* Overlap add the filtered data from previous block.
Save data from this block for the next. */ 

  for (i = 0; i < FFT_LEN - FRM_LEN; i++)
    data_buffer [i] += overlap [i];

  for (i = FRM_LEN; i < FFT_LEN; i++)
    overlap [i - FRM_LEN] = data_buffer [i];


/* Deemphasize the filtered speech and write it out to farray */


  *farray_ptr = data_buffer [0] + DE_EMP_FAC * de_emp_mem;

  for (i = 1; i < FRM_LEN; i++) 
    *(farray_ptr + i) = data_buffer [i] + DE_EMP_FAC * 
                      *(farray_ptr + i - 1); 

  de_emp_mem = *(farray_ptr + FRM_LEN - 1);


  first = FALSE;

}		/* end noise_suprs () */


/***************************************************************************/

void init_window (float *x, int n, float ovlap)
{
	int i;
	float arg;
	int n1;

	/* use smoothed trapezoidal window */

	n1 = ovlap * n;
	arg = 2.*atan(1.)/n1;
	for (i=0; i<n1; i++) {
		x[i] = pow( sin( (i+0.5)*arg ), 2. );
	}
	for (i=n1; i<n-n1; i++) {
		x[i] = 1.;
	}
	for (i=n-n1; i<n; i++) {
		x[i] = pow( sin( ((i-n)+0.5)*arg ), 2. );
	}

	return;

}	/* end of init_window() */

/***************************************************************************/

