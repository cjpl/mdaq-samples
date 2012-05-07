#-*- mode: org -*-
#+TITLE: DAQ code using CAEN ADCs (CAEN V785N)
#+AUTHOR: Exaos Lee <exaos@ciae.ac.cn>, +86 10 6935 7544

* Purpose
  As title mentioned.

* Description
** ODB structure
#+BEGIN_EXAMPLE
[local:expcvadc:S]/>ls -lh Equipment/expcvadc/Common/
Key name                        Type    #Val  Size  Last Opn Mode Value
---------------------------------------------------------------------------
Event ID                        WORD    1     2     4m   0   RWD  0x3
Trigger Mask                    WORD    1     2     4m   0   RWD  0x0
Buffer                          STRING  1     32    4m   0   RWD  SYSTEM
Type                            INT     1     4     4m   0   RWD  0x2
Source                          INT     1     4     4m   0   RWD  0x0
Format                          STRING  1     8     4m   0   RWD
Enabled                         BOOL    1     4     4m   0   RWD  y
Read on                         INT     1     4     4m   0   RWD  0x101
Period                          INT     1     4     4m   0   RWD  0x1
Event limit                     DOUBLE  1     8     4m   0   RWD  0
Num subevents                   DWORD   1     4     4m   0   RWD  0x0
Log history                     INT     1     4     4m   0   RWD  0x0
Frontend host                   STRING  1     32    4m   0   RWD
Frontend name                   STRING  1     32    4m   0   RWD  Frontend using CAEN ADCs
Frontend file name              STRING  1     256   4m   0   RWD

[local:expcvadc:S]/>ls -lh Equipment/expcvadc/Settings/
Key name                        Type    #Val  Size  Last Opn Mode Value
---------------------------------------------------------------------------
V785N_1                         DIR
V785N_2                         DIR
V785N_3                         DIR
V785N_4                         DIR

[local:expcvadc:S]/>ls -lh Equipment/expcvadc/Settings/V785N_1
Key name                        Type    #Val  Size  Last Opn Mode Value
---------------------------------------------------------------------------
Enable                          BOOL    1     4     5m   0   RWD  y
Address                         DWORD   1     4     5m   0   RWD  0x78530000
Mode                            INT     1     4     5m   0   RWD  0x1
Threshold                       WORD    1     2     5m   0   RWD  0x8
#+END_EXAMPLE

** experim.h
   Generated using command "odbedit -e expcvadc -c @odbinit.txt".

   All commaonds are defined in *odbinit.txt*.

** Frontend -- cvadcfe
   Using *vadc_caen.h" .
** Analyzers
*** cvadcana
*** cvadcrana
** Misc

