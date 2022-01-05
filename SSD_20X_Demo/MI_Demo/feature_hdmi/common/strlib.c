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
#include <string.h>
#include <ctype.h>
#include "strlib.h"

#define ASCIILINESZ 1024

void strlwc(const char * s, char * result, unsigned int r_size)
{
    int i = 0;

    if (s == NULL) return;

    if (NULL == result) return;

    if (0 == r_size) return;

    memset(result, 0, r_size);

    while(s[i] && (i <(int) (r_size-1)))
    {
        result[i] = (char)tolower((int)s[i]);
        i++ ;
    }
    result[r_size-1] = (char)0;
}

void strupc(const char * s, char * result, unsigned int r_size)
{
    int i = 0;

    if(s == NULL) return;

    if(NULL == result) return;

    if(0 == r_size) return;

    memset(result, 0, r_size);

    while(s[i] && (i < (int)(r_size-1)))
    {
        result[i] = (char)toupper((int)s[i]);
        i++ ;
    }
    result[r_size-1] = (char)0;
}

char * strskp(char * s)
{
    char * skip = s;
    if(s == NULL) return NULL ;
    while(isspace((int)*skip) && *skip) skip++;
    return skip ;
}

void strcrop(const char * s, char * result, unsigned int r_size)
{
    char * last = NULL;

    if (s == NULL) return;

    if (NULL == result) return;

    if (0 == r_size) return;

    memset(result, 0, r_size);
    strncpy(result, s, r_size-1);
    result[r_size-1] = '\0';

    last = result + strlen(result);
    while(last > result)
    {
        if(!isspace((int)*(last - 1)))
            break ;
        last -- ;
    }
    *last = (char)0;
}

void strstrip(const char * s, char * result, unsigned int r_size)
{
    char * last = NULL;

    if (s == NULL) return;

    if (NULL == result) return;

    if (0 == r_size) return;

    while(isspace((int)*s) && *s) s++;

    memset(result, 0, r_size);
    strncpy(result, s, r_size-1);
    result[r_size-1] = '\0';

    last = result + strlen(result);
    while(last > result)
    {
        if(!isspace((int)*(last - 1)))
            break ;
        last -- ;
    }
    *last = (char)0;
}

void strstrip_for_getstring(const char * s, char * result, unsigned int r_size)
{
    if (s == NULL) return;

    if (NULL == result) return;

    if (0 == r_size) return;

    char * last = NULL;

    while(isspace((int)*s) && *s) s++;

    memset(result, 0, r_size);
    strncpy(result, s, r_size-1);
    result[r_size-1] = '\0';

    last = result + strlen(result);
    while(last > result)
    {
        if(!isspace((int)*(last - 1)))
            break ;
        last -- ;
    }
    *last = (char)0;


    //check ':' nearby blank & take them off
    char * pCheck = result;
    char * pBlankFound;
    char * substr = NULL;
    char l_2[ASCIILINESZ+1] = {0};
    char l_temp[ASCIILINESZ+1] = {0};

    memset(l_2, 0, sizeof(l_2));
    memset(l_temp, 0, sizeof(l_temp));

    //check if have blank in string
    while(!isspace((int)*pCheck) && *pCheck) pCheck++;
    if (pCheck == last)
        return;

    //find ':'
    pCheck = result;
    do{
        if (*pCheck == ':') break;  //find first ':', we don't handle secondary ':'
        pCheck++;
    }while( pCheck != last);

    if (pCheck == last)
        return;   //can't find ':'

    //blank before ':'
    if ( result != pCheck)
    {
        pBlankFound = pCheck - 1;
        while(isspace((int)*pBlankFound))
        {
            *pBlankFound = 0;
            pBlankFound--;
        }
        strcpy(l_2, result);
    }

    substr = l_2;
    snprintf(l_temp, sizeof(l_temp), "%s:", substr);

    //blank after ':'
    pBlankFound = pCheck + 1;
    while(isspace((int)*pBlankFound) && *pBlankFound)
    {
        pBlankFound++;
    }
    memset(l_2, 0, sizeof(l_2));
    snprintf(l_2, sizeof(l_2), "%s%s", l_temp, pBlankFound);

    strncpy(result, l_2, r_size);
}

