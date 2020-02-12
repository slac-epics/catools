/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define epicsExportSharedSymbols
#include <shareLib.h>

#include "chandata.h"

static struct stat buf;
FILE *fr;
/*
char *readFile0();
char *readPipe();
*/
extern int fileno();
extern ssize_t read();


int
readFile1(ptr, off_loc)
     int *off_loc;
     char *ptr;
{
    int i, l, c, i0;

    l = 0;
    i = 0;
    c = *ptr;
    if (c != 0 && c != ' ' && c != '\t')
        off_loc[0] = 0;
    else
    {
        while (*(ptr + i + 1) == ' ' || *(ptr + i + 1) == '\t' || *(ptr + i + 1) == 0)
            i++;
        off_loc[0] = i + 1;
    }
    i0 = i + 1;
    for (i = i0; i < buf.st_size; i++)
    {
        c = *(ptr + i);
        if (c != 0)
            continue;
        while (*(ptr + i + 1) == ' ' || *(ptr + i + 1) == '\t' || *(ptr + i + 1) == 0)
        {
            i++;
            // Fix bug where we were core dumping here in case ptr is memset to 0?
            // This is a local patch; it is possible that caTools is no longer maintained by the collaboration
            if (i >= buf.st_size)
                break;
        }
        l++;
        off_loc[l] = i + 1;
    }
    return l;
}

char *
readFile0(filename, noName)
     char *filename;
     int *noName;
{
    char *ptr;
    int l, i, fd, end;

    if ((fr = fopen(filename, "r")) == NULL)
    {
        printf("Error: caGet failed to open the input file  '%s'\n", filename);
        exit(1);
    }
    fd = fileno(fr);
    fstat(fd, &buf);
    ptr = (char *) calloc(1, buf.st_size + 1);
    i = fread(ptr, buf.st_size, 1, fr);
    if (i == 0)
    {
        printf("fread error %s \n", filename);
        exit(1);
    }
    *(ptr + buf.st_size) = '\0';
    fclose(fr);

    l = 0;
    for (i = 0; i < buf.st_size; i++)
    {
        if (*(ptr + i) == '\n')
        {
            end = i;
            while (*(ptr + end - 1) == '\t' || *(ptr + end - 1) == ' ')
                end--;
            if (end < i)
            {
                *(ptr + i) = ' ';
                *(ptr + end) = 0;
            }
            else
                *(ptr + i) = 0;
            l++;
        }
    }
    *noName = l;

    return (ptr);
}


/* Note: The pipe buff on unix is only 4096 bytes */
char *
readPipe(noName, ca_help)
     int *noName;
     void ca_help();
{
    char *ptr;
    int l, i, fd, end;

    fd = 0;
    fstat(fd, &buf);
    ptr = (char *) calloc(1, buf.st_size + 1);
    i = read(fd, ptr, buf.st_size);
/* empty pipe or read error */
    if (i == 0 || i != buf.st_size)
        ca_help();
    *(ptr + buf.st_size) = '\0';

    l = 0;
    for (i = 0; i < buf.st_size; i++)
    {
        if (*(ptr + i) == '\n')
        {
            end = i;
            while (*(ptr + end - 1) == '\t' || *(ptr + end - 1) == ' ')
                end--;
            if (end < i)
            {
                *(ptr + i) = ' ';
                *(ptr + end) = 0;
            }
            else
                *(ptr + i) = 0;
            l++;
        }
    }
    *noName = l;

    return (ptr);
}

/* return position of the first non blank string position */
int
new_string(st)
     char *st;
{
    int len, i = 0;
    len = strlen(st);
    while (*(st + len - 1) == '\t' || *(st + len - 1) == ' ')
        len--;
    *(st + len) = 0;
    i = 0;
    while (*(st + i) == '\t' || *(st + i) == ' ')
        i++;
    return (i);
}
