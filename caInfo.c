/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/


#include <alarmString.h>

#define DB_TEXT_GLBLSOURCE
#include "chandata.h"

extern chandata *pchandata;

void caGetInfo();
extern int getopt();

void main(argc,argv)
int argc;
char **argv;
{
char *st,name[NAME_LENGTH];
	st = name;

printf("\nEnter PV_name or quit\n\n");

READPV:	printf("PV? : "); 
	st = gets(name);
	if (st == NULL) exit(1);
	if (*st != EOF && *st != NULL) {
	if(*st == 'q' || *st == 'Q') {
			 exit(0); 
			}
	st = &name[new_string(st)];
		if (strlen(st) > 1) {
		caGetInfo(st);
		}
	}
	goto READPV;
}

void caGetInfo(name) 
char *name;
{
struct dbr_ctrl_double dbr;
int status;

   status = ca_find_dev(name,pchandata);

        if (pchandata->state  != cs_conn ) return;
        if (pchandata->type > DBR_STRING) {
	status = ca_get(DBR_CTRL_DOUBLE,pchandata->chid,&dbr);
        ca_pend_io(1.);
	ca_check_return_code(status);
	if (status == ECA_NORMAL) {
	printf("%-30s ",name);
	printf("%10.2f ",dbr.value);
	printf("%-9s ",alarmStatusString[dbr.status]);
	printf("%-12s",alarmSeverityString[dbr.severity]);
	printf("%10.2f ",dbr.upper_disp_limit);
	printf("%10.2f ",dbr.lower_disp_limit);
	printf("%s\n ",dbr.units);
		}
	}
	else printf("*** wrong type of record entered ***\n");

}

