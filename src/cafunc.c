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
 *      Experimental Physics and Industrial Control System (EPICS)
 */

#ifndef WIN32
#include <unistd.h>
#endif

#define epicsExportSharedSymbols
#include "chandata.h"

extern chandata *pchandata;

int dbr_type = DBR_STS_FLOAT;

extern double atof();
extern unsigned int sleep();

union val_tag {
        char cv[16000];
        char pv[400][40];
        short sv[8000];
        int iv[4000];
        float fv[4000];
        double dv[2000];
        } wf_buf;


/****************************************************
   sleep for micro seconds
*****************************************************/
void ca_sleep_time(t)
double t;    /* wait in seconds can be decimal*/
{
/* usleep isn't ANSI-C/POSIX
*/
unsigned u;
#if defined(HP_UX)
	sleep((unsigned int)t);
#elif defined(VMS)
	LIB$WAIT(t);
#elif defined(SGI)
	sleep((unsigned int)t);
#elif defined(SOLARIS)
	sleep((unsigned int)t);
#elif defined(_WIN32)
	u=1000* (unsigned int)t;
	Sleep(u);
#else
	u = 1000000 * t;
	usleep(u);
#endif
}

/****************************************************
   check for command return error code 
*****************************************************/
void ca_check_command_error(i) 
int i;
{
	if (i != 0) 
		CA.CA_ERR = CA_FAIL;
		else CA.CA_ERR = CA_SUCCESS;
}

/**************************************************
 * check a list of array error code return by CA
 **************************************************/
void ca_check_array_return_code(status)
int status;
{
  if (status == ECA_NORMAL) {
		CA.CA_ERR = CA_SUCCESS;
		return;
		}
    else CA.CA_ERR = CA_FAIL;

	
  if (status == ECA_TIMEOUT)
        ca_execerror("The operation timed out: "," on list of channels.");
} 

/**************************************************
 * check error code return by CA for a single device
 **************************************************/
void ca_check_return_code(status)
int status;
{
  if (status == ECA_NORMAL) {
		CA.CA_ERR = CA_SUCCESS;
		pchandata->error = CA_SUCCESS;
		return;
		}
    else { 
	CA.CA_ERR = CA_FAIL;
	pchandata->error = -2;
	}
	
  if (status == ECA_TIMEOUT)
        ca_execerror("The operation timed out: ",(char *)ca_name(pchandata->chid));
  if (status == ECA_GETFAIL)
        ca_execerror("A local database get failed: ",(char *)ca_name(pchandata->chid));
  if (status == ECA_BADCHID)
        ca_execerror("Unconnected or corrupted chid: ",(char *)ca_name(pchandata->chid));
  if (status == ECA_BADCOUNT)
        ca_execerror("More than native count requested: ",(char *)ca_name(pchandata->chid));
  if (status == ECA_BADTYPE)
        ca_execerror("Unknown GET_TYPE: ",(char *)ca_name(pchandata->chid));
  if (status == ECA_STRTOBIG)
        ca_execerror("Unusually large string supplied: ",(char *)ca_name(pchandata->chid));
  if (status == ECA_ALLOCMEM)
        ca_execerror("Unable to allocate memory: ",(char *)ca_name(pchandata->chid));
}

/**************************************************
 *   print error message
 *************************************************/
void ca_execerror(s,t)
char *s, *t;
{
fprintf(stderr,"%s %s\n ",s,t);
}

/***************************************************
 * put real value to ioc channel
 **************************************************/
void ca_put_real_value(name,value)
char *name;
double value;
{
int status;
chandata *pchan;

	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return;
	if (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;

	if (pchan->state == cs_conn) {
	status = ca_put_conn_data(DBR_DOUBLE,pchan,&value);
	if (CA.devprflag > 0 && status == ECA_NORMAL)
		fprintf(stderr,"put success");
	}
}

/**************************************************
 *  return a real value for a given channel name 
 **************************************************/
double ca_get_real_value2(name)
char *name;
{
double value=0;
int status;
chandata *pchan;
	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return value;
	if (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;

	if (pchan->state == cs_conn) {
	if (pchan->type != DBR_STRING) 
		ca_get_conn_data(DBR_STS_DOUBLE,pchan);
	else
		ca_get_conn_data(DBR_STS_STRING,pchan);

	value = pchan->value;
	}
	return (value);
}

/**************************************************
 *  return return value , status, severity 
 **************************************************/
void ca_get_all(name,vals)
char *name;
double *vals;
{
double value;
int status;
chandata *pchan;
	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return;
	if (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;

	CA.CA_ERR = CA_FAIL;
	if (pchan->state == cs_conn) {
        status = ca_get_conn_data(DBR_STS_DOUBLE,pchan);
        if (status == ECA_NORMAL) {
        value = pchan->value;
        *vals = value;
        value = pchan->status;
        *(vals+1) = value;
        value = pchan->severity;
        *(vals+2) = value;

	if (CA.devprflag > 0) {
       	 	fprintf(stderr,"name=%s,",name);
       		fprintf(stderr,"value=%f, status=%f, severity=%f\n ",
               	         *vals,*(vals+1),*(vals+2));
       	 	}

		CA.CA_ERR = CA_SUCCESS;
        }
	}
}

/**************************************************
 *  return a real value for a given channel name by using callback 
 **************************************************/
double ca_get_real_value(name)
char *name;
{
int status;
int count=0,imax;
double value=0;
chandata *pchan;

	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return status;
	while (strcmp(name,(char *)ca_name(pchandata->chid)) != 0 ) {
        ca_find_dev(name,pchandata);
        }
	pchan = pchandata;

#ifdef ACCESS_SECURITY
	if (ca_read_access(pchan->chid) == 0) {
	fprintf(stderr,"Read access denied on : %s\n",(char *)ca_name(pchan->chid));
	return value;
		}
#endif
	if (pchan->state == cs_conn) {
		ca_set_puser(pchan->chid,pchan);
		pchan->error = CA_WAIT;
             	status = ca_get_callback(DBR_STS_DOUBLE,pchan->chid,
				ca_get_dbr_sts_double_callback,pchan);
		if (status != ECA_NORMAL) {
			pchandata = pchan;
			ca_check_return_code(status);
			}
		else {

		/* wait for ca_get_callback to finish */

		imax =(int)(CA.PEND_IO_TIME / CA.PEND_EVENT_TIME);
                ca_pend_event(CA.PEND_EVENT_TIME);

                while(pchan->error == CA_WAIT) {
                        count++;
                        if (count > imax) {
                                ca_execerror("ca_get_real_value timeout on:",
                               (char *)ca_name(pchan->chid));
                                break;
                                }
                        ca_pend_event(CA.PEND_EVENT_TIME); 
                        }
		value = pchan->value;
		}
	}
	return (value);
}

/**************************************************
 *  return string value back for a given channel name 
 **************************************************/
char *ca_get_string2(name)
char *name;
{
int status;
chandata *pchan;

	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return(NULL);
	while (strcmp(name,(char *)ca_name(pchandata->chid)) != 0 ) {
        ca_find_dev(name,pchandata);
        }
	pchan = pchandata;
	if (pchan->state == cs_conn) 
	ca_get_conn_data(DBR_STS_STRING,pchan);

	return(pchan->string);
}

/**************************************************
 *  return string value back for a given channel name 
 **************************************************/
char *ca_get_string(name)
char *name;
{
int status,count=0,imax;
chandata *pchan;
	
	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return(NULL);
	while (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;

#ifdef ACCESS_SECURITY
	if (ca_read_access(pchan->chid) == 0) {
	fprintf(stderr,"Read access denied on : %s\n",(char *)ca_name(pchan->chid));
	return pchan->string;
		}
#endif
	if (pchan->state == cs_conn) {
		ca_set_puser(pchan->chid,pchan);
		pchan->error = CA_WAIT;
		status = ca_get_callback(DBR_STS_STRING,pchan->chid,
			ca_get_dbr_sts_string_callback,pchan);

		if (status != ECA_NORMAL) {
			pchandata = pchan;
			ca_check_return_code(status);
			}
		else {

		/* wait for ca_get_callback to finish */

		imax = (int)(CA.PEND_IO_TIME / CA.PEND_EVENT_TIME);
                ca_pend_event(CA.PEND_EVENT_TIME);
                while(pchan->error == CA_WAIT) {
                        count++;
                        if (count > imax) {
                                ca_execerror("ca_get_string timeout on:",
                               (char *)ca_name(pchan->chid));
                                break;
                                }
                        	ca_pend_event(CA.PEND_EVENT_TIME); 
                       		}
		}
	}
	return (pchan->string) ;
}

/**************************************************
 * find the channel name from IOC 
 **************************************************/
int ca_find_dev(name,pchandata)
char *name;
chandata *pchandata;
{
int status;

/* populate hash table here return the address */

  if (name) pchandata = (chandata *)ca_check_hash_table(name);
  else pchandata = (chandata *)ca_check_hash_table(" ");

  if (pchandata->type != TYPENOTCONN) return (CA_SUCCESS);

  	 status = ca_pend_io(CA.PEND_IO_TIME);
	 ca_check_return_code(status);

        pchandata->type = ca_field_type(pchandata->chid);
	pchandata->state = ca_state(pchandata->chid);

 
if (CA.devprflag > 0) fprintf(stderr,"chid=%p, type=%ld, state=%d\n",
	pchandata->chid,pchandata->type,pchandata->state);
        if (pchandata->state != cs_conn || pchandata->type == TYPENOTCONN) {
	if (CA.devprflag >= 0) 
		ca_execerror((char *)ca_name(pchandata->chid),"--- Invalid channel name");
		CA.CA_ERR = CA_FAIL;
		} else  CA.CA_ERR = CA_SUCCESS;
	return (CA.CA_ERR);
}

/**************************************************
 * get channel data back according to db request type
 **************************************************/
int ca_get_conn_data(dbr_type,pchandata)
int dbr_type;
chandata *pchandata;
{
int status;

switch (dbr_type) {

        case DBR_STRING:
                status = ca_get_dbr_string(pchandata);
                break;

        case DBR_FLOAT:
                status = ca_get_dbr_float(pchandata);
                break;

        case DBR_STS_STRING:
                status = ca_get_dbr_sts_string(pchandata);
                break;

        case DBR_STS_FLOAT:
                status = ca_get_dbr_sts_float(pchandata);
                break;

        case DBR_STS_DOUBLE:
                status = ca_get_dbr_sts_double(pchandata);
                break;

        case DBR_GR_ENUM:
                status = ca_get_dbr_gr_enum(pchandata);
                break;

        case DBR_CTRL_DOUBLE:
                status = ca_get_dbr_ctrl_double(pchandata);
                break;

        }
return status;
}

/**************************************************
 * get channel value for DBR_STRING
**************************************************/
int ca_get_dbr_string(pchandata)
chandata *pchandata;
{
int status;
char value[MAX_STRING_SIZE];

  if (pchandata->state == cs_conn) {

        status = ca_get(DBR_STRING,pchandata->chid,value);
        if (status != ECA_NORMAL) {
                ca_execerror("ca_get failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
	else {
        	status = ca_pend_io(CA.PEND_IO_TIME);
	        ca_check_return_code(status);
		if (status == ECA_NORMAL)
	        strcpy(pchandata->string,value);
		}
          return status;
        }
        else return ECA_BADCHID;
}

/**************************************************
 * get channel value for DBR_FLOAT
**************************************************/
int ca_get_dbr_float(pchandata)
chandata *pchandata;
{
int status;
float value=0.;

  if (pchandata->state == cs_conn) {

        status = ca_get(DBR_FLOAT,pchandata->chid,&value);
        if (status != ECA_NORMAL) {
                ca_execerror("ca_get failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
	else {
	        status = ca_pend_io(CA.PEND_IO_TIME);
	        ca_check_return_code(status);
		if (status == ECA_NORMAL)
	        pchandata->value = value;
		}
          return status;
        }
        else return ECA_BADCHID;
}

/**************************************************
 * get channel value for DBR_STS_STRING
**************************************************/
int ca_get_dbr_sts_string(pchandata)
chandata *pchandata;
{
int status;
struct dbr_sts_string dbr;

  if (pchandata->state == cs_conn) {
        status = ca_get(DBR_STS_STRING,pchandata->chid,&dbr);
        if (status != ECA_NORMAL) {
                ca_execerror("ca_get failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
	else {
          status = ca_pend_io(CA.PEND_IO_TIME);
          ca_check_return_code(status);

	if (status == ECA_NORMAL) {
		if (CA.devprflag > 1) {
		        fprintf(stderr,"\tvalue=%s\n",dbr.value);
		        fprintf(stderr,"\tstatus=%d\n",dbr.status);
		        fprintf(stderr,"\tseverity=%d\n",dbr.severity);
			}
	        pchandata->status = dbr.status;
	        pchandata->severity = dbr.severity;
		strcpy(pchandata->string,dbr.value);
	        pchandata->value = atof(dbr.value);
			}
		}
          return status;
        }
        else return ECA_BADCHID;

}

/**************************************************
 * get channel value for DBR_STS_FLOAT
**************************************************/
int ca_get_dbr_sts_float(pchandata)
chandata *pchandata;
{
int status;
struct dbr_sts_float dbr;

  if (pchandata->state == cs_conn) {
        status = ca_get(DBR_STS_FLOAT,pchandata->chid,&dbr);
        if (status != ECA_NORMAL) {
                ca_execerror("ca_get failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
	else {
          status = ca_pend_io(CA.PEND_IO_TIME);
          ca_check_return_code(status);
	if (status == ECA_NORMAL) {
	if (CA.devprflag > 1) {
       		 fprintf(stderr,"\tvalue=%f\n",dbr.value);
		 fprintf(stderr,"\tstatus=%d\n",dbr.status);
		 fprintf(stderr,"\tseverity=%d\n",dbr.severity);
		}
	        pchandata->value = dbr.value;
	        pchandata->status = dbr.status;
	        pchandata->severity = dbr.severity;
			}
		}
          return status;
        }
        else return ECA_BADCHID;

}

/**************************************************
 * get channel value for DBR_STS_DOUBLE
**************************************************/
int ca_get_dbr_sts_double(pchandata)
chandata *pchandata;
{
int status;
struct dbr_sts_double dbr;


  if (pchandata->state == cs_conn) {

        status = ca_get(DBR_STS_DOUBLE,pchandata->chid,&dbr);

        if (status != ECA_NORMAL) {
                ca_execerror("ca_get failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
	else {
          status = ca_pend_io(CA.PEND_IO_TIME);
          ca_check_return_code(status);
	if (status == ECA_NORMAL) {
	if (CA.devprflag > 1) {
       		fprintf(stderr,"\tvalue=%f\n",dbr.value);
	        fprintf(stderr,"\tstatus=%d\n",dbr.status);
	        fprintf(stderr,"\tseverity=%d\n",dbr.severity);
			}
       		pchandata->value = dbr.value;
	        pchandata->status = dbr.status;
	        pchandata->severity = dbr.severity;
		}
	}
          return status;
        }
        else return ECA_BADCHID;

}

int ca_get_dbr_gr_enum2(pchandata,dbr)
struct dbr_gr_enum *dbr;
chandata *pchandata;
{
int status;

  if (pchandata->state == cs_conn) {

        status = ca_get(DBR_GR_ENUM,pchandata->chid,dbr);
        if (status != ECA_NORMAL) {
                ca_execerror("ca_get failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
        else {
                status = ca_pend_io(CA.PEND_IO_TIME);
                ca_check_return_code(status);
                if (status == ECA_NORMAL)
                pchandata->value = dbr->value;
                pchandata->status= dbr->status;
                pchandata->severity = dbr->severity;
                strcpy(pchandata->string, dbr->strs[dbr->value]);
	}
	return status;
	}
        else return ECA_BADCHID;
}

/**************************************************
 * get channel status string for DBR_GR_ENUM  (name.STAT)
**************************************************/
int ca_get_dbr_gr_enum(pchandata)
chandata *pchandata;
{
int status,i;
struct dbr_gr_enum dbr;

  if (pchandata->state == cs_conn) {

        status = ca_get(DBR_GR_ENUM,pchandata->chid,&dbr);
        if (status != ECA_NORMAL) {
                ca_execerror("ca_get failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
	else {
	        status = ca_pend_io(CA.PEND_IO_TIME);
	        ca_check_return_code(status);
		if (status == ECA_NORMAL)
	        pchandata->value = dbr.value;
	        pchandata->status= dbr.status;
	        pchandata->severity = dbr.severity;
		strcpy(pchandata->string, dbr.strs[dbr.value]);

	/* print out status string defined in database */

if (CA.devprflag > 2) {
	fprintf(stderr,"name=%s,status=%d,severity=%d,value=%d\n",
	(char *)ca_name(pchandata->chid),dbr.status,dbr.severity,dbr.value);

	for (i=0;i<dbr.no_str;i++) 
	fprintf(stderr,"%d      %s\n",i,dbr.strs[i]);
	}
	

		}
          return status;
        }
        else return ECA_BADCHID;
}

/**************************************************
 * get channel value for DBR_CTRL_DOUBLE
**************************************************/
int ca_get_dbr_ctrl_double(pchandata)
chandata *pchandata;
{
int status;
struct dbr_ctrl_double dbr;

  if (pchandata->state == cs_conn) { 

        status = ca_get(DBR_CTRL_DOUBLE,pchandata->chid,&dbr);
        if (status != ECA_NORMAL) {
                ca_execerror("ca_get failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
	else {
          status = ca_pend_io(CA.PEND_IO_TIME);
          ca_check_return_code(status);
	if (status == ECA_NORMAL) {
if (CA.devprflag > 1) {
        fprintf(stderr,"\tprecision=%d\n",dbr.precision);
        fprintf(stderr,"\tRISC_pad0=%d\n",dbr.RISC_pad0);
        fprintf(stderr,"\tunits=%s\n",dbr.units);
	fprintf(stderr,"\tdisp_limit : [ %f : %f ]\n",
		dbr.lower_disp_limit, dbr.upper_disp_limit);
	fprintf(stderr,"\tctrl_limit : [ %f : %f ]\n",
		dbr.lower_ctrl_limit, dbr.upper_ctrl_limit);
        fprintf(stderr,"\tupper_alarm_limit=%f\n",dbr.upper_alarm_limit);
        fprintf(stderr,"\tupper_warning_limit=%f\n",dbr.upper_warning_limit);
        fprintf(stderr,"\tlower_warning_limit=%f\n",dbr.lower_warning_limit);
        fprintf(stderr,"\tlower_alarm_limit=%f\n",dbr.lower_alarm_limit);
        fprintf(stderr,"\tvalue=%f\n",dbr.value);
        fprintf(stderr,"\tstatus=%d\n",dbr.status);
        fprintf(stderr,"\tseverity=%d\n",dbr.severity);
	}

        pchandata->value = dbr.value;
        pchandata->uopr = (float)dbr.upper_disp_limit;
        pchandata->lopr = (float)dbr.lower_disp_limit;
        pchandata->upper_alarm_limit = (float)dbr.upper_alarm_limit;
        pchandata->upper_warning_limit = (float)dbr.upper_warning_limit ;
        pchandata->lower_warning_limit = (float)dbr.lower_warning_limit ;
        pchandata->lower_alarm_limit = (float)dbr.lower_alarm_limit ;
        pchandata->upper_ctrl_limit = (float)dbr.upper_ctrl_limit ;
        pchandata->lower_ctrl_limit = (float)dbr.lower_ctrl_limit ;
        pchandata->status = dbr.status;
        pchandata->severity = dbr.severity;
        strcpy(pchandata->units, dbr.units);
        if (pchandata->largest < dbr.value) pchandata->largest = (float)dbr.value;
        if (pchandata->smallest > dbr.value) pchandata->smallest = (float)dbr.value;
	}
	}
          return status;
        }
        else return ECA_BADCHID;

}

/********************************************************
 * write value to the connected channel according to db request type
 ********************************************************/
int ca_put_conn_data(dbr_type,pchandata,pvalue)
int dbr_type;
chandata *pchandata;
void *pvalue;
{
int status;

switch (dbr_type) {

        case DBR_FLOAT:
                status = ca_put_dbr_float(pchandata,pvalue);
                break;

        case DBR_DOUBLE:
                status = ca_put_dbr_double(pchandata,pvalue);
                break;

        case DBR_STRING:
                status = ca_put_dbr_string(pchandata,pvalue);
                break;
        }
return status;
}

/********************************************************
 * write a real value to the connected channel 
 ********************************************************/
int ca_put_dbr_float(pchandata,pvalue)
chandata *pchandata;
void *pvalue;
{
int status;
float value;

  value = *(float *)pvalue;

  pchandata->type = ca_field_type(pchandata->chid);

  if (pchandata->type !=TYPENOTCONN) {
        status = ca_put(DBR_FLOAT,pchandata->chid,pvalue);
	if (status != ECA_NORMAL) {
                ca_execerror("ca_put failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
	else pchandata->setpoint = value;
	
       if (CA.PEND_EVENT_ON) ca_pend_event(CA.PEND_EVENT_TIME); 
         return status;
        }
        else return ECA_BADCHID;
}

/********************************************************
 * write a double value to the connected channel 
 ********************************************************/
int ca_put_dbr_double(pchandata,pvalue)
chandata *pchandata;
void *pvalue;
{
int status;
double value;

  value = *(double *)pvalue;

  pchandata->type = ca_field_type(pchandata->chid);

  if (pchandata->state == cs_conn) {
        status = ca_put(DBR_DOUBLE,pchandata->chid,pvalue);
 	if (status != ECA_NORMAL) {
                ca_execerror("ca_put failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
  	else pchandata->setpoint = (float)value;
	
        if (CA.PEND_EVENT_ON)  ca_pend_event(CA.PEND_EVENT_TIME);
         return status;
        }
        else return ECA_BADCHID;
}

/********************************************************
 * write a string value to the connected channel 
 ********************************************************/
int ca_put_dbr_string(pchandata,pvalue)
chandata *pchandata;
void *pvalue;
{
int status;

  pchandata->type = ca_field_type(pchandata->chid);

  if (pchandata->state == cs_conn) {
        status = ca_put(DBR_STRING,pchandata->chid,pvalue);
	if (status != ECA_NORMAL) {
                ca_execerror("ca_put failed on",(char *)ca_name(pchandata->chid));
                ca_check_return_code(status);
                }
	else pchandata->setpoint = (float)atof(pchandata->string); 
	
        if (CA.PEND_EVENT_ON)  ca_pend_event(CA.PEND_EVENT_TIME);
         return status;
        }
        else return ECA_BADCHID;
}


/**************************************************
 *  casearch for a device array
 *  time out return CA_FAIL else return 0
 *************************************************/
int ca_search_list(noName,pvName)
int noName;
char **pvName;
{
int command_error=0;
chandata *list;

        command_error = ca_pvlist_search(noName,pvName,&list);
	return(command_error);
}


/**************************************************
 *  casearch for a device array
 *  time out return CA_FAIL else return 0
 *************************************************/
int ca_pvlist_search(noName,pvName,list)
int noName;
char **pvName;
chandata **list;
{
int i,status,command_error=CA_SUCCESS;
chandata *pnow,*phead,*pchan;

	phead = (chandata *)list;
	pnow = phead;
	for (i=0;i<noName;i++) {
	if (pvName[i]) pchan = (chandata *)ca_check_hash_table(pvName[i]);
	else pchan = (chandata *)ca_check_hash_table(" ");
		pnow->next = pchan;
		pchan->next = NULL;
		pnow = pchan;
		}
        status = ca_pend_io(CA.PEND_IOLIST_TIME);
        if (status != ECA_NORMAL) ca_check_array_return_code(status);

	pnow = phead->next; i=0;
	while (pnow)  {
		pchan = pnow;
		if (pchan->error == CA_WAIT) status = ECA_TIMEOUT;
		pchan->type = ca_field_type(pchan->chid);
		pchan->state = ca_state(pchan->chid);

		if (pchan->state != cs_conn) {
			if (CA.devprflag >= 0)
			fprintf(stderr,"%-30s  ***  channel not found\n",pvName[i]);
			pchan->error = CA_WAIT;
			command_error = CA_FAIL;
			}  else   pchan->error = 0;
		pnow = pnow->next; i++;
		}

	ca_check_command_error(command_error);
	if (command_error == CA_SUCCESS) return(CA_SUCCESS);  
		else return(CA_FAIL);
}

/**************************************************
 *  get string value back for a device array
 *************************************************/
void ca_get_string_array(noName,pvName,value)
int noName;
char **pvName;
char **value;
{
int i,status,command_error=0;
int count=0,imax;
chandata *list, *snode, *pchan;

	command_error = ca_pvlist_search(noName,pvName,&list);

        snode = list;
        while (snode)  {
                pchan = snode;
		if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {
                        ca_set_puser(pchan->chid,pchan);
                        pchan->error = CA_WAIT;
                        status = ca_get_callback(DBR_STS_STRING,pchan->chid,
                                ca_get_dbr_sts_string_callback,pchan);

                        if (status != ECA_NORMAL) {
                                 command_error = CA_FAIL;
                                 ca_check_array_return_code(status);
                                }
			}


                snode = snode->next;
                }
	
        imax = (int)(CA.PEND_IOLIST_TIME / CA.PEND_EVENT_TIME);

        snode = list; i=0;
        while (snode)  {
                pchan = snode;
                if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {

                        /* wait for ca_get_callback to finish */

                        while (pchan->error == CA_WAIT) {
                                count++;
                                if (count > imax) {
                	ca_execerror("ca_get_string_array timeout on:",
				(char *)ca_name(pchan->chid));
					break;
					}
                                ca_pend_event(CA.PEND_EVENT_TIME);
                                }
               		 *(value+i) = pchan->string;
			}
                snode = snode->next; i++;
		}	

	ca_check_command_error(command_error);
}
 

/**************************************************
 *  get string value,status,severity  back for a device array
 *************************************************/
void ca_get_all_string_array(noName,pvName,value)
int noName;
char **pvName;
char **value;
{
int i,status,command_error=0;
int count=0,imax;
chandata *list, *snode, *pchan;

	command_error = ca_pvlist_search(noName,pvName,&list);

        snode = list;
        while (snode)  {
                pchan = snode;
		if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {
                        ca_set_puser(pchan->chid,pchan);
                        pchan->error = CA_WAIT;
                        status = ca_get_callback(DBR_STS_STRING,pchan->chid,
                                ca_get_dbr_sts_string_callback,pchan);

                        if (status != ECA_NORMAL) {
                                 command_error = CA_FAIL;
                                 ca_check_array_return_code(status);
                                }
			}


                snode = snode->next;
                }
	
        imax = (int)(CA.PEND_IOLIST_TIME / CA.PEND_EVENT_TIME);

        snode = list; i=0;
        while (snode)  {
                pchan = snode;
                if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {

                        /* wait for ca_get_callback to finish */

                        while (pchan->error == CA_WAIT) {
                                count++;
                                if (count > imax) {
                	ca_execerror("ca_get_all_string_array timeout on:",
				(char *)ca_name(pchan->chid));
					break;
					}
                                ca_pend_event(CA.PEND_EVENT_TIME);
                                }
			sprintf(pchan->string,"%-20s %2d %2d",pchan->string,
				pchan->status,pchan->severity);
               		 *(value+i) = pchan->string;
			}
                snode = snode->next; i++;
		}	

	ca_check_command_error(command_error);
}
 

/**************************************************
 *  list information for a device array
 *************************************************/
void ca_get_info_array(noName,pvName,value)
int noName;
char **pvName;
double  *value;
{
int i,status,command_error=0;
int count=0,imax;
chandata *list, *snode, *pchan;

	command_error = ca_pvlist_search(noName,pvName,&list);

        snode = list;
        while (snode)  {
                pchan = snode;
		if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {
                ca_set_puser(pchan->chid,pchan);
                pchan->error = CA_WAIT;

		if (pchan->type == DBR_STRING)
		status = ca_get_callback(DBR_STS_STRING,pchan->chid, 
			ca_get_dbr_sts_string_callback,pchan);
		else
		status = ca_get_callback(DBR_CTRL_DOUBLE,pchan->chid,
			ca_get_dbr_ctrl_double_callback,pchan);

if (pchan->type == DBR_GR_ENUM || pchan->type ==  DBR_ENUM)  
	ca_get_conn_data(DBR_GR_ENUM,pchan);

	        if (status != ECA_NORMAL) {
			command_error = CA_FAIL;
			ca_check_array_return_code(status);
				}
			}
                snode = snode->next;
		}
        	status = ca_pend_event(CA.PEND_EVENT_TIME);

fprintf(stderr,"\nDEVICE                       TYPE VALUE STATUS SEVR  UNITS       UOPR     LOPR \n");

        imax = (int)(CA.PEND_IOLIST_TIME / CA.PEND_EVENT_TIME);

        snode = list; i=0;
        while (snode)  {
                pchan = snode;
                if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {

                        /* wait for ca_get_callback to finish */

                        while (pchan->error == CA_WAIT) {
                                count++;
                                if (count > imax) {
                	ca_execerror("ca_get_info_array timeout on:",
				(char *)ca_name(pchan->chid));
					break;
					}
                                ca_pend_event(CA.PEND_EVENT_TIME);
                                }
if (pchan->type != DBR_STRING)
fprintf(stderr,"%-30s %2ld %10f %2d %2d %-9s %10f %10f\n",
(char *)ca_name(pchan->chid),pchan->type,pchan->value,pchan->status,
pchan->severity,pchan->units,pchan->uopr,pchan->lopr);
else 
fprintf(stderr,"%-30s %2ld %-10s %2d %2d %-9s %10f %10f\n",
(char *)ca_name(pchan->chid),pchan->type,pchan->string,pchan->status,
pchan->severity,pchan->units,pchan->uopr,pchan->lopr);
			*(value+i) = pchan->value;
			}
                snode = snode->next;
		i++;
                }

	ca_check_command_error(command_error);
}

/**************************************************
 *  populate information for a device array
 *************************************************/
void ca_populate_info_array(noName,pvName)
int noName;
char **pvName;
{
int i,status,command_error=0;
int count=0,imax;
chandata *list, *snode, *pchan;


	command_error = ca_pvlist_search(noName,pvName,&list);

        snode = list;
        while (snode)  {
                pchan = snode;
		if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {
                ca_set_puser(pchan->chid,pchan);
                pchan->error = CA_WAIT;

		if (pchan->type == DBR_STRING)
		status = ca_get_callback(DBR_STS_STRING,pchan->chid, 
			ca_get_dbr_sts_string_callback,pchan);
		else
		status = ca_get_callback(DBR_CTRL_DOUBLE,pchan->chid,
			ca_get_dbr_ctrl_double_callback,pchan);

if (pchan->type == DBR_GR_ENUM || pchan->type ==  DBR_ENUM)  
	ca_get_conn_data(DBR_GR_ENUM,pchan);

	        if (status != ECA_NORMAL) {
			command_error = CA_FAIL;
			ca_check_array_return_code(status);
				}
			}
                snode = snode->next;
		}
        	status = ca_pend_event(CA.PEND_EVENT_TIME);

        imax = (int)(CA.PEND_IOLIST_TIME / CA.PEND_EVENT_TIME);

        snode = list; i=0;
        while (snode)  {
                pchan = snode;
                if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {

                        /* wait for ca_get_callback to finish */

                        while (pchan->error == CA_WAIT) {
                                count++;
                                if (count > imax) {
                		ca_execerror("ca_populate_info_array timeout",
				(char *)ca_name(pchan->chid));
					 break;
					}
                                ca_pend_event(CA.PEND_EVENT_TIME);
                                }
			}
		if (pchan->error == CA_WAIT) break;
                snode = snode->next;
		i++;
                }

	ca_check_command_error(command_error);
}

/**************************************************
 *  get error code  for a device array
 *************************************************/
void ca_get_error_array(noName,pvName,value)
int noName;
char **pvName;
int  *value;
{
int i,command_error=0;
chandata *list, *snode, *pchan;

	command_error = ca_pvlist_search(noName,pvName,&list);

        snode = list; i=0;
        while (snode)  {
                pchan = (chandata *)snode;
		pchan->type = ca_field_type(pchan->chid);
                if (pchan->state != cs_conn) {
			command_error = CA_FAIL;
			*(value+i) = CA_FAIL;
			}
                else {
			if (pchan->error != 0) command_error = CA_FAIL;
			*(value+i) = pchan->error;  /* timeout if is -2 */
			}
                snode = snode->next; i++;
                }

	ca_check_command_error(command_error);
}

/****************************************************
 *  get native DB field type for the given array 
 *  	-1 count will be returned for not connected channel
 ****************************************************/
void ca_get_field_type_array(noName,pvName,value)
int noName;
char **pvName;
int  *value;
{
int i,command_error=0;
chandata *list, *pnow, *pchan;

	command_error = ca_pvlist_search(noName,pvName,&list);

        pnow = list; i=0;
        while (pnow)  {
                pchan = pnow;
		pchan->type = ca_field_type(pchan->chid);
                if (pchan->state != cs_conn) {
			command_error = CA_FAIL;
			*(value+i) = CA_FAIL;
			}
                else {
			*(value+i) = pchan->type;
			}
                pnow = pnow->next; i++;
                }

	ca_check_command_error(command_error);
}

/****************************************************
 *  get native DB count for the given array 
 *  	0 count will be returned for not connected channel
 ****************************************************/
void ca_get_element_count_array(noName,pvName,value)
int noName;
char **pvName;
int  *value;
{
int i,command_error=0;
chandata *list, *pnow, *pchan;

	command_error = ca_pvlist_search(noName,pvName,&list);

        pnow = list; i=0;
        while (pnow)  {
                pchan = pnow;
		pchan->type = ca_field_type(pchan->chid);
                if (pchan->state != cs_conn) {
			command_error = CA_FAIL;
			*(value+i) = 0;
			}
                else {
			*(value+i) = ca_element_count(pchan->chid);
			}
                pnow = pnow->next; i++;
                }

	ca_check_command_error(command_error);
}

/****************************************************
 *  get status value back from current data structure 
 *   -1 indicates unconnected
 ****************************************************/
void ca_old_status_array(noName,pvName,value)
int noName;
char **pvName;
int  *value;
{
int i,command_error=0;
chandata *list, *pnow, *pchan;

	command_error = ca_pvlist_search(noName,pvName,&list);

        pnow = list; i=0;
        while (pnow)  {
                pchan = pnow;
		pchan->type = ca_field_type(pchan->chid);
                if (pchan->state != cs_conn) {
			command_error = CA_FAIL;
			*(value+i) = CA_FAIL;
			}
                else {
			*(value+i) = pchan->status;
			}
                pnow = pnow->next; i++;
                }

	ca_check_command_error(command_error);
}

/**************************************************
 *  get status value back for a device array
 *************************************************/
void ca_get_status_array(noName,pvName,value)
int noName;
char **pvName;
int  *value;
{
int i,status,command_error=0;
int  count=0,imax;
chandata *list, *snode, *pchan;

        command_error = ca_pvlist_search(noName,pvName,&list);

        snode = list;
        while (snode)  {
                pchan = snode;
                if (pchan->state == cs_conn) {
                     ca_set_puser(pchan->chid,pchan);
		     pchan->error = CA_WAIT;
		     if (pchan->type == DBR_STRING)  
		     status = ca_get_callback(DBR_STS_STRING,pchan->chid, 
				ca_get_dbr_sts_string_callback,pchan);
		else 	
	    	     status = ca_get_callback(DBR_STS_DOUBLE,pchan->chid, 
				ca_get_dbr_sts_double_callback,pchan);
		     if (status != ECA_NORMAL) {
				command_error = CA_FAIL;
                                ca_check_array_return_code(status);
				}
			}
                snode = snode->next;
                }

        ca_pend_event(CA.PEND_EVENT_TIME);
        imax = (int)(CA.PEND_IOLIST_TIME / CA.PEND_EVENT_TIME);

        snode = list; i=0;
        while (snode)  {
                pchan = snode;
                if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {

                        while (pchan->error == CA_WAIT) {
                                count++;
                                if (count > imax) {
                	ca_execerror("ca_get_status_array timeout on:",
				(char *)ca_name(pchan->chid));
					break;
					}
                                ca_pend_event(CA.PEND_EVENT_TIME);
                                }
			*(value+i) = pchan->status;
			}
                snode = snode->next; i++;
                }

	ca_check_command_error(command_error);
}

/**************************************************
 *  get double value back for a device array
 *************************************************/
void ca_get_double_array2(noName,pvName,value)
int noName;
char **pvName;
double *value;
{
int i,status,command_error=0;
chandata *list, *snode, *pchan;

        command_error = ca_pvlist_search(noName,pvName,&list);

        snode = list; i=0;
        while (snode)  {
                pchan = snode;
                if (pchan->state == cs_conn) {
			if (pchan->type == DBR_STRING)  
			status = ca_get_conn_data(DBR_STS_STRING,pchan);
			else 	
			status = ca_get_conn_data(DBR_STS_DOUBLE,pchan);
			if (status != ECA_NORMAL) command_error = CA_FAIL;
			*(value+i) = pchan->value;
			}
                snode = snode->next; i++;
                }

	ca_check_command_error(command_error);
}

/**************************************************
 *  get double value,status,severity  back for a device array using callback
 *************************************************/
void ca_get_all_double_array(noName,pvName,value)
int noName;
char **pvName;
double *value;
{
int i,status,command_error=0;
int count=0,imax;
chandata *list,*snode,*pchan;

        command_error = ca_pvlist_search(noName,pvName,&list);

        snode = list;
        while (snode)  {
                pchan = snode;
		if (pchan->state == cs_conn) {
			ca_set_puser(pchan->chid,pchan);
			pchan->error = CA_WAIT;
			status = ca_get_callback(DBR_STS_DOUBLE,pchan->chid,
				ca_get_dbr_sts_double_callback,pchan);

			if (status != ECA_NORMAL) {
				 command_error = CA_FAIL;
				 ca_check_array_return_code(status);
				}
			}
                snode = snode->next;
                }
        status = ca_pend_event(CA.PEND_EVENT_TIME);

	imax = (int)(CA.PEND_IOLIST_TIME / CA.PEND_EVENT_TIME);

        snode = list; i=0;
        while (snode)  {
                pchan = snode;
                if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {

			/* wait for ca_get_callback to finish */

			while (pchan->error == CA_WAIT) {
				count++;
				if (count > imax) {
                	ca_execerror("ca_get_all_double_array timeout on:",
				(char *)ca_name(pchan->chid));
					break;
					}
				ca_pend_event(CA.PEND_EVENT_TIME);
				}
			*(value+i*3) = pchan->value;
			*(value+i*3+1) = pchan->status;
			*(value+i*3+2) = pchan->severity;
			}
                snode = snode->next; i++;
		}
	
	ca_check_command_error(command_error);
}

/**************************************************
 *  get double value back for a device array using callback
 *************************************************/
void ca_get_double_array(noName,pvName,value)
int noName;
char **pvName;
double *value;
{
int i,status,command_error=0;
int count=0,imax;
chandata *list, *snode, *pchan;

        command_error = ca_pvlist_search(noName,pvName,&list);

        snode = list;
	
        while (snode)  {
                pchan = snode;
/*fprintf(stderr,"%s, %x, next=%x\n",(char *)ca_name(pchan->chid),pchan,pchan->next);
 */
		if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {
			ca_set_puser(pchan->chid,pchan);
			pchan->error = CA_WAIT;
			status = ca_get_callback(DBR_STS_DOUBLE,pchan->chid,
				ca_get_dbr_sts_double_callback,pchan);

			if (status != ECA_NORMAL) {
				 command_error = CA_FAIL;
				 ca_check_array_return_code(status);
				}
			}
                snode = snode->next;
                }
        status = ca_pend_event(CA.PEND_EVENT_TIME);

	imax = (int)(CA.PEND_IOLIST_TIME / CA.PEND_EVENT_TIME);

        snode = list; i=0;
        while (snode)  {
                pchan = snode;
                if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {

			/* wait for ca_get_callback to finish */

			while (pchan->error == CA_WAIT) {
				count++;
				if (count > imax) {
                	ca_execerror("ca_get_double_array timeout on:",
				(char *)ca_name(pchan->chid));
					break;
					}
				ca_pend_event(CA.PEND_EVENT_TIME);
				}
			*(value+i) = pchan->value;
			}
                snode = snode->next; i++;
		}

	ca_check_command_error(command_error);
}


/***************************************************************
 * put string array for a device array
 *************************************************************/
void ca_put_string_array(noName,pvName,value)
int noName;
char **pvName;
char **value;
{
int i,status,command_error=0;
chandata *list, *snode,*pchan;
 
 command_error = ca_pvlist_search(noName,pvName,&list);
 
 snode = list; i=0;
 while (snode)  {
   pchan = snode;
   if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {
     if (CA.devprflag > 0) 
       fprintf(stderr,"%-30s %s\n",pvName[i],value[i]);
     status = ca_put(DBR_STRING,pchan->chid,value[i]);
   }
   if (status != ECA_NORMAL) {
     ca_execerror("ca_put failed on",
		  (char *)ca_name(pchan->chid));
     pchandata = pchan;
     ca_check_return_code(status);
   }
   else pchan->setpoint = (float)atof(value[i]);
   
   snode = snode->next; i++;
 }
 
 ca_pend_event(CA.PEND_EVENT_TIME);
}

/***************************************************************
 * put double value array for a device array
 *************************************************************/
void ca_put_double_array(noName,pvName,value)
int noName;
char **pvName;
double *value;
{
int i,status,command_error=0;
chandata *pchan,*list,*snode;

 command_error = ca_pvlist_search(noName,pvName,&list);
 
 snode = list; i=0;
 while (snode)  {
   pchan = snode;
   if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {
     if (CA.devprflag > 1)
       printf("%-30s %f\n",pvName[i],value[i]);
     status = ca_put(DBR_DOUBLE,pchan->chid,value+i); 
   }
   
   if (status != ECA_NORMAL) {
     ca_execerror("ca_put failed on",
		  (char *)ca_name(pchan->chid));
     pchandata = pchan;
     ca_check_return_code(status);
   }
   else pchan->setpoint = (float)value[i];
   
   snode = snode->next; i++;
 }
 
 ca_pend_event(CA.PEND_EVENT_TIME);
}


/******************************************
	get record field no of data count
******************************************/
int ca_get_count(name)
char *name;
{
int no=0;     /* count */
int status;
chandata *pchan;

	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return no;
	while (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;

	if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {
       	 	no = ca_element_count(pchan->chid);
	}
	return no;
}

/******************************************
 * get waveform data
 * return a float array 
 *****************************************/
int ca_get_wave_form(name,ret)
char *name;
double *ret;     /* return real array */
{
int i,count=0,type,status;
unsigned size1,size2,size3;
chandata *pchan;

	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return -1;
	while (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;

#ifdef ACCESS_SECURITY
	if (ca_read_access(pchan->chid) == 0) {
	fprintf(stderr,"Read access denied on : %s\n",(char *)ca_name(pchan->chid));
	return CA_FAIL;
		}
#endif
        count = ca_element_count(pchan->chid);
if(count == 0 || pchan->state != cs_conn) {
	if (CA.devprflag >=0 )
	fprintf(stderr,"%s --- Invalid channel name\n",(char *)ca_name(pchan->chid));
	status = ECA_BADCHID;
	pchandata = pchan;
	ca_check_return_code(status);
	return status;
	}
if (pchan->state == cs_conn && pchan->type != TYPENOTCONN) {
        type = ca_field_type(pchan->chid);

        size1 = dbr_size_n(type,count);
        size2 = dbr_size[type];
        size3 = dbr_value_size[type];
if (CA.devprflag > 0)
fprintf(stderr,"type=%d, count=%d, size1=%d, size2=%d, size3=%d\n",
        type,count,size1,size2,size3);

        ca_array_get(type,count,pchan->chid,&wf_buf);
        status = ca_pend_io(CA.PEND_IOLIST_TIME);
        if (status != ECA_NORMAL) {
		pchandata = pchan;
		ca_check_return_code(status);
		}
		

switch (type) {

case DBR_CHAR:                          /* UCHAR */
        for (i=0;i<count;i++) *(ret+i) = wf_buf.cv[i];
        break;

case DBR_STRING:                        /* STRING*/
        for (i=0;i<count;i++) *(ret+i) = atof(wf_buf.pv[i]);
        break;


case DBR_FLOAT:                         /* FLOAT */
        for (i=0;i<count;i++) *(ret+i) = wf_buf.fv[i];
        break;

case DBR_DOUBLE:                        /* DOUBLE */
        for (i=0;i<count;i++) *(ret+i) = wf_buf.dv[i];
        break;

case DBR_SHORT:                        /* SHORT*/
        for (i=0;i<count;i++) *(ret+i) = wf_buf.sv[i];
        break;

case DBR_ENUM:                        /* ENUM */
        for (i=0;i<count;i++) *(ret+i) = wf_buf.sv[i];
        break;

default: /* CHAR SHORT USHORT  LONG ULONG */
        for (i=0;i<count;i++) *(ret+i) = wf_buf.iv[i];
        }

if (CA.devprflag > 1) {
        fprintf(stderr,"ca_get_wave_form: ");
        for (i=0;i<count;i++) fprintf(stderr,"%f ",*(ret+i));

        fprintf(stderr,"\n");
                }
    }

}

/******************************************
 * put waveform data
 * return  n data (n<2000)
 *         CA_FAIL failed
 *****************************************/
int ca_put_wave_form(name,n,val)
char *name;
int n;
double *val;

{
int i,count=0,type,status;
unsigned size1,size2,size3;
chandata *pchan;

	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return CA_FAIL;
	while (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;

#ifdef ACCESS_SECURITY
	if (ca_write_access(pchan->chid) == 0) {
	fprintf(stderr,"Write access denied on : %s\n",(char *)ca_name(pchan->chid));
	return CA_FAIL;
		}
#endif
        count = ca_element_count(pchan->chid);
if(count == 0 || pchan->state != cs_conn) {
	if (CA.devprflag >=0 )
	fprintf(stderr,"%s --- Invalid channel name\n",(char *)ca_name(pchan->chid));
	status = ECA_BADCHID;
	pchandata = pchan;
	ca_check_return_code(status);
	return(count);
	}
if (pchan->type != TYPENOTCONN && pchan->state == cs_conn) {
        type = ca_field_type(pchan->chid);

        size1 = dbr_size_n(type,count);
        size2 = dbr_size[type];
        size3 = dbr_value_size[type];
if (CA.devprflag > 0)
fprintf(stderr,"type=%d, count=%d, size1=%d, size2=%d, size3=%d\n",
        type,count,size1,size2,size3);

if (count > n) count=n;

if (CA.devprflag > 1) {
        fprintf(stderr,"ca_put_wave_form: ");
        for (i=0;i<count;i++) fprintf(stderr,"%f ",*(val+i));
        fprintf(stderr,"\n");
                }

switch (type) {

case DBR_CHAR:                          /* UCHAR */
        for (i=0;i<count;i++) wf_buf.cv[i] = (char)*(val+i);
        status = ca_array_put(type,count,pchan->chid,wf_buf.cv);
        break;

case DBR_STRING:                          /* STRING */
        for (i=0;i<count;i++) sprintf(wf_buf.pv[i],"%f", *(val+i));
        status = ca_array_put(type,count,pchan->chid,wf_buf.pv);
        break;

case DBR_FLOAT:                         /* FLOAT */
        for (i=0;i<count;i++) wf_buf.fv[i] = (float)*(val+i);
        status = ca_array_put(type,count,pchan->chid,wf_buf.fv);
        break;

case DBR_DOUBLE:                        /* DOUBLE */
        for (i=0;i<count;i++) wf_buf.dv[i] = *(val+i);
        status = ca_array_put(type,count,pchan->chid,wf_buf.dv);
        break;

case DBR_SHORT:                        /*  SHORT */
        for (i=0;i<count;i++) wf_buf.sv[i] = (short)*(val+i);
        status = ca_array_put(type,count,pchan->chid,wf_buf.sv);
        break;

case DBR_ENUM:                        /*  ENUM */
        for (i=0;i<count;i++) wf_buf.sv[i] = (short)*(val+i);
        status = ca_array_put(type,count,pchan->chid,wf_buf.sv);
        break;

default: /* CHAR UCHAR USHORT  LONG ULONG */
        for (i=0;i<count;i++) wf_buf.iv[i] = (int)*(val+i);
        status = ca_array_put(type,count,pchan->chid,wf_buf.iv);
        }

        if (status != ECA_NORMAL) {
		pchandata = pchan;
		ca_check_return_code(status);
		}
        ca_pend_event(CA.PEND_EVENT_TIME);
        return (count);
	}
	
        else { return(CA_FAIL); }
}


/******************************************
 * get waveform data in native data type
 * The caller must be careful to pass a pointer to an array which
 * can hold the number of elements defined in IOC
 *****************************************/
int ca_get_native_wave_form(name, count, pdata)
char *name;
int count;
void *pdata;
{
int type,status;
chandata *pchan;
	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return CA_FAIL;
	while (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;

#ifdef ACCESS_SECURITY
	if (ca_read_access(pchan->chid) == 0) {
	fprintf(stderr,"Read access denied on : %s\n",(char *)ca_name(pchan->chid));
	return CA_FAIL;
		}
#endif
if (pchan->type != TYPENOTCONN) {
        type = ca_field_type(pchan->chid);

if(pchan->state == cs_closed) {
	if (CA.devprflag >=0 )
	fprintf(stderr,"%s --- Invalid channel name\n",(char *)ca_name(pchan->chid));
	status = ECA_BADCHID;
	pchandata = pchan;
	ca_check_return_code(status);
	}
else {
if (CA.devprflag > 0)
   fprintf(stderr,"type=%d, count=%d\n",type,count);

        ca_array_get(type,count,pchan->chid,pdata);

	status = ca_pend_io(CA.PEND_IOLIST_TIME);
        if (status != ECA_NORMAL)  {
		pchandata = pchan;
		ca_check_return_code(status);
		}
    }
	}
        else return(CA_FAIL);

        return 0;
}

/******************************************
 * WARNING: from Mark Rivers, it is not working right
 * for string type yet
 * return  n data   
 *         CA_FAIL failed
 *****************************************/
int ca_put_native_wave_form(name,n,val)
char *name;
int n;
void *val;

{
int count,type,status;
chandata *pchan;

	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return CA_FAIL;
	while (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;
 
#ifdef ACCESS_SECURITY
	if (ca_write_access(pchan->chid) == 0) {
	fprintf(stderr,"Write access denied on : %s\n",(char *)ca_name(pchan->chid));
	return CA_FAIL;
		}
#endif
        if (pchan->type != TYPENOTCONN && pchan->state != cs_closed) {
           type = ca_field_type(pchan->chid);
           count = ca_element_count(pchan->chid);

           if (count > n) count=n;
           if (CA.devprflag > 0)
                fprintf(stderr,"ca_put_native_wave_form: type=%d, count=%d\n",
                                type,count);
		
           ca_array_put(type,count,pchan->chid,val);
           ca_pend_event(CA.PEND_EVENT_TIME);
           return (count);
        }

           else { return(CA_FAIL); }
}


/******************************************
 * put waveform data for string type
 * return  n data
 *         CA_FAIL failed
 *****************************************/
int ca_put_string_wave_form(name,n,val)
char *name;
int n;
char **val; 
{
int i;
int count,type,status,size;
chandata *pchan;

	status = ca_find_dev(name,pchandata);
	if (status == CA_FAIL) return status;
	while (strcmp(name,(char *)ca_name(pchandata->chid)) != 0) 
		ca_find_dev(name,pchandata); 
	pchan = pchandata;
 
#ifdef ACCESS_SECURITY
	if (ca_write_access(pchan->chid) == 0) {
	fprintf(stderr,"Write access denied on : %s\n",(char *)ca_name(pchan->chid));
	return CA_FAIL;
		}
#endif
	if (pchan->type != TYPENOTCONN && pchan->state != cs_closed) {
           type = ca_field_type(pchan->chid);
           count = ca_element_count(pchan->chid);
	   size = dbr_size_n(type,count);

	   if (count > n) count=n;
	   if (CA.devprflag > 0)
   		fprintf(stderr,"ca_put_string_wave_form: type=%d, count=%d\n",
           	                type,count);
	switch (type) {
	case DBR_CHAR:
		for (i=0;i<n;i++) wf_buf.cv[i]=atoi(val[i]);
       	   	status = ca_array_put(type,count,pchan->chid,wf_buf.cv);
		break;
	case DBR_STRING:
		for (i=0;i<n;i++) sprintf(wf_buf.pv[i],"%s",val[i]);
       	   	status = ca_array_put(type,count,pchan->chid,wf_buf.pv);
		break;
	case DBR_FLOAT:
		for (i=0;i<n;i++) wf_buf.fv[i]=(float)atof(val[i]);
       	   	status = ca_array_put(type,count,pchan->chid,wf_buf.fv);
		break;
	case DBR_DOUBLE:
		for (i=0;i<n;i++) wf_buf.dv[i]=atof(val[i]);
       	   	status = ca_array_put(type,count,pchan->chid,wf_buf.dv);
		break;
	case DBR_SHORT:
	case DBR_ENUM:
		for (i=0;i<n;i++) wf_buf.sv[i]=atoi(val[i]);
       	   	status = ca_array_put(type,count,pchan->chid,wf_buf.sv);
		break;
	default:
		for (i=0;i<n;i++) wf_buf.iv[i]=atoi(val[i]);
       	   	status = ca_array_put(type,count,pchan->chid,wf_buf.iv);
		break;
	}
	   if (status != ECA_NORMAL) {
		ca_check_return_code(status);
		pchandata = pchan;
		}
           ca_pend_event(CA.PEND_EVENT_TIME);
	   return (count);
        }
	
           else { return(CA_FAIL); }
}


