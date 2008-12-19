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
 *
 */

#ifdef _WIN32
#include "windows.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include<sys/types.h>
#include<sys/stat.h>
#ifndef _WIN32
#include<sys/time.h>
#endif

#include <dbDefs.h>
#include <cadef.h>

#include "shareLib.h"

#if defined(SOLARIS)
unsigned sleep(unsigned seconds);
#endif


#ifndef FLDNAME_SZ
#define FLDNAME_SZ 4  /*Field Name Size*/
#endif

#ifndef NULL
#define NULL            0
#endif
#define NAME_LENGTH	PVNAME_SZ+FLDNAME_SZ+2
#define CA_SUCCESS 0
#define CA_FAIL   -1
#define CA_WAIT   -2            /* error = -2 waiting for new value */
                                /* error = -1 not connected */

#define cs_never_conn   0       /* channel never conn*/
#define cs_prev_conn    1       /* channel previously  conn*/
#define cs_conn         2       /* channel conn*/
#define cs_closed       3       /* channel cleared because never conn*/

struct caGlobals {
        int CA_ERR;
        int devprflag;
	int PEND_EVENT_ON;
        float PEND_IO_TIME;
        float PEND_IOLIST_TIME;
        float PEND_EVENT_TIME;
	int version;
        };

typedef struct event_queue {
	double *pvalues;
	short maxvalues;
	short numvalues;
	short overflow;
        char *scan_names;
        char **pscan;
	} EVENT_QUEUE;

typedef struct chandata{
	struct chandata *next;
        chid chid;
        chtype type;
        evid evid;
	int state;
	int form;
	int error;  int event;
        int status;
        int severity;
        double value; 
        float uopr;   /* upper_disp_limit */
        float lopr;   /* lower_disp_limit */
        float upper_alarm_limit;
        float upper_warning_limit;
        float lower_warning_limit;
        float lower_alarm_limit;
        float upper_ctrl_limit;
        float lower_ctrl_limit;
        float setpoint;
        float largest;
        float smallest;
	char units[8];
	char len;
	char string[MAX_STRING_SIZE]; 
	TS_STAMP stamp;
	EVENT_QUEUE *p_event_queue;
        } chandata;

#ifndef DB_TEXT_GLBLSOURCE
    epicsShareExtern struct caGlobals CA;
#else
    epicsShareDef struct caGlobals CA = {
        0,      /* CA_ERR */
        0,      /* devprflag */
        0,      /* PEND_EVENT_ON */
        3.0,    /* PEND_IO_TIME */
        5.0,   /* PEND_IOLIST_TIME */
        0.001f,  /* PEND_EVENT_TIME */
        3};     /* VERSION NO */
#endif

chandata *ca_check_hash_table(char *);
void ca_sleep_time(double t);
void ca_check_command_error(int i);
void ca_check_array_return_code(int status);
void ca_execerror(char *s, char *t);
void ca_put_real_value(char *name,double value);
void ca_get_all(char *name, double *vals);
void ca_get_string_array(int noName, char **pvName, char **value);
void ca_get_info_array(int noName, char **pvName, double *value);
void ca_populate_info_array(int noName, char **pvName);
void ca_get_error_array(int noName, char **pvName, int *value);
void ca_get_field_type_array(int noName, char **pvName, int *value);
void ca_get_element_count_array(int noName, char **pvName, int *value);
void ca_old_status_array(int noName, char **pvName, int *value);
void ca_get_status_array(int noName, char **pvName, int *value);
void ca_get_double_array2(int noName, char **pvName, double *value);
void ca_get_all_double_array(int noName,char **pvName, double *value);
void ca_get_all_string_array(int noName,char **pvName, char **value);
void ca_get_double_array(int noName, char **pvName, double *value);
void ca_put_string_array(int noName, char **pvName, char **value);
void ca_put_double_array(int noName, char **pvName, double *value);

double iocClockTime();
void ca_monitor_add_event_array();
void ca_monitor_clear_event_array();
void ca_monitor_add_event();
void ca_monitor_clear_event();
int ca_monitor_get_event();
void ca_monitor_get_event_array();
void ca_monitor_get_value_array();
void ca_monitor_get_all_array();
int ca_monitor_check_event();
int ca_monitor_check_event_array();
void ca_connect_change_event();
void  ca_monitor_value_change_event();
void  ca_monitor_string_change_event();
void ca_connect_add_event();
void ca_process_CA();

void ca_get_dbr_sts_double_callback(struct event_handler_args args);
void ca_get_dbr_sts_string_callback(struct event_handler_args args);
void ca_get_dbr_ctrl_double_callback(struct event_handler_args args);

epicsShareFunc void epicsShareAPI ca_check_return_code(int status);
epicsShareFunc double epicsShareAPI ca_get_real_value2(char *name);
epicsShareFunc double epicsShareAPI ca_get_real_value(char *name);
epicsShareFunc char * epicsShareAPI ca_get_string2(char *name);
epicsShareFunc char * epicsShareAPI ca_get_string(char *name);
epicsShareFunc int epicsShareAPI ca_find_dev(char *name, chandata *pchandata);
epicsShareFunc int epicsShareAPI ca_get_conn_data(int dbr_type, chandata *pchandata);
epicsShareFunc int epicsShareAPI ca_get_dbr_string(chandata *pchandata);
epicsShareFunc int epicsShareAPI ca_get_dbr_float(chandata *pchandata);
epicsShareFunc int epicsShareAPI ca_get_dbr_sts_string(chandata *pchandata);
epicsShareFunc int epicsShareAPI ca_get_dbr_sts_float(chandata *pchandata);
epicsShareFunc int epicsShareAPI ca_get_dbr_sts_double(chandata *pchandata);
epicsShareFunc int epicsShareAPI ca_get_dbr_gr_enum2(chandata *pchandata, struct dbr_gr_enum *dbr);
epicsShareFunc int epicsShareAPI ca_get_dbr_gr_enum(chandata *pchandata);
epicsShareFunc int epicsShareAPI ca_get_dbr_ctrl_double(chandata *pchandata);
epicsShareFunc int epicsShareAPI ca_put_conn_data(int dbr_type,chandata *pchandata, void *pvalue);
epicsShareFunc int epicsShareAPI ca_put_dbr_float(chandata *pchandata, void *pvalue);
epicsShareFunc int epicsShareAPI ca_put_dbr_double(chandata *pchandata, void *pvalue);
epicsShareFunc int epicsShareAPI ca_put_dbr_string(chandata *pchandata, void *pvalue);
epicsShareFunc int epicsShareAPI ca_search_list(int noName, char **pvName);
epicsShareFunc int epicsShareAPI ca_pvlist_search(int noName, char **pvName, chandata **list);
epicsShareFunc int epicsShareAPI ca_get_count(char *name);
epicsShareFunc int epicsShareAPI ca_get_wave_form(char *name, double *ret);
epicsShareFunc int epicsShareAPI ca_put_wave_form(char *name, int n, double *val);
epicsShareFunc int epicsShareAPI ca_get_native_wave_form(char *name, int count, void *pdata);
epicsShareFunc int epicsShareAPI ca_put_native_wave_form(char *name, int n, void *val);
epicsShareFunc int epicsShareAPI ca_put_string_wave_form(char *name, int n, char **val);
/* caRead */
epicsShareFunc char * epicsShareAPI readFile0(char *ptr,int *off);
epicsShareFunc int epicsShareAPI readFile1(char *ptr,int *off);
epicsShareFunc char * epicsShareAPI readPipe(int *noName, void func());
epicsShareFunc int epicsShareAPI new_string(char *ptr);

