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
#ifndef _INIPARSER_H_
#define _INIPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "strlib.h"
#include "dictionary.h"

#ifdef __cplusplus
extern "C" {
#endif

int iniparser_getnsec(dictionary * d);
char * iniparser_getsecname(dictionary * d, int n);
void iniparser_dump_ini(dictionary * d, FILE * f);
void iniparser_dump(dictionary * d, FILE * f);
char * iniparser_getstring(dictionary * d, const char * key, char * def);
int iniparser_getint(dictionary * d, const char * key, int notfound);
unsigned int iniparser_getunsignedint(dictionary * d, const char * key, unsigned int notfound);
double iniparser_getdouble(dictionary * d, char * key, double notfound);
int iniparser_getboolean(dictionary * d, const char * key, int notfound);
int iniparser_setstring(dictionary * ini, const char * entry, char * val);
void iniparser_unset(dictionary * ini, char * entry);
int iniparser_find_entry(dictionary * ini, char * entry) ;
dictionary * iniparser_load(const char * ininame);
void iniparser_freedict(dictionary * d);
char * iniparser_getVal(dictionary * d,char * section,int i);
int iniparser_getNumberOfSection(dictionary * d,char * section);
const char ** ParserStrToArray(const char *inputStr, int *count);

#ifdef __cplusplus
}
#endif
#endif
