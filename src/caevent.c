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


/* #include<sys/timeb.h>  not ansi C for solaris */
#define epicsExportSharedSymbols
#include <shareLib.h>

#include "chandata.h"

void *pfdctx;                   /*caddr_t pfdctx;  fdmgr context */

int event_timeout_id;
#define USEC_TIME_OUT 10000
static struct timeval timeout = { 0, USEC_TIME_OUT };

int CONN = 1;
extern chandata *pchandata;

extern int fdmgr_add_fd();
extern int fdmgr_clear_fd();
extern int fdmgr_init();
extern int fdmgr_add_timeout();
extern int fdmgr_clear_timeout();

extern double atof();

/******************************************************
  time stamp in seconds get from IOC 
******************************************************/
double
iocClockTime(stamp)
     TS_STAMP *stamp;
{
    char nowText[28];
    double time;

    if ((stamp->nsec + stamp->secPastEpoch) == 0)
        epicsTimeGetCurrent(stamp);

    time = 0.001 * (stamp->nsec / 1000000) + stamp->secPastEpoch;
    if (CA.devprflag > 2)
    {
        epicsTimeToStrftime(nowText, 28, "%m/%d/%y %H:%M:%S.%09f", stamp);
        fprintf(stderr, "epicsTimeToStrftime:%s", nowText);
        fprintf(stderr, " IOC time=%.3f sec\n", time);
    }

    return time;
}


/******************************************************
  get value callback instead of ca_get function
******************************************************/
void
ca_get_dbr_sts_double_callback(args)
     struct event_handler_args args;
{
    chandata *pchandata;

    pchandata = (chandata *) args.usr;

#ifdef ACCESS_SECURITY
    if (args.status == ECA_NORMAL)
    {
#endif

        if (CA.devprflag > 1)
            fprintf(stderr, "Old: name=%s, value=%f, stat=%d, sevr=%d, error=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->value, pchandata->status, pchandata->severity, pchandata->error);

        /* get new value */

        pchandata->value = ((struct dbr_sts_double *) args.dbr)->value;
        pchandata->status = ((struct dbr_sts_double *) args.dbr)->status;
        pchandata->severity = ((struct dbr_sts_double *) args.dbr)->severity;
        pchandata->error = 0;

        if (CA.devprflag > 1)
            fprintf(stderr, "New: name=%s, value=%f, stat=%d, sevr=%d, error=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->value, pchandata->status, pchandata->severity, pchandata->error);

#ifdef ACCESS_SECURITY
    }
    else
        fprintf(stderr, "Diagnostic: Read access denied : %s\n", ca_name(pchandata->chid));
#endif
}



/******************************************************
  get value callback instead of ca_get function
******************************************************/
void
ca_get_dbr_sts_string_callback(args)
     struct event_handler_args args;
{
    chandata *pchandata;

    pchandata = (chandata *) args.usr;

#ifdef ACCESS_SECURITY
    if (args.status == ECA_NORMAL)
    {
#endif

        if (CA.devprflag > 1)
            fprintf(stderr, "Old: name=%s, value=%s, stat=%d, sevr=%d, error=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->string, pchandata->status, pchandata->severity, pchandata->error);

        /* get new value */

        strcpy(pchandata->string, ((struct dbr_sts_string *) args.dbr)->value);
        pchandata->status = ((struct dbr_sts_string *) args.dbr)->status;
        pchandata->severity = ((struct dbr_sts_string *) args.dbr)->severity;
        pchandata->value = atof(pchandata->string);
        pchandata->error = 0;

        if (CA.devprflag > 1)
            fprintf(stderr, "New: name=%s, value=%s, stat=%d, sevr=%d, error=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->string, pchandata->status, pchandata->severity, pchandata->error);
#ifdef ACCESS_SECURITY
    }
    else
        fprintf(stderr, "Diagnostic: Read access denied : %s\n", ca_name(pchandata->chid));
#endif

}

/******************************************************
  get dbr_ctrl_double_callback instead of ca_get function
******************************************************/
void
ca_get_dbr_ctrl_double_callback(args)
     struct event_handler_args args;
{
    chandata *pchandata;
    pchandata = (chandata *) args.usr;

#ifdef ACCESS_SECURITY
    if (args.status == ECA_NORMAL)
    {
#endif

        if (CA.devprflag > 1)
            fprintf(stderr, "Old: name=%s, value=%f, stat=%d, sevr=%d, error=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->value, pchandata->status, pchandata->severity, pchandata->error);

        /* get new value */

        pchandata->value = ((struct dbr_ctrl_double *) args.dbr)->value;
        pchandata->uopr = (float) ((struct dbr_ctrl_double *) args.dbr)->upper_disp_limit;
        pchandata->lopr = (float) ((struct dbr_ctrl_double *) args.dbr)->lower_disp_limit;
        pchandata->upper_alarm_limit =
            (float) ((struct dbr_ctrl_double *) args.dbr)->upper_alarm_limit;
        pchandata->upper_warning_limit =
            (float) ((struct dbr_ctrl_double *) args.dbr)->upper_warning_limit;
        pchandata->lower_warning_limit =
            (float) ((struct dbr_ctrl_double *) args.dbr)->lower_warning_limit;
        pchandata->lower_alarm_limit =
            (float) ((struct dbr_ctrl_double *) args.dbr)->lower_alarm_limit;
        pchandata->upper_ctrl_limit =
            (float) ((struct dbr_ctrl_double *) args.dbr)->upper_ctrl_limit;
        pchandata->lower_ctrl_limit =
            (float) ((struct dbr_ctrl_double *) args.dbr)->lower_ctrl_limit;
        pchandata->status = ((struct dbr_ctrl_double *) args.dbr)->status;
        pchandata->severity = ((struct dbr_ctrl_double *) args.dbr)->severity;
        strcpy(pchandata->units, ((struct dbr_ctrl_double *) args.dbr)->units);
        if (pchandata->largest < ((struct dbr_ctrl_double *) args.dbr)->value)
            pchandata->largest = (float) ((struct dbr_ctrl_double *) args.dbr)->value;
        if (pchandata->smallest > ((struct dbr_ctrl_double *) args.dbr)->value)
            pchandata->smallest = (float) ((struct dbr_ctrl_double *) args.dbr)->value;
        pchandata->error = 0;

        if (CA.devprflag > 1)
        {
            fprintf(stderr, "New: name=%s, value=%f, stat=%d, sevr=%d, error=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->value, pchandata->status, pchandata->severity, pchandata->error);
            fprintf(stderr, "\tprecision=%d\n", ((struct dbr_ctrl_double *) args.dbr)->precision);
            fprintf(stderr, "\tRISC_pad0=%d\n", ((struct dbr_ctrl_double *) args.dbr)->RISC_pad0);
            fprintf(stderr, "\tunits=%s\n", pchandata->units);
            fprintf(stderr, "\tdisp_limit : [ %f : %f ]\n", pchandata->uopr, pchandata->lopr);
            fprintf(stderr, "\tctrl_limit : [ %f : %f ]\n",
                    ((struct dbr_ctrl_double *) args.dbr)->lower_ctrl_limit,
                    ((struct dbr_ctrl_double *) args.dbr)->upper_ctrl_limit);
            fprintf(stderr, "\tupper_alarm_limit=%f\n", pchandata->upper_alarm_limit);
            fprintf(stderr, "\tupper_warning_limit=%f\n", pchandata->upper_warning_limit);
            fprintf(stderr, "\tlower_warning_limit=%f\n", pchandata->lower_warning_limit);
            fprintf(stderr, "\tlower_alarm_limit=%f\n", pchandata->lower_alarm_limit);
            fprintf(stderr, "\tvalue=%f\n", pchandata->value);
            fprintf(stderr, "\tstatus=%d\n", pchandata->status);
            fprintf(stderr, "\tseverity=%d\n", pchandata->severity);
        }

#ifdef ACCESS_SECURITY
    }
    else
        fprintf(stderr, "Diagnostic: Read access denied : %s\n", ca_name(pchandata->chid));
#endif

}



/******************************************************
  add a monitor list
******************************************************/
void
ca_monitor_add_event_array(noName, pvName)
     int noName;
     char **pvName;
{
    int status, command_error = 0;
    chandata *list, *snode, *pchan;

    command_error = ca_pvlist_search(noName, pvName, &list);

    snode = list;
    while (snode)
    {
        pchan = snode;
        pchan->type = ca_field_type(pchan->chid);
        if (pchan->state != cs_conn)
            command_error = CA_FAIL;
        else if (pchan->evid == NULL)
        {

            if (ca_field_type(pchan->chid) == DBR_STRING || ca_field_type(pchan->chid) == DBR_ENUM)
                status = ca_add_masked_array_event(DBR_TIME_STRING, 0,
                                                   pchan->chid,
                                                   ca_monitor_string_change_event,
                                                   pchan,
                                                   (float) 0, (float) 0, (float) 0,
                                                   &(pchan->evid), DBE_VALUE | DBE_ALARM);

            else
                status = ca_add_masked_array_event(DBR_TIME_DOUBLE, 0,
                                                   pchan->chid,
                                                   ca_monitor_value_change_event,
                                                   pchan,
                                                   (float) 0, (float) 0, (float) 0,
                                                   &(pchan->evid), DBE_VALUE | DBE_ALARM);

            pchandata = pchan;
            ca_check_return_code(status);

            /* automatically add a change connection event */

            ca_connect_add_event(pchan);

            if (CA.devprflag > 0)
                fprintf(stderr, "name=%s, evid=%p\n", ca_name(pchan->chid), pchan->evid);
        }
        snode = snode->next;
    }

    ca_check_command_error(command_error);

    ca_pend_event(CA.PEND_EVENT_TIME);

}

/******************************************************
  clear a monitor list
******************************************************/
void
ca_monitor_clear_event_array(noName, pvName)
     int noName;
     char **pvName;
{
    int status, command_error = 0;
    chandata *list, *snode, *pchan;

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
            status = ca_clear_event(pchan->evid);
            pchandata = pchan;
            ca_check_return_code(status);
            pchan->evid = NULL;
        }
        snode = snode->next;
    }

    ca_check_command_error(command_error);

    ca_pend_event(CA.PEND_EVENT_TIME);
}


/******************************************************
  add a monitor channel 
******************************************************/
void
ca_monitor_add_event(pchandata)
     chandata *pchandata;
{
    void ca_monitor_value_change_event();
    int status;

    if (pchandata->evid == NULL)
    {
        if (ca_field_type(pchandata->chid) == DBR_STRING ||
            ca_field_type(pchandata->chid) == DBR_ENUM)
            status = ca_add_masked_array_event(DBR_TIME_STRING, 0,
                                               pchandata->chid,
                                               ca_monitor_string_change_event,
                                               pchandata,
                                               (float) 0, (float) 0, (float) 0,
                                               &(pchandata->evid), DBE_VALUE | DBE_ALARM);

        else
            status = ca_add_masked_array_event(DBR_TIME_DOUBLE, 0,
                                               pchandata->chid,
                                               ca_monitor_value_change_event,
                                               pchandata,
                                               (float) 0, (float) 0, (float) 0,
                                               &(pchandata->evid), DBE_VALUE | DBE_ALARM);

        ca_check_return_code(status);

/* automatically add a change connection event */

        ca_connect_add_event(pchandata);

        if (CA.PEND_EVENT_ON)
            ca_pend_event(0.00001);
    }
}

/******************************************************
  clear  a monitor channel 
******************************************************/
void
ca_monitor_clear_event(pchandata)
     chandata *pchandata;
{
    int status;

    if (pchandata->evid)
    {
        status = ca_clear_event(pchandata->evid);
        ca_check_return_code(status);
        pchandata->evid = NULL;
    }
    if (CA.PEND_EVENT_ON)
        ca_pend_event(0.00001);
}

/************************************************************
  event been monitored event =0, new event =1, 
 ***********************************************************/
int
ca_monitor_get_event(pchandata)
     chandata *pchandata;
{
    int event = 0;

    if (CA.PEND_EVENT_ON)
        ca_pend_event(0.00001);

    if (pchandata->evid)
    {
        if (pchandata->event)
        {
            if (CA.devprflag > 0)
                fprintf(stderr, "caEventMonitor: name=%s, value=%f, status=%d, event=%d\n",
                        ca_name(pchandata->chid), pchandata->value,
                        pchandata->status, pchandata->event);
            event = pchandata->event;
            pchandata->event = 0;
        }
    }
    else
    {
        if (CA.devprflag >= 0)
            fprintf(stderr, "Error: %s is not monitored yet.\n", ca_name(pchandata->chid));
    }
    return (event);
}


/******************************************************
  value change event callback 
******************************************************/
void
ca_monitor_value_change_event(args)
     struct event_handler_args args;
{
    chandata *pchandata;
    double time;

    pchandata = (chandata *) args.usr;

#ifdef ACCESS_SECURITY
    if (args.status == ECA_NORMAL)
    {
#endif

        if (CA.devprflag > 1)
            fprintf(stderr, "Old: name=%s, value=%f, stat=%d, sevr=%d, event=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->value, pchandata->status, pchandata->severity, pchandata->event);
        pchandata->value = ((struct dbr_time_double *) args.dbr)->value;
        pchandata->status = ((struct dbr_time_double *) args.dbr)->status;
        pchandata->severity = ((struct dbr_time_double *) args.dbr)->severity;
        pchandata->event = 1;
        pchandata->stamp = ((struct dbr_time_double *) args.dbr)->stamp;
        if (CA.devprflag > 1)
        {
            time = iocClockTime(&pchandata->stamp);
            fprintf(stderr, "New: name=%s, value=%f, stat=%d, sevr=%d, event=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->value, pchandata->status, pchandata->severity, pchandata->event);


        }
#ifdef ACCESS_SECURITY
    }
    else
        fprintf(stderr, "Diagnostic: Read access denied : %s\n", ca_name(pchandata->chid));
#endif
}

/******************************************************
  string change event callback 
******************************************************/
void
ca_monitor_string_change_event(args)
     struct event_handler_args args;
{
    chandata *pchandata;
    double time;

    pchandata = (chandata *) args.usr;

#ifdef ACCESS_SECURITY
    if (args.status == ECA_NORMAL)
    {
#endif

        if (CA.devprflag > 1)
            fprintf(stderr, "OldString: name=%s, value=%s, stat=%d, sevr=%d, event=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->string, pchandata->status, pchandata->severity, pchandata->event);
        strcpy(pchandata->string, ((struct dbr_time_string *) args.dbr)->value);
        pchandata->status = ((struct dbr_time_string *) args.dbr)->status;
        pchandata->severity = ((struct dbr_time_string *) args.dbr)->severity;
        pchandata->event = 1;
        pchandata->stamp = ((struct dbr_time_string *) args.dbr)->stamp;
        if (CA.devprflag > 1)
        {
            time = iocClockTime(&pchandata->stamp);
            fprintf(stderr, "NewString: name=%s, value=%s, stat=%d, sevr=%d, event=%d\n",
                    ca_name(pchandata->chid),
                    pchandata->string, pchandata->status, pchandata->severity, pchandata->event);
        }

        if (pchandata->type != DBR_STRING)
            ca_get_callback(DBR_STS_DOUBLE, pchandata->chid,
                            ca_get_dbr_sts_double_callback, pchandata);
#ifdef ACCESS_SECURITY
    }
    else
        fprintf(stderr, "Diagnostic: Read access denied : %s\n", ca_name(pchandata->chid));
#endif
}

/******************************************************
  process event id for a monitored list and reset to 0
******************************************************/
void
ca_monitor_get_event_array(noName, pvName, vals)
     int noName;
     char **pvName;
     int *vals;
{
    int i, command_error = 0;
    chandata *list, *snode, *pchan;

    ca_pend_event(0.00001);

    command_error = ca_pvlist_search(noName, pvName, &list);

    snode = list;
    i = 0;
    while (snode)
    {
        pchan = snode;
        pchan->type = ca_field_type(pchan->chid);
        if (pchan->state != cs_conn)
            command_error = CA_FAIL;
        else if (pchan->evid)
        {
            if (CA.devprflag > 0)
                fprintf(stderr, "caEventMonitorList: name=%s, value=%f, status=%d, event=%d\n",
                        ca_name(pchan->chid), pchan->value, pchan->status, pchan->event);

            *(vals + i) = pchan->event;
            if (pchan->event)
                pchan->event = 0;       /* reset after read */

        }
        else
        {
            if (CA.devprflag >= 0)
                fprintf(stderr, "Error: %s is not monitored yet.\n", ca_name(pchan->chid));
        }
        snode = snode->next;
        i++;
    }

    ca_check_command_error(command_error);
}


/******************************************************
  get event values for a monitored list
******************************************************/
void
ca_monitor_get_value_array(noName, pvName, vals)
     int noName;
     char **pvName;
     double *vals;
{
    int i, command_error = 0;
    chandata *list, *snode, *pchan;

    ca_pend_event(0.00001);

    command_error = ca_pvlist_search(noName, pvName, &list);

    snode = list;
    i = 0;
    while (snode)
    {
        pchan = snode;
        pchan->type = ca_field_type(pchan->chid);
        if (pchan->state != cs_conn)
            command_error = CA_FAIL;
        else if (pchan->evid)
        {
            if (CA.devprflag > 0)
                fprintf(stderr, "caGetMonitorValueList: name=%s, value=%f, status=%d, event=%d\n",
                        ca_name(pchan->chid), pchan->value, pchan->status, pchan->event);

            *(vals + i) = pchan->value;
            if (pchan->event)
                pchan->event = 0;       /* reset after read */
        }
        else
        {
            if (CA.devprflag >= 0)
                fprintf(stderr, "Error: %s is not monitored yet.\n", ca_name(pchan->chid));
        }
        snode = snode->next;
        i++;
    }

    ca_check_command_error(command_error);
}



/******************************************************
  get event value, status and severity  for a monitored list
******************************************************/
void
ca_monitor_get_all_array(noName, pvName, vals)
     int noName;
     char **pvName;
     double *vals;
{
    int i, command_error = 0;
    chandata *list, *snode, *pchan;

    ca_pend_event(0.00001);

    command_error = ca_pvlist_search(noName, pvName, &list);

    snode = list;
    i = 0;
    while (snode)
    {
        pchan = snode;
        pchan->type = ca_field_type(pchan->chid);
        if (pchan->state != cs_conn)
            command_error = CA_FAIL;
        else if (pchan->evid)
        {
            if (CA.devprflag > 0)
                fprintf(stderr, "caGetMonitorList: name=%s, value=%f, status=%d, event=%d\n",
                        ca_name(pchan->chid), pchan->value, pchan->status, pchan->event);

            *(vals + i * 3) = pchan->value;
            *(vals + i * 3 + 1) = pchan->status;
            *(vals + i * 3 + 2) = pchan->severity;
            if (pchan->event)
                pchan->event = 0;       /* reset after read */
        }
        else
        {
            if (CA.devprflag >= 0)
                fprintf(stderr, "Error: %s is not monitored yet.\n", ca_name(pchan->chid));
        }
        snode = snode->next;
        i++;
    }

    ca_check_command_error(command_error);
}


/******************************************************
 * check for single event for a monitored channel: return 0 or -1
 *      -1   no event happened
 *      0    at least one value change event happened
******************************************************/
int
ca_monitor_check_event(name)
     char *name;
{
    int EVENT = CA_FAIL;
    int command_error = 0;
    chandata *pchan;

    if (CA.PEND_EVENT_ON)
        ca_pend_event(0.00001);

    ca_find_dev(name, pchandata);
    while (strcmp(name, ca_name(pchandata->chid)) != 0)
    {
        ca_find_dev(name, pchandata);
    }
    pchan = pchandata;

    if (pchan->state != cs_conn)
        command_error = CA_FAIL;

    else if (pchan->evid)
    {
        if (pchan->event)
        {
            EVENT = 0;
            if (CA.devprflag > 0)
                fprintf(stderr, "caWaitEventMonitor: name=%s, value=%f, status=%d, event=%d\n",
                        ca_name(pchan->chid), pchan->value, pchan->status, pchan->event);
        }
    }
    else
    {
        if (CA.devprflag >= 0)
            fprintf(stderr, "Error: %s is not monitored yet.\n", ca_name(pchan->chid));
    }

    ca_check_command_error(command_error);
    return (EVENT);
}

/******************************************************
 * check any event occurs  for a monitored list : return 0 or -1
 *      -1   no event happened
 *      0    at least one value change event happened
******************************************************/
int
ca_monitor_check_event_array(noName, pvName, vals)
     int noName;
     char **pvName;
     int *vals;
{
    int EVENT = CA_FAIL;
    int i, command_error = 0;
    chandata *list, *snode, *pchan;

    ca_pend_event(0.00001);

    command_error = ca_pvlist_search(noName, pvName, &list);

    snode = list;
    i = 0;
    while (snode)
    {
        pchan = snode;
        pchan->type = ca_field_type(pchan->chid);
        if (pchan->state != cs_conn)
            command_error = CA_FAIL;
        else if (pchan->evid)
        {
            if (pchan->event)
            {
                EVENT = 0;
                if (CA.devprflag > 0)
                    fprintf(stderr,
                            "caWaitEventMonitorList: name=%s, value=%f, status=%d, event=%d\n",
                            ca_name(pchan->chid), pchan->value, pchan->status, pchan->event);

                *(vals + i) = pchan->event;
            }
        }
        else
        {
            if (CA.devprflag >= 0)
                fprintf(stderr, "Error: %s is not monitored yet.\n", ca_name(pchan->chid));
        }
        snode = snode->next;
        i++;
    }

    ca_check_command_error(command_error);
    return (EVENT);
}


/****************************************************
 *  change connection event callback 
 ****************************************************/
void
ca_connect_change_event(args)
     struct connection_handler_args args;
{
    chandata *pchandata;

    pchandata = (chandata *) ca_puser(args.chid);
    if (pchandata->state == cs_conn)
    {
        fprintf(stderr, "****Connection lost happened on %s ****\n", ca_name(pchandata->chid));
        pchandata->state = cs_prev_conn;
    }
    else
    {
        fprintf(stderr, "****Reconnection established on %s ****\n", ca_name(pchandata->chid));
        pchandata->state = cs_conn;
    }

}



/****************************************************
 *  add connection event for a channel
****************************************************/
void
ca_connect_add_event(pchandata)
     chandata *pchandata;
{
    void ca_connect_change_event();

    ca_set_puser(pchandata->chid, pchandata);
    ca_change_connection_event(pchandata->chid, ca_connect_change_event);
}



/**************************************************
   process ca event queue
*****************************************************/
void
ca_process_CA()
{
    ca_pend_event(CA.PEND_EVENT_TIME);
}



static void
registerCA(pfdctx, fd, condition)
     void *pfdctx;
     int fd;
     int condition;
{


    if (condition)
    {
        fdmgr_add_fd(pfdctx, fd, ca_process_CA, NULL);
    }
    else
    {
        fdmgr_clear_fd(pfdctx, fd);
    }
}


void
caInit()
{
    int status;
/*
 *  initialize channel access
 */
    status = ca_task_initialize();
    if (status != ECA_NORMAL)
        fprintf(stderr, "caInit: ca_task_initialize failed.\n");

/*
 *  initialize fdmgr
 */
    pfdctx = (void *) fdmgr_init();

    if (pfdctx == NULL)
        fprintf(stderr, "caInit: fdmgr_init failed.\n");

/*
 * and add CA fd to pfdctx's input stream...
 */
    status = ca_add_fd_registration(registerCA, pfdctx);
    if (status != ECA_NORMAL)
        fprintf(stderr, "caInit: ca_add_fd_registration failed.\n");

}

void caAddTimeout()
{
	fprintf(stderr,"caAddTimeout\n");
	event_timeout_id = fdmgr_add_timeout(pfdctx, &timeout,
		ca_flush_io(),NULL);
}

void
caClearTimeout()
{
	fprintf(stderr,"caClearTimeout\n");
	if (event_timeout_id) {
		fdmgr_clear_timeout(pfdctx, event_timeout_id);
	event_timeout_id = NULL;
	}
}

void
caClose()
{
    int status;

/* cancel registration of the CA file descriptors */

    status = ca_add_fd_registration(registerCA, pfdctx);
    if (status != ECA_NORMAL)
        fprintf(stderr, "caClose: ca_add_fd_registration failed.\n");

/* and close channel access */

    status = ca_task_exit();
    if (status != ECA_NORMAL)
        fprintf(stderr, "caClose: ca_task_exit failed\n");

}
