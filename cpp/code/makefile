# ======================================================================*/
#      Enhanced Variable Rate Codec - Master C code Specification       */
#      Copyright (C) 1997-1998 Telecommunications Industry Association. */
#      All rights reserved.                                             */
# ----------------------------------------------------------------------*/
#  Note:  Reproduction and use of this software for the design and      */
#      development of North American Wideband CDMA Digital              */
#      Cellular Telephony Standards is authorized by the TIA.           */
#      The TIA does not authorize the use of this software for any      */
#      other purpose.                                                   */
#                                                                       */
#      The availability of this software does not provide any license   */
#      by implication, estoppel, or otherwise under any patent rights   */
#      of TIA member companies or others covering any use of the        */
#      contents herein.                                                 */
#                                                                       */
#      Any copies of this software or derivative works must include     */
#      this and all other proprietary notices.                          */
# ======================================================================*/
TARGET=../bin/EvrcFlt
FXPTLIBS=../dsp_fx
TTYDIR=../ttylib
CC=gcc
#CCFLAGS=-O3 -Wall -ansi
CCFLAGS=-Wall 
CCINCDIR=-I../include -I$(TTYDIR) -I$(FXPTLIBS)
CCLIBDIR=-L$(TTYDIR) -L$(FXPTLIBS)
CCLIBS=-ltty -ldsp40 -lm
CCOBJS=\
	a2lsp.o\
	acb_ex.o\
	acelp_co.o\
	apf.o\
	auto.o\
	bitpack.o\
	bitupack.o\
	bl_intrp.o\
	bqiir.o\
	cod3_10.o\
	cod8_35.o\
	comacb.o\
	convh.o\
	convolve.o\
	corr_xh.o\
	cshift.o\
	d_globs.o\
	decode.o\
	dec3_10.o\
	dec8_35.o\
	durbin.o\
	e_globs.o\
	encode.o\
	fer.o\
	fer_sim.o\
	filter.o\
	fndppf.o\
	getext1k.o\
	getgain.o\
	getres.o\
	globs.o\
	impulser.o\
	interpol.o\
	lpcana.o\
	lsp2a.o\
	lspmaq.o\
	main.o\
	maxeloc.o\
	mdfyorig.o\
	mod.o\
	ns127.o\
	pit_shrp.o\
	pktoav.o\
	putacbc.o\
	rda.o\
	r_fft.o\
	rom.o\
	synfltr.o\
	weight.o\
	w2res.o\
	zeroinpt.o

.c.o:
	${CC} ${CCFLAGS} ${CCINCDIR} -c $*.c


${TARGET}: ${CCOBJS} $(TTYDIR)/libtty.a $(FXPTLIBS)/libdsp40.a
	${CC} -o ${TARGET} ${CCFLAGS} $(CCLIBDIR) ${CCOBJS} ${CCLIBS}

clean:
	rm -f *.o
clobber:
	rm -f *.o core ${TARGET} ${TARGET}.exe

main.o decode.o: $(TTYDIR)/tty_dbg.h
main.o: $(TTYDIR)/tty.h
encode.o: $(TTYDIR)/tty.h
decode.o: $(TTYDIR)/tty.h
getgain.o: $(TTYDIR)/tty.h
cshift.o: ../include/ansi.h
d_globs.o: ../include/ansi.h
decode.o: ../include/ansi.h
fer.o: ../include/ansi.h
rda.o: ../include/ansi.h
