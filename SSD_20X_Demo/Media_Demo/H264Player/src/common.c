#include <sys/time.h>
#include "common.h"


MI_U64 getOsTime(void)
{
    MI_U64 u64CurTime = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    u64CurTime = ((unsigned long long)(tv.tv_sec))*1000 + tv.tv_usec/1000;
    return u64CurTime;
}



/********************************************Vdec**************************************************/
NALU_t *AllocNALU(int buffersize)
{
    NALU_t *n;
    if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)
    {
        printf("AllocNALU: calloc n error\n");
        return NULL;
    }
    n->max_size = buffersize;
    //printf("AllocNALU: calloc buf size = %d\n", n->max_size);

    if ((n->buf = (char*)calloc (buffersize, sizeof (char))) == NULL)
    {
        free (n);
        printf("AllocNALU: calloc n->buf error\n");
        return NULL;
    }
    return n;
}


void FreeNALU(NALU_t *n)
{
    if (n)
    {
        if (n->buf)
        {
            free(n->buf);
            n->buf=NULL;
        }
        free (n);
    }
}

int FindStartCode2 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 1))
        return 0;
    else
        return 1;
}

int FindStartCode3 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 0) || (Buf[3] != 1))
        return 0;
    else
        return 1;
}

int GetAnnexbNALU (NALU_t *nalu, MI_S32 chn, FILE *fp)
{
    int pos = 0;
    int StartCodeFound, rewind;
    unsigned char *Buf;
    int info2 = 0, info3 = 0;

    if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
    {
        printf("GetAnnexbNALU: Could not allocate Buf memory\n");
        return -1;
    }

    nalu->startcodeprefix_len=3;
    if (3 != fread (Buf, 1, 3, fp))
    {
        free(Buf);
        return -1;
    }
    info2 = FindStartCode2 (Buf);
    if(info2 != 1)
    {
        if(1 != fread(Buf+3, 1, 1, fp))
        {
            free(Buf);
            return -1;
        }
        info3 = FindStartCode3 (Buf);
        if (info3 != 1)
        {
            free(Buf);
            return -1;
        }
        else
        {
            pos = 4;
            nalu->startcodeprefix_len = 4;
        }
    }
    else
    {
        nalu->startcodeprefix_len = 3;
        pos = 3;
    }
    StartCodeFound = 0;
    info2 = 0;
    info3 = 0;
    while (!StartCodeFound)
    {
        if (feof (fp))
        {
            nalu->len = (pos-1);
            memcpy (nalu->buf, &Buf[0], nalu->len);
            free(Buf);
            if (!_bChkStreamEnd)
            {
                fseek(fp, 0, 0);
            }
            else
            {
                printf("end of file...\n");
            }
            return pos-1;
        }
        Buf[pos++] = fgetc (fp);
        info3 = FindStartCode3(&Buf[pos-4]);
        if(info3 != 1)
            info2 = FindStartCode2(&Buf[pos-3]);
        StartCodeFound = (info2 == 1 || info3 == 1);
    }
    rewind = (info3 == 1) ? -4 : -3;
    if (0 != fseek (fp, rewind, SEEK_CUR))
    {
        free(Buf);
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file\n");
    }
    nalu->len = (pos+rewind);
    memcpy (nalu->buf, &Buf[0], nalu->len);
    free(Buf);
    return (pos+rewind);
}




