/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/
/*
 *      Original Author: Ben-chin Cha
 *      Date:            3-10-92
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 */

#define NOTFOUND        -1
#define FOUND           1
#define TABSIZE         512

#define epicsExportSharedSymbols
#include <shareLib.h>

#include "chandata.h"

chandata *pchandata;

typedef struct item item;
struct item
{
    item *next;
    chandata *pchandata;
};

item *table[TABSIZE];
int num;                        /* no of occupied slots in table */
int succlen;                    /* total length of all successful searches */

/*
static unsigned short hash5();
*/
static int hash7();
static chandata *AllocChandata();
static int pv_search();
chandata *ca_check_hash_table();
int ca_clear_hash_table();

static int T[256] = {
    39, 159, 180, 252, 71, 6, 13, 164, 232, 35, 226, 155, 98, 120, 154, 69,
    157, 24, 137, 29, 147, 78, 121, 85, 112, 8, 248, 130, 55, 117, 190, 160,
    176, 131, 228, 64, 211, 106, 38, 27, 140, 30, 88, 210, 227, 104, 84, 77,
    75, 107, 169, 138, 195, 184, 70, 90, 61, 166, 7, 244, 165, 108, 219, 51,
    9, 139, 209, 40, 31, 202, 58, 179, 116, 33, 207, 146, 76, 60, 242, 124,
    254, 197, 80, 167, 153, 145, 129, 233, 132, 48, 246, 86, 156, 177, 36, 187,
    45, 1, 96, 18, 19, 62, 185, 234, 99, 16, 218, 95, 128, 224, 123, 253,
    42, 109, 4, 247, 72, 5, 151, 136, 0, 152, 148, 127, 204, 133, 17, 14,
    182, 217, 54, 199, 119, 174, 82, 57, 215, 41, 114, 208, 206, 110, 239, 23,
    189, 15, 3, 22, 188, 79, 113, 172, 28, 2, 222, 21, 251, 225, 237, 105,
    102, 32, 56, 181, 126, 83, 230, 53, 158, 52, 59, 213, 118, 100, 67, 142,
    220, 170, 144, 115, 205, 26, 125, 168, 249, 66, 175, 97, 255, 92, 229, 91,
    214, 236, 178, 243, 46, 44, 201, 250, 135, 186, 150, 221, 163, 216, 162, 43,
    11, 101, 34, 37, 194, 25, 50, 12, 87, 198, 173, 240, 193, 171, 143, 231,
    111, 141, 191, 103, 74, 245, 223, 20, 161, 235, 122, 63, 89, 149, 73, 238,
    134, 68, 93, 183, 241, 81, 196, 49, 192, 65, 212, 94, 203, 10, 200, 47
};

static int TT[256] = {
    1, 87, 49, 12, 176, 178, 102, 166, 121, 193, 6, 84, 249, 230, 44, 163,
    14, 197, 213, 181, 161, 85, 218, 80, 64, 239, 24, 226, 236, 142, 38, 200,
    110, 177, 104, 103, 141, 253, 255, 50, 77, 101, 81, 18, 45, 96, 31, 222,
    25, 107, 190, 70, 86, 237, 240, 34, 72, 242, 20, 214, 244, 227, 149, 235,
    97, 234, 57, 22, 60, 250, 82, 175, 208, 5, 127, 199, 111, 62, 135, 246,
    174, 169, 211, 58, 66, 154, 106, 195, 245, 171, 17, 187, 182, 179, 0, 243,
    132, 56, 148, 75, 128, 133, 158, 100, 130, 126, 91, 13, 153, 246, 216, 219,
    119, 68, 223, 78, 83, 88, 201, 99, 122, 11, 92, 32, 136, 114, 52, 10,
    138, 30, 48, 183, 156, 35, 61, 26, 143, 74, 251, 94, 129, 162, 63, 152,
    170, 7, 115, 167, 241, 206, 3, 150, 55, 59, 151, 220, 90, 53, 23, 131,
    125, 173, 15, 238, 79, 95, 89, 16, 105, 137, 225, 224, 217, 160, 37, 123,
    118, 73, 2, 157, 46, 116, 9, 145, 134, 228, 207, 212, 202, 215, 69, 229,
    27, 188, 67, 124, 168, 252, 42, 4, 29, 108, 21, 247, 19, 205, 39, 203,
    233, 40, 186, 147, 198, 192, 155, 33, 164, 191, 98, 204, 165, 180, 117, 76,
    140, 36, 210, 172, 41, 54, 159, 8, 185, 232, 113, 196, 231, 47, 146, 120,
    51, 65, 28, 144, 254, 221, 93, 189, 194, 139, 112, 43, 71, 109, 184, 209
};

/**************************************************
 * exclusive or used for hash table with two tables
 **************************************************/
/*
static unsigned short hash5(pname)
char *pname;
{
    unsigned short h=0;
    unsigned char  *h0=(unsigned char *)&h;
    unsigned char  *h1= h0 + 1;
    int   length,      i;

    length = strlen(pname);
    for(i=0; i<length; i+=2, pname+=2) {
        h = h ^ (*(unsigned short *)pname);
        *h0 = T[*h0];
        *h1 = TT[*h1];
    }
    return(*h0+*h1);
}
*/
/**************************************************
 * exclusive or used for hash table with two tables
 **************************************************/
static int
hash7(s)
     char *s;
{
    int i;
    int h, h0, h1;
    char *str;
    str = s;
    h = 0;
    h0 = 0;
    h1 = 0;
    for (i = 0; i < (int) strlen(s); i += 2, str += 2)
    {
        h0 = h0 ^ (*(char *) str);
        h1 = h1 ^ (*(char *) (str + 1));
        h0 = T[h0];
        h1 = TT[h1];
    }
    h = h0 + h1 + 1;
    return h;
}

/******************************************
 * allocate chandata 
 ******************************************/
static chandata *
AllocChandata()
{
    chandata *cdata;
    cdata = (chandata *) calloc(1, sizeof(chandata));
    if (cdata == NULL)
    {
        printf("AllocChandata failed on pchandata\n");
        exit(-2);
    }
    cdata->type = TYPENOTCONN;
    return (cdata);
}

/****************************************************************
 * find 's' in the hash table, if not found allocate space for it 
****************************************************************/
static int
pv_search(s)
     char *s;
{
    int h, len = 1;
    item **try;

    h = hash7(s);
    if (h < 0)
    {
        printf("Error: h can not be negative\n");
        exit(0);
    }
    try = &table[h];
    while (*try)
    {
        pchandata = (*try)->pchandata;
        if (strcmp(ca_name(pchandata->chid), s) == 0)
            return FOUND;
        len++;
        try = &((*try)->next);
    }

    *try = (item *) calloc(1, sizeof(item));
    if (*try == NULL)
    {
        printf("calloc failed on *try\n");
        exit(-2);
    }
    pchandata = (chandata *) AllocChandata();
    (*try)->pchandata = pchandata;
/*      strcpy(ca_name(pchandata->chid),s); */
    succlen += len;
    num++;
    return NOTFOUND;

}

/****************************************************************
 * add ca_search for new channel name
****************************************************************/
chandata *
ca_check_hash_table(name)
     char *name;
{
    int status;
    switch (pv_search(name))
    {
    case FOUND:                /*          printf("Already exists!\n");  */
        break;
    case NOTFOUND:
        status = ca_search(name, &(pchandata->chid));
        if (status != ECA_NORMAL)
            ca_check_return_code(status);
        break;
    }
    return (pchandata);
}

/***************************************************************
  clear hash table
***************************************************************/
int
ca_clear_hash_table()
{
    int i, status = ECA_NORMAL;
    item *temp, *try;

    for (i = 0; i < TABSIZE; i++)
    {
        temp = table[i];
        while (temp)
        {
            pchandata = temp->pchandata;
            if (pchandata->state != cs_closed)
            {
                if (pchandata->type != TYPENOTCONN && pchandata->evid)
                {
                    status = ca_clear_event(pchandata->evid);
                    if (status != ECA_NORMAL)
                        printf("ca_clear_hash_table: ca_clear_event failed on :[%s]\n",
                               ca_name(pchandata->chid));
                }
                if (pchandata->type != TYPENOTCONN && pchandata->chid)
                {
                    status = ca_clear_channel(pchandata->chid);
                    if (status != ECA_NORMAL)
                        printf("ca_clear_hash_table: ca_clear_channel failed on :[%s]\n",
                               ca_name(pchandata->chid));
                }
            }
            free(pchandata);
            try = temp->next;
            free(temp);
            temp = try;
        }
        table[i] = NULL;

    }
    ca_pend_io(3.0);
    return status;
}

/***************************************************************
  get channel list from the hash table
***************************************************************/
int
get_hash_table_channel_list(array)
     char **array;
{
    int i, no = 0;
    item *temp, *try;
    chandata *pchandata;

    for (i = 0; i < TABSIZE; i++)
    {
        temp = table[i];
        while (temp)
        {
            pchandata = temp->pchandata;
            array[no] = (char *) ca_name(pchandata->chid);
            try = temp->next;
            temp = try;
            no++;
        }
    }

    return (no);
}
