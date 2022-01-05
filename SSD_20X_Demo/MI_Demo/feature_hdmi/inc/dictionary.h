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
#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	int			n ;		/** Number of entries in dictionary */
	int			size ;	/** Storage size */
	char 		**val ;	/** List of string values */
	char 		**key ;	/** List of string keys */
	unsigned	*hash ;	/** List of hash values for keys */
}dictionary ;

unsigned dictionary_hash(char * key);
dictionary * dictionary_new(int size);
void dictionary_del(dictionary * vd);
char * dictionary_get(dictionary * d, char * key, char * def);
int dictionary_set(dictionary * vd, char * key, char * val);
void dictionary_unset(dictionary * d, char * key);
void dictionary_dump(dictionary * d, FILE * out);
char * dictionary_getVal(dictionary * d,char * section,int i);
int dictionary_getNumberOfSection(dictionary * d,char * section);

#ifdef __cplusplus
}
#endif

#endif
