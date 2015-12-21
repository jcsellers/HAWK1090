#!/usr/bin/env python
import unittest
import ossie.utils.testing
import os
from omniORB import any
from ossie.utils import sb
import time
import filecmp
import os
import hashlib

case1 = True

case1_pass=False
if case1:
        print "executing case 1"
        sb.reset()
        decoder=sb.launch("ADSBdecoder_MR")
        reader=sb.launch("FileReader")

    #####configure the properties####
        reader.configure({'sample_rate':'2Msps','file_format': 'SCALAR OCTET (8o)', 'source_uri':'/home/redhawk/git/ADSBdecoder_MR/tests/testfiles/modes1.bin'})
        decoder.configure({'InteractiveMode':1,'StatsMode':1,'NetMode':0})
        
####do the connections
        reader.connect(decoder)
        sb.start()
        reader.configure({'playback_state':'PLAY'})
      #  time.sleep(0.25)
        
        time.sleep(3)
        expected = 460 #note there is a bug where the input buffer size is hardcoded and discards samples when receiveing from file. But for our purposes 460 works. 
        actual = decoder.PreambleCount
        if actual == expected:
            case1_pass=True
        else: 
            print "failed....."
            print actual 
        time.sleep(3)
        
time.sleep(3)
print "results...."
print "Case 1 Pass..." + str(case1_pass)
