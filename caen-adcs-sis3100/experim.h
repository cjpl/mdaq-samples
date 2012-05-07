/********************************************************************\

  Name:         experim.h
  Created by:   ODBedit program

  Contents:     This file contains C structures for the "Experiment"
                tree in the ODB and the "/Analyzer/Parameters" tree.

                Additionally, it contains the "Settings" subtree for
                all items listed under "/Equipment" as well as their
                event definition.

                It can be used by the frontend and analyzer to work
                with these information.

                All C structures are accompanied with a string represen-
                tation which can be used in the db_create_record function
                to setup an ODB structure which matches the C structure.

  Created on:   Mon Sep 26 16:49:53 2011

\********************************************************************/

#ifndef EXCL_EXPCVADC

#define EXPCVADC_SETTINGS_DEFINED

typedef struct {
  struct {
    BOOL      enable;
    DWORD     address;
    INT       mode;
    WORD    threshold[16];
  } v785n_1;
  struct {
    BOOL      enable;
    DWORD     address;
    INT       mode;
    WORD      threshold[16];
  } v785n_2;
  struct {
    BOOL      enable;
    DWORD     address;
    INT       mode;
    WORD      threshold[16];
  } v785n_3;
  struct {
    BOOL      enable;
    DWORD     address;
    INT       mode;
    WORD      threshold[16];
  } v785n_4;
  struct {
    BOOL      enable;
    DWORD     address;
  } v560_1;
} EXPCVADC_SETTINGS;

#define EXPCVADC_SETTINGS_STR(_name) const char *_name[] = {\
"[V785N_1]",\
"Enable = BOOL : y",\
"Address = DWORD : 2019295232",\
"Mode = INT : 1",\
"Threshold = WORD[16] :",\
"[0] 8",\
"[1] 8",\
"[2] 8",\
"[3] 8",\
"[4] 8",\
"[5] 8",\
"[6] 8",\
"[7] 8",\
"[8] 8",\
"[9] 8",\
"[10] 8",\
"[11] 8",\
"[12] 8",\
"[13] 8",\
"[14] 8",\
"[15] 8",\
"",\
"[V785N_2]",\
"Enable = BOOL : n",\
"Address = DWORD : 0",\
"Mode = INT : 0",\
"Threshold = WORD[16] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"",\
"[V785N_3]",\
"Enable = BOOL : n",\
"Address = DWORD : 2019426304",\
"Mode = INT : 1",\
"Threshold = WORD[16] :",\
"[0] 8",\
"[1] 8",\
"[2] 8",\
"[3] 8",\
"[4] 8",\
"[5] 8",\
"[6] 8",\
"[7] 8",\
"[8] 8",\
"[9] 8",\
"[10] 8",\
"[11] 8",\
"[12] 8",\
"[13] 8",\
"[14] 8",\
"[15] 8",\
"",\
"[V785N_4]",\
"Enable = BOOL : n",\
"Address = DWORD : 0",\
"Mode = INT : 0",\
"Threshold = WORD[16] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"",\
"[V560_1]",\
"Enable = BOOL : n",\
"Address = DWORD : 1443823616",\
"",\
NULL }

#define EXPCVADC_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} EXPCVADC_COMMON;

#define EXPCVADC_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 3",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 2",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 257",\
"Period = INT : 1",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] daq-ntof",\
"Frontend name = STRING : [32] Frontend using CAEN ADCs",\
"Frontend file name = STRING : [256] cvadcfe.c",\
"Status = STRING : [256] Frontend using CAEN ADCs@daq-ntof",\
"Status color = STRING : [32] #00FF00",\
"",\
NULL }

#endif

#ifndef EXCL_SCALER

#define SCALER_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
} SCALER_COMMON;

#define SCALER_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 4",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 33",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 377",\
"Period = INT : 2000",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] daq-ntof",\
"Frontend name = STRING : [32] Frontend using CAEN ADCs",\
"Frontend file name = STRING : [256] cvadcfe.c",\
"Status = STRING : [256] Frontend using CAEN ADCs@daq-ntof",\
"Status color = STRING : [32] #00FF00",\
"",\
NULL }

#endif

