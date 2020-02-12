/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/

#include <shareLib.h>

#define DB_TEXT_GLBLSOURCE
#include "chandata.h"

#include <alarmString.h>

extern chandata *pchandata;

FILE *fw;
char *str;
int *off_loc;
chid *pchid;
struct dbr_ctrl_double *dbr;

void caInfo1_exit();
void caGetInfoArray();
void caInfo1_output();
void caInfo1_help();
extern int getopt();

float list_io_time=10.;
extern char *optarg;
extern int optind;

int main(argc,argv)
int argc;
char **argv;
{
int noName,i,c,type;
struct dbr_ctrl_double *db;

        while ((c = getopt(argc,argv,"w:")) != -1) 
        switch (c) {
        case 'w':
                list_io_time  = (float)atof(optarg);
                if (list_io_time < 10.) list_io_time = 10.;
                break;
        }

        if (argc==1 || argv[optind] == NULL ) str = (char *)readPipe(&noName,&caInfo1_help);
        else
        str = (char *)readFile0(argv[optind],&noName);
        off_loc = (int *)calloc(noName+1,sizeof(int));

        noName = readFile1(str,off_loc);
	
        pchid = (chid *)calloc(noName+1,sizeof(chid));
	dbr = (struct dbr_ctrl_double *)calloc(noName+1,sizeof(struct dbr_ctrl_double));
        if ( pchid == NULL || dbr == NULL) {
                printf("Error: alloc failed on pchid, dbr\n");
                caInfo1_exit(1);
                }

	caGetInfoArray(noName,str,off_loc,pchid,dbr);

	if (argc == (optind+2))  caInfo1_output(argv[optind+1],noName);
	else {
	printf("\n");
	for (i=0;i<noName;i++)  {
	type = ca_field_type(pchid[i]);
	if (type == TYPENOTCONN) {
		printf("%d %-30s *** NOT FOUND / NOT CONNECTED ***\n",type,str+off_loc[i]);
		continue;
		}
	if (type > DBR_STRING) {
	db = (struct dbr_ctrl_double *)(dbr+i);
	printf("%2d ",type);
	printf("%-30s ",str+off_loc[i]);
	printf("%10.2f ",db->value);
	printf("%-9s ",alarmStatusString[db->status]);
	printf("%-12s",alarmSeverityString[db->severity]);
	printf("%10.2f ",db->upper_disp_limit);
	printf("%10.2f ",db->lower_disp_limit);
	printf("%s\n ",db->units);
		}
	else printf("%d %-30s *** UNKNOWN:wrong type of record requested ***\n",type,str+off_loc[i]);

	}
	}
	printf(" *** caInfo completed normally ***\n");

	caInfo1_exit(0);

}


void caInfo1_output(filename,noName)
char *filename;
int noName;
{
int i;
chtype type;

	if ((fw = fopen(filename,"w")) == NULL ) {
	printf("Error: caInfo1 failed to open the output file '%s'\n",filename);
	caInfo1_exit(1);
	}

	for (i=0;i<noName;i++) { 
	type = ca_field_type(pchid[i]);
		if (type == TYPENOTCONN ) {
			fprintf(fw,"%2d ",ca_state(pchid[i]));
			fprintf(fw,"%-30s ",str+off_loc[i]);
			fprintf(fw," *** NOT FOUND / NOT CONNECTED ***\n");
			}
	else {
	if (type > DBR_STRING) {
		fprintf(fw,"%ld ",type);
		fprintf(fw,"%-30s ",str+off_loc[i]);
		fprintf(fw,"%10.2f ",dbr[i].value);
		fprintf(fw,"%-9s ",alarmStatusString[dbr[i].status]);
		fprintf(fw,"%-12s",alarmSeverityString[dbr[i].severity]);
		fprintf(fw,"%10.2f ",dbr[i].upper_disp_limit);
		fprintf(fw,"%10.2f ",dbr[i].lower_disp_limit);
		fprintf(fw,"%s\n ",dbr[i].units);
		}
		else {
			fprintf(fw,"%ld ",type);
			fprintf(fw,"%-30s ",str+off_loc[i]);
			fprintf(fw," *** UNKNOWN:wrong type of record requested ***\n");
			}
		}
	}
	fclose(fw);

}

void caInfo1_exit(i)
int i;
{
	free(str);
	free(off_loc);
	free(pchid);
	free(dbr);
	ca_task_exit(); 
	exit(0);	
}

void caInfo1_help() 
{
	printf("\nUsage:  caInfo1 [-w sec] <infile> [outfile]\n\n");
        printf("  [-w sec]   -   Option to override the default timeout wait time, \n");
	printf("                 default wait time is 10 seconds.\n");
        printf("  <infile>   -   Required, which contains a list of channel names\n");
        printf("                 each line contains a single channel name followed\n");
        printf("                 with a carriage return.\n");
        printf("  [outfile]  -   Optional output file, which records the obtained\n");
	printf("                 operating infomation from IOC for the requested\n");
	printf("                 <infile> list.\n");
	exit(1);
}

void caGetInfoArray(noName,str,off_loc,pchid,dbr)
int noName,*off_loc;
char *str;
chid *pchid;
struct dbr_ctrl_double *dbr;
{
int i;
	ca_task_initialize();

	for (i=0;i<noName;i++) 
		ca_search(str+off_loc[i],&pchid[i]);
	ca_pend_io(list_io_time);

	for (i=0;i<noName;i++) { 
		if (ca_field_type(pchid[i]) > DBR_STRING)
		ca_get(DBR_CTRL_DOUBLE,pchid[i],&dbr[i]);
		}
	ca_pend_io(list_io_time);

}

