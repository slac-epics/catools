/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/
/* caMonWF.c
 *  monitor queue for waveform record
 */

#define epicsExportSharedSymbols
#include <shareLib.h>

#include "chandata.h"

extern chandata *pchandata;


/******************************************************
  get waveform values for a monitored pv, 
  return CA.CA_ERR if failed, 
  return whole array 
******************************************************/
int ca_monitor_WF_get(pvName,count,vals)
char *pvName;
int count;
void *vals;
{
int status,command_error=0,num;
chandata *pchan;

	num = CA_FAIL;
	ca_pend_event(0.00001);

	ca_find_dev(pvName,pchandata);
	while(strcmp(pvName,ca_name(pchandata->chid)) != 0 ) {
		ca_find_dev(pvName,pchandata);
	}
	pchan = pchandata;

        pchan->type = ca_field_type(pchan->chid);
        if (pchan->state != cs_conn) 
		command_error = CA_FAIL;

	else  if (pchan->evid ) {
		status = ca_get_native_wave_form(pvName,count,vals);

	if (CA.devprflag > 0) { 
		fprintf(stderr,"caMonitorWF get: name=%s, value=%f, status=%d, event=%d\n",
		ca_name(pchan->chid),pchan->value,
		pchan->status,pchan->event);
		fprintf(stderr,"\n");
		}
	if (pchan->event > 0) {
		num = count;
		pchan->event = 0;  /* reset after read*/
		}
	} else {
                if (CA.devprflag >= 0)
		 fprintf(stderr,"Error: %s is not monitored yet.\n",
			ca_name(pchan->chid)); 
		}

        ca_check_command_error(command_error);

	return (num);
}

