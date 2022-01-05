/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _STRLIB_H_
#define _STRLIB_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void strlwc(const char * s, char * result, unsigned int r_size);
void strupc(const char * s, char * result, unsigned int r_size);
void strcrop(const char * s, char * result, unsigned int r_size);
void strstrip(const char * s, char * result, unsigned int r_size);
void strstrip_for_getstring(const char * s, char * result, unsigned int r_size);
char * strskp(char * s);

#ifdef __cplusplus
}
#endif

#endif
