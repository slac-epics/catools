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
 *      Date:            8-10-92
 *
 */

#include <sys/stat.h>

#define DB_TEXT_GLBLSOURCE
#include "chandata.h"

char *str;
int *off_loc;
char *value,**value_string;
chid *pchid;

float list_io_time=10.;
extern char *optarg;
extern int optind;
extern int getopt();

void caGetStringArray();
void caGet_help();
void caGet_output();
void caGet_exit();

int main(argc,argv)
int argc;
char **argv;
{
int noName,i,c; 


	while ((c = getopt(argc,argv,"w:")) != -1) 
        switch (c) {
        case 'w':
                list_io_time  = (float)atof(optarg);
                if (list_io_time < 10.) list_io_time = 10.;
                break;
        }

	if (argc==1 || argv[optind] == NULL ) str = (char *)readPipe(&noName,caGet_help);
	else
	str = (char *)readFile0(argv[optind],&noName);
	off_loc = (int *)calloc(noName+1,sizeof(int));

	noName = readFile1(str,off_loc);

	value = (char *	)calloc(noName+1,MAX_STRING_SIZE);
	value_string = (char **)calloc(noName+1,sizeof(char *));
	pchid = (chid *)calloc(noName+1,sizeof(chid));
	if (value == NULL || value_string== NULL || pchid == NULL) {
		printf("Error: alloc failed on value,value_string,pchid\n");
		exit(1);
		}
	
	caGetStringArray(noName,str,off_loc,pchid,value,value_string); 

	if (argc == (optind + 2)) caGet_output(argv[optind+1],noName);
	else {
	printf("\n");
	for (i=0;i<noName;i++) {
                if (ca_field_type(pchid[i]) != TYPENOTCONN) {
		printf("%-30s  %s\n",str+off_loc[i],value_string[i]);
		}
                else printf("%-30s  *** NOT FOUND / NOT CONNECTED ***\n",
			str+off_loc[i]);
		}
	}

	printf(" *** caGet completed normally ***\n");
	caGet_exit(0);
}

void caGet_help()
{
	printf("\nUsage:  caGet1 [-w sec] <infile> [outfile]\n\n");

	printf("This tool uses CA to get values in DBR_STRING form for a list of PVs.\n");
	printf("Output results are sent to stdout or saved in a output file.\n\n");
        printf("  [-w sec]   -   Option to override the default timeout wait time, \n");
        printf("                 default wait time is 10 seconds.\n");
	printf("  <infile>   -   Required, which contains a list of channel names,\n");
	printf("                 each line contains a single channel name followed\n");
	printf("                 with a carriage return.\n");
	printf("  [outfile]  -   Optional output file, which records the obtained values\n");
	printf("                 from IOC for the requested <infile> list.\n");
	printf("                 This output file can be used directly by caPut1.\n\n");

	exit(1);
}


void caGet_output(filename,noName) 
char *filename;
int noName;
{
int i;
FILE *fw;
	if ((fw = fopen(filename,"w")) == NULL ) {
	printf("Error: caGet failed to open the output file '%s'\n",filename);
	caGet_exit(1);
	}

	for (i=0;i<noName;i++) 
		fprintf(fw,"%-30s %s\n",str+off_loc[i],value_string[i]);	
	fclose(fw);
}

void caGet_exit(i) 
int i;
{
	free(str);
	free(off_loc);
	free(value);
	free(value_string);
	free(pchid);
	ca_task_exit();
	exit(0);
}

void caGetStringArray(noName,str,off_loc,pchid,value,value_string)
int noName,*off_loc;
char *str;
char *value;
char **value_string;
chid *pchid;
{
int i,type;

	ca_task_initialize();

	for (i=0;i<noName;i++) 
		ca_search(str+off_loc[i],&pchid[i]);
	ca_pend_io(list_io_time);

	for (i=0;i<noName;i++) {
	type = ca_field_type(pchid[i]);
		value_string[i] = (char *)(value + i*MAX_STRING_SIZE);
		ca_get(DBR_STRING,pchid[i],value_string[i]);
	}
	ca_pend_io(list_io_time);

}

