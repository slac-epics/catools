/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/
/* caScan.c
 *  special monitor queue for scan record
 */

#define epicsExportSharedSymbols
#include <shareLib.h>

#include "chandata.h"

extern chandata *pchandata;

void ca_monitor_value_change_event_trigger_scan();

/* trigger scan: event allocation  */
EVENT_QUEUE *
ca_alloc_event_trigger_scan(int size, int nonames)
{
    int i;
    EVENT_QUEUE *pevent;
    pevent = (EVENT_QUEUE *) calloc(1, sizeof(EVENT_QUEUE));
    pevent->pvalues = (double *) calloc(nonames * size + 1, sizeof(double));
    pevent->maxvalues = size;
    pevent->numvalues = 0;
    pevent->overflow = nonames; /* this used for nonames */
    pevent->scan_names = (char *) calloc(nonames, MAX_STRING_SIZE);
    pevent->pscan = (char **) calloc(nonames, sizeof(char *));
    for (i = 0; i < nonames; i++)
    {
        pevent->pscan[i] = (char *) (pevent->scan_names + i * MAX_STRING_SIZE);
    }
    return (pevent);
}


/******************************************************
trigger scan queuing value change event callback 
******************************************************/
void
ca_monitor_value_change_event_trigger_scan(struct event_handler_args args)
{
    chandata *pchan;
    double time;
    int num, nonames;

    pchan = (chandata *) args.usr;

#ifdef ACCESS_SECURITY
    if (args.status == ECA_NORMAL)
    {
#endif

        if (CA.devprflag > 1)
            fprintf(stderr, "Old: name=%s, value=%f, stat=%d, sevr=%d, event=%d\n",
                    ca_name(pchan->chid),
                    pchan->value, pchan->status, pchan->severity, pchan->event);
        pchan->value = ((struct dbr_time_double *) args.dbr)->value;
        pchan->status = ((struct dbr_time_double *) args.dbr)->status;
        pchan->severity = ((struct dbr_time_double *) args.dbr)->severity;
        pchan->event = 3;

        pchan->stamp = ((struct dbr_time_double *) args.dbr)->stamp;
        if (CA.devprflag > 1)
        {
            time = iocClockTime(&pchan->stamp);
            fprintf(stderr, "New: name=%s, value=%f, stat=%d, sevr=%d, event=%d\n",
                    ca_name(pchan->chid),
                    pchan->value, pchan->status, pchan->severity, pchan->event);


        }

#ifdef ACCESS_SECURITY
    }
    else
        fprintf(stderr, "Diagnostic: Read access denied : %s\n", ca_name(pchan->chid));
#endif

    if (pchan->p_event_queue->numvalues < pchan->p_event_queue->maxvalues)
    {

        /* get the trigger scan pvname's  values */

        nonames = pchan->p_event_queue->overflow;
        num = pchan->p_event_queue->numvalues * nonames;
        ca_monitor_get_value_array(nonames, pchan->p_event_queue->pscan,
                                   (pchan->p_event_queue->pvalues + num));

        /* increase the num */

        pchan->p_event_queue->numvalues += 1;
    }
}



/******************************************************
 trigger scan : add the  monitor list only double value, return CA.CA_ERR
******************************************************/
int
ca_monitor_add_event_trigger_array_scan(chandata * pchandata, int npts, int nonames, char **pvnames)
{
int i=0,status=0,command_error=0;

    pchandata->type = ca_field_type(pchandata->chid);
    if (pchandata->state != cs_conn)
        command_error = CA_FAIL;
    else if (pchandata->evid == NULL)
    {

        pchandata->p_event_queue = (EVENT_QUEUE *) ca_alloc_event_trigger_scan(npts, nonames);

        /*  add the scan record monitoring */

        for (i = 0; i < nonames; i++)
            sprintf(pchandata->p_event_queue->pscan[i], "%s", pvnames[i]);

        ca_monitor_add_event_array(nonames, pchandata->p_event_queue->pscan);
        if (CA.CA_ERR == CA_FAIL)
            fprintf(stderr, "Error: trigger scan monitor 2 failed\n");

        status = ca_add_masked_array_event(DBR_TIME_DOUBLE, 0,
                                           pchandata->chid,
                                           ca_monitor_value_change_event_trigger_scan,
                                           pchandata,
                                           (float) 0, (float) 0, (float) 0,
                                           &(pchandata->evid), DBE_VALUE | DBE_ALARM);

    }

    /* automatically add a change connection event */

    ca_connect_add_event(pchandata);

    ca_check_return_code(status);

    ca_pend_event(CA.PEND_EVENT_TIME);

    return (CA.CA_ERR);
}


/******************************************************
 trigger scan:  clear the trigger monitor list , return CA.CA_ERR 
******************************************************/
int
ca_monitor_clear_event_trigger_array_scan(int noName, char **pvName)
{
    int status, command_error = 0;
    chandata *list, *snode, *pchan;
    int nonames;

    command_error = ca_pvlist_search(noName, pvName, &list);

    snode = list;
    while (snode)
    {
        pchan = snode;
        pchan->type = ca_field_type(pchan->chid);
        if (pchan->state != cs_conn)
            command_error = CA_FAIL;
        else if (pchan->evid)
        {
            nonames = pchan->p_event_queue->overflow;
            ca_monitor_clear_event_array(nonames, pchan->p_event_queue->pscan);
            if (CA.CA_ERR == CA_FAIL)
                fprintf(stderr, "Error: trigger scan clear event 2 failed.\n");
            status = ca_clear_event(pchan->evid);
            pchandata = pchan;
            ca_check_return_code(status);
            pchan->evid = NULL;
            pchan->event = 0;
            free(pchan->p_event_queue->pvalues);
            free(pchan->p_event_queue->scan_names);
            free(pchan->p_event_queue->pscan);
            free(pchan->p_event_queue);
            pchan->p_event_queue = NULL;
        }
        snode = snode->next;
    }

    ca_check_command_error(command_error);

    ca_pend_event(CA.PEND_EVENT_TIME);

    return (CA.CA_ERR);
}


/******************************************************
trigger scan :  get event values for monitored pvs, return CA.CA_ERR
******************************************************/
int
ca_monitor_get_value_trigger_scan(char *pvName, double *vals)
{
    int i, command_error = 0, num, ii = 0;
    evid temp_evid;
    chandata *pchan;
    int nonames;

    num = CA_FAIL;
    ca_pend_event(0.00001);

    ca_find_dev(pvName, pchandata);

    /* take care the racing problem  for scan record */

    while (strcmp(pvName, ca_name(pchandata->chid)) != 0)
    {
        ca_find_dev(pvName, pchandata);
    }
    pchan = pchandata;

    pchan->type = ca_field_type(pchan->chid);
    temp_evid = pchan->evid;
    if (pchan->state != cs_conn)
        command_error = CA_FAIL;
    else if (temp_evid)
    {
        if (pchan->p_event_queue->numvalues > 0)
        {
            nonames = pchan->p_event_queue->overflow;
            ii = nonames * pchan->p_event_queue->numvalues;
            for (i = 0; i < ii; i++)
            {
                *(vals + i) = *(pchan->p_event_queue->pvalues + i);
            }
        }

        num = pchan->p_event_queue->numvalues;

    }
    else
    {
        if (CA.devprflag >= 0)
            fprintf(stderr, "Error: %s is not monitored yet.\n", ca_name(pchan->chid));
    }
    ca_check_command_error(command_error);

    return (num);
}

/******************************************************
trigger scan :  zero a monitor queue list , return CA.CA_ERR 
******************************************************/
int
ca_monitor_zero_event_trigger_array_scan(int noName, char **pvName)
{
    int i, command_error = 0, size;
    chandata *list, *snode, *pchan;
    int nonames;
    command_error = ca_pvlist_search(noName, pvName, &list);

    snode = list;
    while (snode)
    {
        pchan = snode;
        pchan->type = ca_field_type(pchan->chid);
        if (pchan->state != cs_conn)
            command_error = CA_FAIL;
        else if (pchan->evid)
        {
            size = pchan->p_event_queue->maxvalues;
            nonames = pchan->p_event_queue->overflow;
            for (i = 0; i < nonames * size; i++)
                pchan->p_event_queue->pvalues[i] = 0.;
            pchan->p_event_queue->numvalues = 0;
            pchan->event = 0;

        }
        snode = snode->next;
    }

    ca_check_command_error(command_error);

    ca_pend_event(CA.PEND_EVENT_TIME);

    return (CA.CA_ERR);
}
