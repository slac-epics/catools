/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/
/* caDB.c */
/*
 *      Original Author: Ben-chin Cha
 *      Date:            10-22-93
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 */

#define epicsExportSharedSymbols
#include <shareLib.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <fcntl.h>

#include        "cvtFast.h" 
#include        "dbStaticLib.h"
#include        "epicsVersion.h"
#include        "errlog.h"

#define 	MAX_NUM_FIELD 	50

DBBASE  *ca_dbOpen();

#if  EPICS_REVISION && EPICS_REVISION == 13
#define dbFindRecordType    dbFindRecdes
#define dbGetNRecordTypes   dbGetNRecdes
#define dbNextRecordType    dbNextRecdes
#define dbFirstField        dbFirstFielddes
#define dbGetRecordTypeName dbGetRecdesName
#define dbFirstRecordType   dbFirstRecdes
#define dbNextField         dbNextFielddes
#define dbGetMenuChoices    dbGetChoices

long dbReadDatabaseFP(
    DBBASE **ppdbbase,FILE *fp, const char *path,const char *substitutions)
{
     return(dbRead(*ppdbbase,fp));
}
#endif


/* input:	database name
   output:      pdbbase, pdbentry
   return:      -1 failed
		0 succeeded
*/
DBBASE  *ca_dbOpen(filename)
char *filename;
{
    DBBASE      *pdbbase;
    FILE        *fp;
    long        status;
char file[80];
strcpy(file,filename);

        fp = fopen(file,"r");
        if (!fp) {
                printf("ERROR: Database file '%s' not found\n\n",filename);
                return(NULL);
                }
        pdbbase=dbAllocBase();
        status=dbReadDatabaseFP(&pdbbase,fp,0,0);
        if(status) errMessage(status,"dbRead");
        fclose(fp);
	if (status) return(NULL);
	return(pdbbase);
}


/* input: 	filename -   database name
		name	 -   record pv name
		field	 -   field name
   output:      S_type	 -   record type as string
		S_value  -   field value as string
   return:      1  if found
		0  if not found
*/
int ca_dbGetRecField(filename,name,field,S_type,S_value)
char *filename, *name, *field;
char S_type[],S_value[];
{
    DBBASE      *pdbbase;
    DBENTRY     dbentry;
    DBENTRY     *pdbentry=&dbentry;
    long        status;
    int         i=0;
char *temp;
char *type,*value;
char file[80];
strcpy(file,filename);

	pdbbase = (DBBASE *)ca_dbOpen(file);
	if (pdbbase==NULL ) {
                printf("ca_dbOpen: '%s' failed!\n",file);
                return(0);
                }

    dbInitEntry(pdbbase,pdbentry);
        status = dbFirstRecordType(pdbentry);
    if(status) {printf("No record description\n"); return status;}
    while(!status) {
	type = dbGetRecordTypeName(pdbentry);
        status = dbFirstRecord(pdbentry);
        if (!status) temp=dbGetRecordName(pdbentry);
        while (!status) {
                if (strcmp(name, temp) == 0) {
	i=1;
	if (strcmp(field,"TYPE") != 0 ) {
           status = dbFirstField(pdbentry,TRUE);
           if (status) printf("  No Fields\n");
           while (!status) {
                temp=dbGetFieldName(pdbentry);
                if (strcmp(field,temp) != 0 ) status = dbNextField(pdbentry,
TRUE);
                else {
                        value=dbGetString(pdbentry);
			strcpy(S_value,value);
			strcpy(S_type,type);
                        status=TRUE;
                        goto STEP1;
                                }
                        }
                }
	else {
		strcpy(S_type,type);
		strcpy(S_value,type);
		status = TRUE;
		goto STEP1;
		}
	}
           status = dbNextRecord(pdbentry);
           temp = dbGetRecordName(pdbentry);
        }
        status = dbNextRecordType(pdbentry);
   }
   printf("End of all Records! ");
		i=1;
		strcpy(S_type," ");
		strcpy(S_value," ");

STEP1:
    dbFinishEntry(pdbentry);

        dbFreeBase(pdbbase);

    return(i);
}


/* input:	database name
		record name
   output:
		S_type    - record_type 
		S_field[] - array of fieldnames
		S_value[] - array of fieldvalues
		D_type[]  - array of fieldtypes
   return:
		number of fields found
*/
int ca_dbGetRecFields(filename,name,S_type,S_field,S_value,D_type)
char *filename, *name;
char S_type[],S_field[][5],S_value[][MAX_STRING_SIZE];
int D_type[];
{
    DBBASE      *pdbbase;
    DBENTRY     dbentry;
    DBENTRY     *pdbentry=&dbentry;
    long        status;
    int         i=0,num=0;
char *temp;
char *type,*value;
char file[80];
strcpy(file,filename);

	pdbbase = (DBBASE *)ca_dbOpen(file);
	if (pdbbase==NULL ) {
                printf("ca_dbOpen: '%s' failed!\n",file);
                return(0);
                }

    dbInitEntry(pdbbase,pdbentry);
        status = dbFirstRecordType(pdbentry);
    if(status) {printf("No record description\n"); return status;}
    while(!status) {
	type = dbGetRecordTypeName(pdbentry);
        status = dbFirstRecord(pdbentry);
        if (!status) temp=dbGetRecordName(pdbentry);
        while (!status) {
                if (strcmp(name, temp) == 0) {
	i=0;
	   strcpy(S_type,type);
           status = dbFirstField(pdbentry,TRUE);
           if (status) printf("  No Fields\n");
           while (!status) {
                temp=dbGetFieldName(pdbentry);
		D_type[i] = dbGetFieldType(pdbentry);
                        value=dbGetString(pdbentry);
			strcpy(S_field[i],temp);
			strcpy(S_value[i],value);
                status = dbNextField(pdbentry,TRUE);
		i++;
		}
               status=TRUE;
               goto STEP1;
                        }
           status = dbNextRecord(pdbentry);
           temp = dbGetRecordName(pdbentry);
            }
        status = dbNextRecordType(pdbentry);
   }
   printf("End of all Records! ");
		i=0;
		strcpy(S_type," ");
		strcpy(S_value[0]," ");

STEP1:
    dbFinishEntry(pdbentry);

        dbFreeBase(pdbbase);

num = i;

    return(num);
}






/* input:	database name
		record type 
   output:
		S_field[] - array of fieldnames
		D_type[]  - array of fieldtype
   return:
		number of fields found
*/
int ca_dbGetRecFieldNames(filename,S_type,S_field,D_type)
char *filename, *S_type;
char S_field[][5];
int D_type[];
{
    DBBASE      *pdbbase;
    DBENTRY     dbentry;
    DBENTRY     *pdbentry=&dbentry;
    long        status;
    int         i=0,num=0;
char file[80];
strcpy(file,filename);

	pdbbase = (DBBASE *)ca_dbOpen(file);
	if (pdbbase==NULL ) {
                printf("ca_dbOpen: '%s' failed!\n",file);
                return(0);
                }

    dbInitEntry(pdbbase,pdbentry);

	status = dbFindRecordType(pdbentry,S_type);
	while(!status) {
	i=0;
           status = dbFirstField(pdbentry,TRUE);
           if (status) printf("  No Fields\n");
           while (!status) {
			strcpy(S_field[i],dbGetFieldName(pdbentry));
			D_type[i] = dbGetFieldType(pdbentry);
                status = dbNextField(pdbentry,TRUE);
		i++;
		}
               status=TRUE;
           }

    dbFinishEntry(pdbentry);

        dbFreeBase(pdbbase);

	num = i;

    return(num);
}




/* input:	pdbbase - database pointer	
		S_type  - record type 
   output:
		S_field[] - array of fieldnames
		D_type[] - array of fieldtype
   return:
		number of fields found
*/
int ca_dbGetRecFieldNames2(pdbbase,S_type,S_field,D_type)
DBBASE 	*pdbbase;
char *S_type;
char S_field[][5];
int D_type[];
{
    long        status;
    int         i=0,num=0;
DBENTRY dbentry;
DBENTRY *pdbentry=&dbentry;

    dbInitEntry(pdbbase,pdbentry);

	status = dbFindRecordType(pdbentry,S_type);
	while(!status) {
	i=0;
           status = dbFirstField(pdbentry,TRUE);
           if (status) printf("  No Fields\n");
           while (!status) {
			strcpy(S_field[i],dbGetFieldName(pdbentry));
			D_type[i] = dbGetFieldType(pdbentry);
                status = dbNextField(pdbentry,TRUE);
		i++;
		}
               status=TRUE;
           }

    dbFinishEntry(pdbentry);

	num = i;

    return(num);
}


/* input:	database pointer  
		record name
   output:
		S_type    - record_type 
		S_field[] - array of fieldnames
		S_value[] - array of fieldvalues
		D_type[] - array of fieldtype
   return:
		number of fields found
*/
int ca_dbGetRecFields2(pdbbase,name,S_type,S_field,S_value,D_type)
DBBASE      *pdbbase;
char *name;
char S_type[],S_field[][5],S_value[][MAX_STRING_SIZE];
int D_type[];
{
    DBENTRY     dbentry;
    DBENTRY     *pdbentry=&dbentry;
    long        status;
    int         i=0,num=0;
char *temp;
char *type,*value;

        pdbentry=dbAllocEntry(pdbbase);

    dbInitEntry(pdbbase,pdbentry);
        status = dbFirstRecordType(pdbentry);
    if(status) {printf("No record description\n"); return status;}
    while(!status) {
	type = dbGetRecordTypeName(pdbentry);
        status = dbFirstRecord(pdbentry);
        if (!status) temp=dbGetRecordName(pdbentry);
        while (!status) {
                if (strcmp(name, temp) == 0) {
	i=0;
	   strcpy(S_type,type);
           status = dbFirstField(pdbentry,TRUE);
           if (status) printf("  No Fields\n");
           while (!status) {
                temp=dbGetFieldName(pdbentry);
		D_type[i]=dbGetFieldType(pdbentry);
                        value=dbGetString(pdbentry);
			strcpy(S_field[i],temp);
			strcpy(S_value[i],value);
                status = dbNextField(pdbentry,TRUE);
		i++;
		}
               status=TRUE;
               goto STEP1;
                        }
           status = dbNextRecord(pdbentry);
           temp = dbGetRecordName(pdbentry);
            }
        status = dbNextRecordType(pdbentry);
   }
   printf("End of all Records! ");
		i=0;
		strcpy(S_type," ");
		strcpy(S_value[0]," ");

STEP1:
    dbFinishEntry(pdbentry);

num = i;

    return(num);
}


