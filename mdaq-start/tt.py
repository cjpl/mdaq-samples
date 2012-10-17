#!/usr/bin/env python
# mode: python; coding: utf-8;
from __future__ import print_function
import os, sys
import subprocess as sp

sh_script = '''#!/bin/bash
export PATH={path}:$PATH
export MIDASSYS={msys}
export MIDAS_EXPTAB={mtab}
export MIDAS_DIR={mdir}
PWD={cwd} {cli}
'''

shstr = sh_script.format(path='/usr/lib/mdaq/bin',
                 msys='/usr/lib/mdaq',
                 mtab='./exptab',
                 mdir='',
                 cwd='./',
                 cli='/usr/lib/mdaq/bin/echo-env')
print(shstr)
os.system("/bin/bash <<EOF\n%s\nEOF"%shstr)

