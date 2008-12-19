/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/
/* This program puts the values to IOC channels
  It reads an input file which contains a list of
  channel_name &  string_value and puts the specified 
  STRING_VALUE into the channel. 

  It can also accept input from the pipe
*/

#define DB_TEXT_GLBLSOURCE
#include "chandata.h"


struct stat buf;
extern chandata *pchandata;
FILE *fr,*fw;
char *str;
int *off_loc, *value_loc;
chid *pchid;

int readPutValue();
void caPutStringArray();
void caPut_help();
void caPut_exit();

char *caPut_readFile0();
char *caPut_readPipe();
float list_io_time=10.;
extern char *optarg;
extern int optind;
extern int getopt();
extern int fileno();
extern ssize_t read();

void main(argc,argv)
int argc;
char **argv;
{
int noName,c;

        while ((c = getopt(argc,argv,"w:")) != -1) 
        switch (c) {
        case 'w':
                list_io_time  = (float)atof(optarg);
                if (list_io_time < 10.) list_io_time = 10.;
                break;
        }

	if (argc == 1 || argv[optind] == NULL ) 
		str = (char *)caPut_readPipe(&noName);
	else
	str = (char *)caPut_readFile0(argv[optind],&noName);

	off_loc = (int *)calloc(noName+1,sizeof(int));
	value_loc = (int *)calloc(noName+1,sizeof(int));
	pchid = (chid *)calloc(noName+1,sizeof(chid));
        if (value_loc == NULL || off_loc == NULL || pchid == NULL) {
                printf("Error: alloc failed on value,value_string,pchid\n");
                caPut_exit(1);
                }

	noName = readPutValue(str,off_loc,value_loc); 

	caPutStringArray(noName,str,off_loc,value_loc,pchid);

	printf(" *** caPut completed normally ***\n");

        caPut_exit(0);
}

void caPut_help()
{
	printf("\nUsage:  caPut1 [-w sec] <infile>\n\n");
printf("This tool puts the set values to IOC channels.  It reads an input \n");
printf("file which contains a list of pair values of channel_name &  set_value\n");
printf("and puts the set_values into IOC through CA call. The set_value\n");
printf("must be in DBR_STRING form as created by caGet1.\n\n");
 
        printf("  [-w sec]   -   Option to override the default timeout wait time, \n");
        printf("                 default wait time is 10 seconds.\n");

        printf("  <infile>   -   Required, which contains a list of lines, each line\n");
	printf("                 consists of a channel name and the corresponding value\n");
	printf("                 to be put to IOC. \n\n");
	exit(1);
}

void caPut_exit(i)
int i;
{
	free(str);
	free(off_loc);
	free(value_loc);
	free(pchid);
}

void caPutStringArray(noName,str,off_loc,value_loc,pchid)
int noName;
char *str;
int *off_loc,*value_loc;
chid *pchid;
{
int i;

	ca_task_initialize();

	for (i=0;i<noName;i++)  
		ca_search(str+off_loc[i],&pchid[i]);
	ca_pend_io(list_io_time);

	printf("\n");
	for (i=0;i<noName;i++) 
		if (ca_field_type(pchid[i])!= TYPENOTCONN) {
		if (strlen(str+off_loc[i]) > 0) {
		ca_put(DBR_STRING,pchid[i],(str+value_loc[i])); 
		printf("%-30s %s\n",str+off_loc[i],str+value_loc[i]);
			}
		}
		else printf("%-30s  *** NOT FOUND / NOT CONNECTED ***\n",
			str+off_loc[i]);

	ca_flush_io(); 

	ca_task_exit();
}

int readPutValue(ptr,off_loc,value_loc)
char *ptr;
int *off_loc,*value_loc;
{
int l,i;

	l = 0; 
        for (i=0;i<buf.st_size;i++) {
		while(*(ptr+i) == ' ' || *(ptr+i) == '\t') i++;
		if (*(ptr+i) != 0) off_loc[l] = i;
		while (*(ptr+i+1) != ' ' && *(ptr+i+1) != '\t' &&
			*(ptr+i+1) != 0) i++;
		*(ptr+i+1) = 0;
		i++;
		while (*(ptr+i+1) == ' ' || *(ptr+i+1) == '\t') i++;  
                   if (*(ptr+i+1) != 0) value_loc[l] = i+1;
		i++;
		while (*(ptr+i+1) != '\n') i++;
		*(ptr+i+1)=0;  if (value_loc[l] == 0) value_loc[l]=i+1;
		l++; i=i+1;
                }
	return(l);
}


char *caPut_readFile0(filename,noName)
char *filename;
int *noName;
{
char *ptr;
int l,i,fd;

if ((fr = fopen(filename,"r")) == NULL ) {
        printf("Error: caPut failed to open the input file  '%s'\n",filename);
        exit(1);
        }
        fd = fileno(fr);
        fstat(fd,&buf);
        ptr = (char *)calloc(1,buf.st_size+1);
        i = fread(ptr,buf.st_size,1,fr);
        if (i == 0) {
                printf("fread error %s \n",filename);
                exit(1);
                }
        *(ptr+buf.st_size) = '\0';
        fclose(fr);

        l=0;
        for (i=0;i<buf.st_size;i++) {
        if (*(ptr+i) == '\n') {
                l++; /* *(ptr+i) = 0; */
                }
        }
        *noName = l;

        return(ptr);
}



char *caPut_readPipe(noName)
int *noName;
{
char *ptr;
int l,i,fd;

	fd = 0;
        fstat(fd,&buf);
        ptr = (char *)calloc(1,buf.st_size+1);
        i = read(fd,ptr,buf.st_size);
        if (i == 0 || i != buf.st_size) caPut_help();
        *(ptr+buf.st_size) = '\0';

        l=0;
        for (i=0;i<buf.st_size;i++) {
        if (*(ptr+i) == '\n') {
                l++; /**(ptr+i) = 0; */
                }
        }
        *noName = l;

        return(ptr);
}


