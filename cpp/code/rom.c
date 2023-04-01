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
/*  Module:     rom.c                                                   */
/*----------------------------------------------------------------------*/
/*  Memory Usage:														*/
/*		ROM: 				6150										*/
/*		Static/Global RAM: 	0											*/
/*		Stack/Local RAM: 	0											*/
/*----------------------------------------------------------------------*/
/*======================================================================*/
/*         ..Includes.                                                  */
/*----------------------------------------------------------------------*/
#include  "macro.h"

/*======================================================================*/
/*         ..Globals.                                                   */
/*----------------------------------------------------------------------*/

/* ROM tables */
long    Base18[SubFrameSize / SPACING] =
{1, 18, 324, 5832, 104976, 1889568};

/* Quantization table for pitch gain */
float   ppvq[ACBGainSize] =
{0.0, 0.3, 0.55, 0.7, 0.8, 0.9, 1.0, 1.2};
float   gnvq_8[maxFCBGainSize];	/* Quantization table for fcb gain */
float   gnvq_4[maxFCBGainSize];	/* Quantization table for fcb gain */

short   nsize8[2] =
{16, 16};
short   lognsize8[2] =
{4, 4};				/* c.b. size of each sub-matrix   */
short   nsub8[2] =
{5, 5};				/* Vector size of each sub-matrix */


float   rnd_delay[NoOfSubFrames + 2] =
{55.0, 80.0, 39.0, 71.0, 33.0};

/* New AT&T half-rate quantizer */
short nsize22[3] = {128,128,256};
short nsub22[3] = {3,3,4};
short lognsize22[3] = {7,7,8};

/* New AT&T full-rate quantizer */
short nsize28[4] = {64,64,512,128};
short lognsize28[4] = {6,6,9,7};
short nsub28[4] = {2,2,3,3};

/* LSP quantizer tables (AT&T quantizer) */
#include "lsptab.dat"

float   Logqtbl[256 * 3] =
{
	-0.2464E-01,
	-0.4005E-02,
	-0.1107E+00,
	0.8734E+00,
	0.1004E+01,
	0.9930E+00,
	0.4222E+00,
	0.3894E+00,
	0.5020E+00,
	0.1450E+01,
	0.1328E+01,
	0.1278E+01,
	0.1957E+00,
	0.2169E+00,
	0.2735E+00,
	0.1142E+01,
	0.1240E+01,
	0.1157E+01,
	0.7881E+00,
	0.6778E+00,
	0.4185E+00,
	0.1504E+01,
	0.1468E+01,
	0.1534E+01,
	0.3173E+00,
	0.2693E+00,
	-0.9526E-01,
	0.1141E+01,
	0.1154E+01,
	0.1044E+01,
	0.5147E+00,
	0.5784E+00,
	0.8802E+00,
	0.1502E+01,
	0.1407E+01,
	0.1409E+01,
	0.3163E+00,
	0.3592E+00,
	0.2830E+00,
	0.1217E+01,
	0.1213E+01,
	0.1216E+01,
	0.1023E+01,
	0.1139E+01,
	-0.9526E-01,
	0.1619E+01,
	0.1655E+01,
	0.1642E+01,
	0.1437E+00,
	0.1505E+00,
	0.6838E-01,
	0.9794E+00,
	0.1021E+01,
	0.1117E+01,
	0.4701E+00,
	0.6426E+00,
	0.5519E+00,
	0.1366E+01,
	0.1397E+01,
	0.1406E+01,
	0.2918E+00,
	0.3022E+00,
	0.2420E+00,
	0.1309E+01,
	0.1241E+01,
	0.1220E+01,
	0.7989E+00,
	0.7654E+00,
	0.7391E+00,
	0.1612E+01,
	0.1502E+01,
	0.1447E+01,
	0.2594E+00,
	0.1948E+00,
	0.2555E+00,
	0.1091E+01,
	0.1150E+01,
	0.1272E+01,
	0.3423E+00,
	0.4150E+00,
	0.1294E+01,
	0.1729E+01,
	0.1377E+01,
	0.1065E+01,
	0.4103E+00,
	0.3287E+00,
	0.3228E+00,
	0.1144E+01,
	0.1281E+01,
	0.1416E+01,
	0.1047E+01,
	0.1117E+01,
	0.6188E+00,
	0.1914E+01,
	0.1777E+01,
	0.1516E+01,
	-0.2117E-01,
	0.2159E+00,
	0.2351E+00,
	0.1093E+01,
	0.1088E+01,
	0.1026E+01,
	0.5567E+00,
	0.5092E+00,
	0.4654E+00,
	0.1510E+01,
	0.1449E+01,
	0.1201E+01,
	0.2362E+00,
	0.3426E+00,
	0.2549E+00,
	0.1340E+01,
	0.1225E+01,
	0.1117E+01,
	0.1203E+01,
	0.3819E+00,
	0.2269E+00,
	0.1373E+01,
	0.1404E+01,
	0.1830E+01,
	0.2570E+00,
	0.2668E+00,
	0.1636E+00,
	0.1219E+01,
	0.1098E+01,
	0.1122E+01,
	0.6985E+00,
	0.8456E+00,
	0.1069E+01,
	0.1550E+01,
	0.1501E+01,
	0.1388E+01,
	0.2870E+00,
	0.3060E+00,
	0.3599E+00,
	0.1178E+01,
	0.1345E+01,
	0.1302E+01,
	0.1270E+01,
	0.1215E+01,
	0.1812E+00,
	0.1725E+01,
	0.1777E+01,
	0.1693E+01,
	0.2074E+00,
	0.2104E+00,
	0.1539E+00,
	0.1105E+01,
	0.1034E+01,
	0.1104E+01,
	0.6683E+00,
	0.6646E+00,
	0.6639E+00,
	0.1403E+01,
	0.1462E+01,
	0.1435E+01,
	0.3389E+00,
	0.3754E+00,
	0.2150E+00,
	0.1288E+01,
	0.1325E+01,
	0.1257E+01,
	0.8933E+00,
	0.8253E+00,
	0.8133E+00,
	0.1555E+01,
	0.1579E+01,
	0.1565E+01,
	0.3264E+00,
	0.2434E+00,
	0.2852E+00,
	0.1242E+01,
	0.1180E+01,
	0.1202E+01,
	0.1314E+00,
	0.1698E+00,
	0.1646E+01,
	0.1797E+01,
	0.1597E+01,
	0.1241E+01,
	0.4721E+00,
	0.5346E+00,
	0.3066E+00,
	0.1274E+01,
	0.1401E+01,
	0.1351E+01,
	0.1455E+01,
	0.1386E+01,
	0.6430E+00,
	0.1828E+01,
	0.1867E+01,
	0.1825E+01,
	-0.3265E+00,
	-0.2956E+00,
	-0.2462E+00,
	0.1035E+01,
	0.1020E+01,
	0.1003E+01,
	0.3702E+00,
	0.4307E+00,
	0.7072E+00,
	0.1424E+01,
	0.1345E+01,
	0.1352E+01,
	0.2267E+00,
	0.2680E+00,
	0.3037E+00,
	0.1235E+01,
	0.1249E+01,
	0.1146E+01,
	0.9944E+00,
	0.6485E+00,
	0.5248E+00,
	0.1539E+01,
	0.1492E+01,
	0.1612E+01,
	0.3815E+00,
	0.3360E+00,
	-0.9526E-01,
	0.1163E+01,
	0.1144E+01,
	0.1117E+01,
	0.6734E+00,
	0.7656E+00,
	0.1014E+01,
	0.1568E+01,
	0.1438E+01,
	0.1455E+01,
	0.3409E+00,
	0.3317E+00,
	0.3856E+00,
	0.1180E+01,
	0.1284E+01,
	0.1284E+01,
	0.1244E+01,
	0.1214E+01,
	-0.9526E-01,
	0.1753E+01,
	0.1598E+01,
	0.1744E+01,
	0.1548E+00,
	0.1388E+00,
	0.2020E+00,
	0.1027E+01,
	0.1133E+01,
	0.1093E+01,
	0.3906E+00,
	0.7505E+00,
	0.5705E+00,
	0.1420E+01,
	0.1357E+01,
	0.1543E+01,
	0.3252E+00,
	0.3136E+00,
	0.2804E+00,
	0.1351E+01,
	0.1309E+01,
	0.1224E+01,
	0.8781E+00,
	0.8095E+00,
	0.7109E+00,
	0.1614E+01,
	0.1580E+01,
	0.1433E+01,
	0.3222E+00,
	0.2298E+00,
	0.2157E+00,
	0.1216E+01,
	0.1077E+01,
	0.1247E+01,
	0.1363E+01,
	0.1280E+01,
	0.1317E+01,
	0.1751E+01,
	0.1457E+01,
	0.1182E+01,
	0.4428E+00,
	0.4082E+00,
	0.3181E+00,
	0.1157E+01,
	0.1227E+01,
	0.1604E+01,
	0.1286E+01,
	0.1268E+01,
	0.8167E+00,
	0.1994E+01,
	0.2018E+01,
	0.1307E+01,
	0.2671E-01,
	0.2594E+00,
	0.3397E+00,
	0.1164E+01,
	0.1080E+01,
	0.9321E+00,
	0.5998E+00,
	0.6076E+00,
	0.5081E+00,
	0.1442E+01,
	0.1442E+01,
	0.1375E+01,
	0.2390E+00,
	0.3554E+00,
	0.3426E+00,
	0.1287E+01,
	0.1307E+01,
	0.1144E+01,
	0.1200E+01,
	0.7495E+00,
	0.3967E+00,
	0.1561E+01,
	0.1517E+01,
	0.1898E+01,
	0.3598E+00,
	0.3463E+00,
	0.1200E+00,
	0.1298E+01,
	0.1125E+01,
	0.1062E+01,
	0.7577E+00,
	0.1013E+01,
	0.1194E+01,
	0.1537E+01,
	0.1513E+01,
	0.1464E+01,
	0.4041E+00,
	0.4038E+00,
	0.3897E+00,
	0.1293E+01,
	0.1219E+01,
	0.1378E+01,
	0.1250E+01,
	0.1391E+01,
	0.2451E+00,
	0.1558E+01,
	0.1764E+01,
	0.1728E+01,
	0.2700E+00,
	0.1894E+00,
	0.1924E+00,
	0.1111E+01,
	0.1112E+01,
	0.1173E+01,
	0.7579E+00,
	0.8342E+00,
	0.4781E+00,
	0.1464E+01,
	0.1477E+01,
	0.1469E+01,
	0.4001E+00,
	0.3104E+00,
	0.2217E+00,
	0.1346E+01,
	0.1421E+01,
	0.1312E+01,
	0.1071E+01,
	0.8967E+00,
	0.7511E+00,
	0.1616E+01,
	0.1551E+01,
	0.1574E+01,
	0.3329E+00,
	0.2785E+00,
	0.3140E+00,
	0.1281E+01,
	0.1209E+01,
	0.1239E+01,
	0.2805E+00,
	0.2687E+00,
	0.1646E+01,
	0.1814E+01,
	0.1514E+01,
	0.1510E+01,
	0.6231E+00,
	0.4200E+00,
	0.3701E+00,
	0.1255E+01,
	0.1429E+01,
	0.1454E+01,
	0.1642E+01,
	0.1581E+01,
	0.7112E+00,
	0.1844E+01,
	0.1963E+01,
	0.1895E+01,
	-0.4208E-01,
	-0.1491E+00,
	-0.7639E-01,
	0.1046E+01,
	0.9598E+00,
	0.9176E+00,
	0.4478E+00,
	0.4605E+00,
	0.5111E+00,
	0.1521E+01,
	0.1292E+01,
	0.1342E+01,
	0.2220E+00,
	0.2549E+00,
	0.2510E+00,
	0.1186E+01,
	0.1254E+01,
	0.1171E+01,
	0.8999E+00,
	0.4960E+00,
	0.4943E+00,
	0.1423E+01,
	0.1484E+01,
	0.1620E+01,
	0.2796E+00,
	0.2778E+00,
	-0.2820E+00,
	0.1170E+01,
	0.1181E+01,
	0.1076E+01,
	0.4068E+00,
	0.8541E+00,
	0.9352E+00,
	0.1584E+01,
	0.1416E+01,
	0.1387E+01,
	0.3325E+00,
	0.3655E+00,
	0.3340E+00,
	0.1224E+01,
	0.1257E+01,
	0.1245E+01,
	0.1061E+01,
	0.1138E+01,
	-0.9526E-01,
	0.1681E+01,
	0.1704E+01,
	0.1673E+01,
	0.1932E+00,
	0.1489E+00,
	0.1258E+00,
	0.1023E+01,
	0.1088E+01,
	0.1145E+01,
	0.5190E+00,
	0.6873E+00,
	0.5172E+00,
	0.1380E+01,
	0.1405E+01,
	0.1474E+01,
	0.3393E+00,
	0.3100E+00,
	0.2231E+00,
	0.1354E+01,
	0.1249E+01,
	0.1270E+01,
	0.7363E+00,
	0.8508E+00,
	0.8247E+00,
	0.1612E+01,
	0.1537E+01,
	0.1509E+01,
	0.2952E+00,
	0.2053E+00,
	0.2590E+00,
	0.1138E+01,
	0.1219E+01,
	0.1262E+01,
	0.1345E+01,
	0.1289E+01,
	0.1338E+01,
	0.1437E+01,
	0.1360E+01,
	0.1442E+01,
	0.4826E+00,
	0.3298E+00,
	0.3842E+00,
	0.1219E+01,
	0.1311E+01,
	0.1413E+01,
	0.1212E+01,
	0.1186E+01,
	0.6357E+00,
	0.1873E+01,
	0.1939E+01,
	0.1674E+01,
	0.1260E+01,
	0.1306E+01,
	0.1368E+01,
	0.1146E+01,
	0.1077E+01,
	0.1025E+01,
	0.6029E+00,
	0.5039E+00,
	0.5781E+00,
	0.1514E+01,
	0.1420E+01,
	0.1324E+01,
	0.2652E+00,
	0.3192E+00,
	0.3042E+00,
	0.1368E+01,
	0.1198E+01,
	0.1200E+01,
	0.1234E+01,
	0.4910E+00,
	0.3464E-01,
	0.1347E+01,
	0.1560E+01,
	0.1861E+01,
	0.2766E+00,
	0.2887E+00,
	0.2029E+00,
	0.1257E+01,
	0.1105E+01,
	0.1145E+01,
	0.1351E+01,
	0.1353E+01,
	0.1406E+01,
	0.1506E+01,
	0.1580E+01,
	0.1362E+01,
	0.2794E+00,
	0.3868E+00,
	0.4277E+00,
	0.1234E+01,
	0.1334E+01,
	0.1336E+01,
	0.1280E+01,
	0.1252E+01,
	0.1805E+00,
	0.1387E+01,
	0.1396E+01,
	0.1434E+01,
	0.2902E+00,
	0.1170E+00,
	0.1698E+00,
	0.1134E+01,
	0.1077E+01,
	0.1117E+01,
	0.6986E+00,
	0.7177E+00,
	0.7366E+00,
	0.1370E+01,
	0.1491E+01,
	0.1495E+01,
	0.4031E+00,
	0.5144E+00,
	0.1751E+00,
	0.1333E+01,
	0.1377E+01,
	0.1257E+01,
	0.9212E+00,
	0.8934E+00,
	0.8897E+00,
	0.1589E+01,
	0.1614E+01,
	0.1523E+01,
	0.3152E+00,
	0.2164E+00,
	0.3230E+00,
	0.1300E+01,
	0.1145E+01,
	0.1212E+01,
	0.1269E+01,
	0.1245E+01,
	0.1497E+01,
	0.1763E+01,
	0.1716E+01,
	0.1311E+01,
	0.4702E+00,
	0.5422E+00,
	0.4306E+00,
	0.1342E+01,
	0.1433E+01,
	0.1423E+01,
	0.1472E+01,
	0.1404E+01,
	0.8371E+00,
	0.1936E+01,
	0.1883E+01,
	0.1838E+01,
	0.1266E+01,
	0.1295E+01,
	0.1302E+01,
	0.1074E+01,
	0.1002E+01,
	0.1023E+01,
	0.5206E+00,
	0.4045E+00,
	0.6549E+00,
	0.1457E+01,
	0.1378E+01,
	0.1363E+01,
	0.2715E+00,
	0.2629E+00,
	0.2841E+00,
	0.1264E+01,
	0.1271E+01,
	0.1175E+01,
	0.1337E+01,
	0.1305E+01,
	0.1306E+01,
	0.1555E+01,
	0.1571E+01,
	0.1657E+01,
	0.3341E+00,
	0.4147E+00,
	-0.3648E+00,
	0.1188E+01,
	0.1185E+01,
	0.1161E+01,
	0.6198E+00,
	0.7208E+00,
	0.1157E+01,
	0.1582E+01,
	0.1465E+01,
	0.1513E+01,
	0.3839E+00,
	0.3651E+00,
	0.3814E+00,
	0.1214E+01,
	0.1256E+01,
	0.1292E+01,
	0.1361E+01,
	0.1363E+01,
	0.1312E+01,
	0.1793E+01,
	0.1693E+01,
	0.1669E+01,
	0.1889E+00,
	0.1275E+00,
	0.2534E+00,
	0.1066E+01,
	0.1174E+01,
	0.1133E+01,
	0.4999E+00,
	0.8207E+00,
	0.5813E+00,
	0.1478E+01,
	0.1416E+01,
	0.1497E+01,
	0.3814E+00,
	0.3138E+00,
	0.2889E+00,
	0.1396E+01,
	0.1265E+01,
	0.1233E+01,
	0.9458E+00,
	0.9161E+00,
	0.5875E+00,
	0.1672E+01,
	0.1632E+01,
	0.1553E+01,
	0.3505E+00,
	0.2525E+00,
	0.2364E+00,
	0.1211E+01,
	0.1138E+01,
	0.1235E+01,
	0.1391E+01,
	0.1231E+01,
	0.1355E+01,
	0.1783E+01,
	0.1510E+01,
	0.1199E+01,
	0.4227E+00,
	0.4548E+00,
	0.3671E+00,
	0.1281E+01,
	0.1254E+01,
	0.1661E+01,
	0.1338E+01,
	0.1379E+01,
	0.9531E+00,
	0.2148E+01,
	0.1965E+01,
	0.1584E+01,
	0.9324E-01,
	0.3575E+00,
	0.3522E+00,
	0.1212E+01,
	0.1086E+01,
	0.1044E+01,
	0.6128E+00,
	0.6136E+00,
	0.6060E+00,
	0.1484E+01,
	0.1507E+01,
	0.1396E+01,
	0.2820E+00,
	0.3848E+00,
	0.3156E+00,
	0.1368E+01,
	0.1287E+01,
	0.1128E+01,
	0.1369E+01,
	0.1352E+01,
	0.1358E+01,
	0.1381E+01,
	0.1765E+01,
	0.2113E+01,
	0.1314E+01,
	0.1345E+01,
	0.1334E+01,
	0.1290E+01,
	0.1172E+01,
	0.1119E+01,
	0.1304E+01,
	0.1377E+01,
	0.1427E+01,
	0.1490E+01,
	0.1540E+01,
	0.1536E+01,
	0.3994E+00,
	0.4402E+00,
	0.4173E+00,
	0.1323E+01,
	0.1307E+01,
	0.1392E+01,
	0.1400E+01,
	0.1388E+01,
	0.1369E+01,
	0.1669E+01,
	0.1818E+01,
	0.1834E+01,
	0.2742E+00,
	0.2235E+00,
	0.1986E+00,
	0.1137E+01,
	0.1139E+01,
	0.1201E+01,
	0.1324E+01,
	0.1385E+01,
	0.1349E+01,
	0.1455E+01,
	0.1574E+01,
	0.1454E+01,
	0.5019E+00,
	0.3255E+00,
	0.2555E+00,
	0.1388E+01,
	0.1438E+01,
	0.1300E+01,
	0.1394E+01,
	0.1349E+01,
	0.1411E+01,
	0.1639E+01,
	0.1580E+01,
	0.1681E+01,
	0.3920E+00,
	0.2498E+00,
	0.3523E+00,
	0.1301E+01,
	0.1221E+01,
	0.1285E+01,
	0.1318E+01,
	0.1342E+01,
	0.1494E+01,
	0.1910E+01,
	0.1680E+01,
	0.1470E+01,
	0.6082E+00,
	0.5270E+00,
	0.4173E+00,
	0.1255E+01,
	0.1477E+01,
	0.1503E+01,
	0.1807E+01,
	0.1742E+01,
	0.6553E+00,
	0.2000E+01,
	0.2072E+01,
	0.2051E+01
};