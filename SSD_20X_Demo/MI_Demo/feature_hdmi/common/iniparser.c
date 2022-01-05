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
#include <assert.h>
#include "strlib.h"
#include "iniparser.h"

#define ASCIILINESZ         (9000)
#define INI_INVALID_KEY     ((char*)-1)

typedef enum
{
    LINE_UNPROCESSED,
    LINE_ERROR,
    LINE_EMPTY,
    LINE_COMMENT,
    LINE_SECTION,
    LINE_VALUE
}line_status ;

int iniparser_getnsec(dictionary * d)
{
    int i ;
    int nsec ;

    if(d == NULL) return -1 ;
    nsec = 0 ;
    for(i = 0 ; i < d->size ; i++)
    {
        if(d->key[i] == NULL)
            continue ;
        if(strchr(d->key[i], ':') == NULL)
        {
            nsec ++ ;
        }
    }
    return nsec ;
}

char * iniparser_getsecname(dictionary * d, int n)
{
    int i ;
    int foundsec ;

    if((d == NULL) || (n < 0)) return NULL ;
    foundsec = 0 ;
    for(i = 0 ; i < d->size ; i++)
    {
        if(d->key[i] == NULL)
            continue ;
        if(strchr(d->key[i], ':') == NULL)
        {
            foundsec++ ;
            if(foundsec > n)
                break ;
        }
    }
    if(foundsec <= n)
    {
        return NULL ;
    }
    return d->key[i] ;
}

void iniparser_dump(dictionary * d, FILE * f)
{
    int     i ;

    if((d == NULL) || (f == NULL)) return ;
    for(i = 0 ; i < d->size ; i++)
    {
        if(d->key[i] == NULL)
            continue ;
        if(d->val[i] != NULL)
        {
            fprintf(f, "[%s]=[%s]\n", d->key[i], d->val[i]);
        }
        else
        {
            fprintf(f, "[%s]=UNDEF\n", d->key[i]);
        }
    }
    int fno = fileno(f);
    fsync(fno);
    return;
}

void iniparser_dump_ini(dictionary * d, FILE * f)
{
    int     i, j ;
    char    keym[ASCIILINESZ+1];
    int     nsec ;
    char *  secname ;
    int     seclen ;
    int     fno ;
    if((d == NULL) || (f == NULL)) return ;

    fno  = fileno(f);
    nsec = iniparser_getnsec(d);
    if(nsec < 1)
    {
        /* No section in file: dump all keys as they are */
        for(i = 0 ; i < d->size ; i++)
        {
            if(d->key[i] == NULL)
                continue ;
            fprintf(f, "%s = %s\n", d->key[i], d->val[i]);
        }
        fsync(fno);
        return ;
    }
    for(i = 0 ; i < nsec ; i++)
    {
        secname = iniparser_getsecname(d, i) ;
        assert(secname);

        seclen  = (int)strlen(secname);
        fprintf(f, "\n[%s]\n", secname);
        //sprintf(keym, "%s:", secname);
        snprintf(keym, ASCIILINESZ, "%s:", secname);
        for(j = 0 ; j < d->size ; j++)
        {
            if(d->key[j] == NULL)
                continue ;
            if(!strncmp(d->key[j], keym, seclen + 1))
            {
                fprintf(f,
                        "%-30s = %s\n",
                        d->key[j] + seclen + 1,
                        d->val[j] ? d->val[j] : "");
            }
        }
    }
    fprintf(f, "\n");
    fsync(fno);
    return ;
}

char * iniparser_getstring(dictionary * d, const char * key, char * def)
{
    char lc_key[ASCIILINESZ+1] = {0};
    char * sval = NULL;
    char non_blank_key[ASCIILINESZ+1] = {0};

    if((d == NULL) || (key == NULL))
        return def ;

    //Remove blanks at the beginning and the end of a string.
    strstrip_for_getstring(key, non_blank_key, ASCIILINESZ+1);

    strlwc(non_blank_key, lc_key, ASCIILINESZ+1);
    sval = dictionary_get(d, lc_key, def);
    return sval ;
}

int iniparser_getint(dictionary * d, const char * key, int notfound)
{
    char    *   str ;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if(str == INI_INVALID_KEY) return notfound ;
    return (int)strtol(str, NULL, 0);
}

unsigned int iniparser_getunsignedint(dictionary * d, const char * key, unsigned int notfound)
{
    char    *   str ;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if(str == INI_INVALID_KEY) return notfound ;
    return (unsigned int)strtoul(str, NULL, 0);
}

double iniparser_getdouble(dictionary * d, char * key, double notfound)
{
    char    *   str ;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if(str == INI_INVALID_KEY) return notfound ;
    return atof(str);
}

int iniparser_getboolean(dictionary * d, const char * key, int notfound)
{
    char    *   c ;
    int         ret ;

    c = iniparser_getstring(d, key, INI_INVALID_KEY);
    if(c == INI_INVALID_KEY) return notfound ;
    if((c[0] == 'y') || (c[0] == 'Y') || (c[0] == '1') || (c[0] == 't') || (c[0] == 'T'))
    {
        ret = 1 ;
    }
    else if((c[0] == 'n') || (c[0] == 'N') || (c[0] == '0') || (c[0] == 'f') || (c[0] == 'F'))
    {
        ret = 0 ;
    }
    else
    {
        ret = notfound ;
    }
    return ret;
}

int iniparser_find_entry(
    dictionary  *   ini,
    char        *   entry
)
{
    int found = 0 ;
    if(iniparser_getstring(ini, entry, INI_INVALID_KEY) != INI_INVALID_KEY)
    {
        found = 1 ;
    }
    return found ;
}

int iniparser_setstring(dictionary * ini, const char * entry, char * val)
{
    char lwc_key[ASCIILINESZ+1] = {0};
    strlwc(entry, lwc_key, ASCIILINESZ+1);
    return dictionary_set(ini, lwc_key, val);
}

void iniparser_unset(dictionary * ini, char * entry)
{
    char lwc_key[ASCIILINESZ+1] = {0};
    strlwc(entry, lwc_key, ASCIILINESZ+1);
    dictionary_unset(ini, lwc_key);
}

static line_status iniparser_line(
    char * input_line,
    char * section,
    char * key,
    char * value)
{
    line_status sta ;
    char        line[ASCIILINESZ+1];
    int         len ;

    if(input_line!=NULL)
    {
        char strip_line[ASCIILINESZ+1] = {0};
        strstrip(input_line, strip_line, ASCIILINESZ+1);
        strncpy(line, strip_line, ASCIILINESZ);
    }
    line[ASCIILINESZ] = '\0';

    len = (int)strlen(line);

    sta = LINE_UNPROCESSED ;
    if(len < 1)
    {
        /* Empty line */
        sta = LINE_EMPTY ;
    }
    else if(line[0] == '#')
    {
        /* Comment line */
        sta = LINE_COMMENT ;
    }
    else if((line[0] == '[') && (line[len-1] == ']'))
    {
        /* Section name */
        sscanf(line, "[%[^]]", section);
        if(section!=NULL)
        {
            char strip_section[ASCIILINESZ+1] = {0};
            char lwc_section[ASCIILINESZ+1] = {0};
            strstrip(section, strip_section, ASCIILINESZ+1);
            strlwc(strip_section, lwc_section, ASCIILINESZ+1);
            strncpy(section, lwc_section, ASCIILINESZ);
        }
        sta = LINE_SECTION ;
    }
    else if((sscanf(line, "%[^=] = \"%[^\"]\"", key, value) == 2)
            ||  (sscanf(line, "%[^=] = '%[^\']'",   key, value) == 2)
            ||  (sscanf(line, "%[^=] = %[^;#]",     key, value) == 2))
    {
        /* Usual key=value, with or without comments */
        if(key!=NULL)
        {
            char strip_key[ASCIILINESZ+1] = {0};
            char lwc_key[ASCIILINESZ+1] = {0};
            strstrip(key, strip_key, ASCIILINESZ+1);
            strlwc(strip_key, lwc_key, ASCIILINESZ+1);
            strncpy(key, lwc_key, ASCIILINESZ);
        }
        if(value!=NULL)
        {
            char strip_value[ASCIILINESZ+1] = {0};
            strstrip(value, strip_value, ASCIILINESZ+1);
            strncpy(value, strip_value, ASCIILINESZ);
        }
        /*
         * sscanf cannot handle '' or "" as empty values
         * this is done here
         */
        if(value!=NULL && (!strcmp(value, "\"\"") || (!strcmp(value, "''"))))
        {
            value[0] = 0 ;
        }
        sta = LINE_VALUE ;
    }
    else if((sscanf(line, "%[^=] = %[;#]", key, value) == 2)
            ||  (sscanf(line, "%[^=] %[=]", key, value) == 2))
    {
        /*
         * Special cases:
         * key=
         * key=;
         * key=#
         */
        if(key!=NULL)
        {
            char strip_key[ASCIILINESZ+1] = {0};
            char lwc_key[ASCIILINESZ+1] = {0};
            strstrip(key, strip_key, ASCIILINESZ+1);
            strlwc(strip_key, lwc_key, ASCIILINESZ+1);
            strncpy(key, lwc_key, ASCIILINESZ);
        }
        value[0] = 0 ;
        sta = LINE_VALUE ;
    }
    else
    {
        /* Generate syntax error */
        sta = LINE_ERROR ;
    }
    return sta ;
}

dictionary * iniparser_load(const char * ininame)
{
    FILE * in ;

    char line    [ASCIILINESZ+1] ;
    char section [ASCIILINESZ+1] ;
    char key     [ASCIILINESZ+1] ;
    char tmp     [ASCIILINESZ+1] ;
    char val     [ASCIILINESZ+1] ;
    char *substr1 = NULL;
    char *substr2 = NULL;

    int  last = 0 ;
    int  len ;
    int  lineno = 0 ;
    int  errs = 0;

    dictionary * dict ;

    if(ininame == NULL)
    {
        return NULL;
    }

    in = fopen(ininame, "r");
    if(in == NULL)
    {
        fprintf(stderr, "iniparser: cannot open %s\n", ininame);
        return NULL ;
    }
    dict = dictionary_new(0) ;
    if(!dict)
    {
        fclose(in);
        return NULL ;
    }

    memset(line,    0, sizeof(line));
    memset(section, 0, sizeof(section));
    memset(key,     0, sizeof(key));
    memset(tmp,     0, sizeof(tmp));
    memset(val,     0, sizeof(val));
    last = 0 ;

    while(fgets(line + last, ASCIILINESZ+1-last, in) != NULL)
    {
        int end = 0;
        lineno++;
        len = (int)strlen(line);
        end = len-1;
        /* Safety check against buffer overflows */
        if ((len == ASCIILINESZ) && (line[end]!='\n'))
        {
            fprintf(stderr,
                    "iniparser: input line too long in %s (%d)\n",
                    ininame,
                    lineno);
            dictionary_del(dict);
            fclose(in);
            return NULL ;
        }
        /* Get rid of \n and spaces at end of line */
        while((len > 0) &&
                ((line[end] == '\n') || (line[end] == '\r') || (isspace(line[end]))))
        {
            line[end] = 0 ;
            len--;
            end = len-1;
        }
        /* Detect multi-line */
        if((len>0)&&(line[end] == '\\'))
        {
            /* Multi-line value */
            last = end;
            continue ;
        }
        else
        {
            last = 0 ;
        }
        switch(iniparser_line(line, section, key, val))
        {
            case LINE_EMPTY:
            case LINE_COMMENT:
                break ;

            case LINE_SECTION:
                errs = dictionary_set(dict, section, NULL);
                break ;

            case LINE_VALUE:
                //sprintf(tmp, "%s:%s", section, key);
                substr1 = section;
                substr2 = key;
                snprintf(tmp, sizeof(tmp), "%s:%s", substr1, substr2);
                errs = dictionary_set(dict, tmp, val) ;
                break ;

            case LINE_ERROR:
                fprintf(stderr, "iniparser: syntax error in %s (%d):\n",
                        ininame,
                        lineno);
                fprintf(stderr, "-> %s\n", line);
                errs++ ;
                break;

            default:
                break ;
        }
        memset(line, 0, ASCIILINESZ);
        last = 0;
        if(errs < 0)
        {
            fprintf(stderr, "iniparser: memory allocation failure\n");
            break ;
        }
    }
    if(errs)
    {
        dictionary_del(dict);
        dict = NULL ;
    }
    fclose(in);
    return dict ;
}

void iniparser_freedict(dictionary * d)
{
    dictionary_del(d);
}

char * iniparser_getVal(dictionary * d,char * section,int i)
{
    if((d == NULL)) return NULL;
    char lwc_section[ASCIILINESZ+1] = {0};
    strlwc(section, lwc_section, ASCIILINESZ+1);
    return dictionary_getVal(d,lwc_section,i);
}

int iniparser_getNumberOfSection(dictionary * d,char * section)
{
	char lwc_section[ASCIILINESZ+1] = {0};
    strlwc(section, lwc_section, ASCIILINESZ+1);
    return dictionary_getNumberOfSection(d,lwc_section);
}


const char ** ParserStrToArray(const char *inputStr, int *count)
{
    char *str = NULL;
    const char **strArray = NULL;
    int leftIndex = -1, rightIndex = -2;
    int i = 0;
    int len = 0;

    for (i = 0; i < (int)strlen(inputStr); i++)
    {
        if (*(inputStr+i) == '{')
        {
            leftIndex = i + 1;
            break;
        }
    }
    for (i = strlen(inputStr)-1; i >= 0; i--)
    {
        if (*(inputStr+i) == '}')
        {
            rightIndex = i - 1;
            break;
        }
    }

    if (leftIndex > rightIndex)
    {
        return NULL;
    }
    len = rightIndex - leftIndex + 1;

    str = (char*)malloc(len+1);
    if(str)
    {
        strncpy(str,inputStr+leftIndex,len);
        str[len] = '\0';
        char *pch = strtok(str, " ,");
        while (pch != NULL)
        {
            pch = strtok(NULL, " ,");
            *count += 1;
        }
        free(str);

        if(*count)
        {
            const char **token = NULL;
            token = (const char**)malloc(sizeof(char*)*(*count+1)+len+1);
            if(token)
            {
                memset(token, 0, sizeof(char*)*(*count+1)+len+1);
                str = (char*)(token+(*count+1));
                strncpy(str,inputStr+leftIndex,len);
                str[len]='\0';
                i = 0;
                char *pch = strtok(str, ",");
                while(pch != NULL)
                {
                    token[i++] = pch;
                    pch = strtok(NULL, ",");
                }
                strArray = token;
            }
        }
    }

    return strArray;
}

