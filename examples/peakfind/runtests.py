#!/usr/bin/python

import sys
import re
import subprocess
#from agilent.devices import N5183 as SignalGenerator
from hittite.devices import T2240 as SignalGenerator

#wsa_ip = "10.126.110.121"
#siggen_ip = "10.126.110.19"
wsa_ip = "172.16.1.112"
siggen_ip = "172.16.1.82"
siggen_external_atten = 30

tests = [
   # mode, fstart, fstop, rbw, fin, pin

   # 100 MHz span, 50 kHz RBW
   [ "SHN", 2400e6, 2500e6, 50e3, 2403e6, 0 ], # left edge
   [ "SHN", 2400e6, 2500e6, 50e3, 2498e6, 0 ], # right edge
   [ "SHN", 2400e6, 2500e6, 50e3, 2450e6, 0 ], # center band

   # 4 MHz span, 10kHz RBW
   [ "SHN", 2400e6, 2405e6, 10e3, 2401e6, 0 ], # left edge
   [ "SHN", 2400e6, 2405e6, 10e3, 2402.5e6, 0 ], # center
   [ "SHN", 2400e6, 2405e6, 10e3, 2404e6, 0 ], # right edge

   # bottom edge, 30 kHz RBW
   [ "SHN", 50e6, 1000e6, 30e3, 51e6, 0 ], # left edge

   # top edge, 30 kHz RBW
   [ "SHN", 7500e6, 8000e6, 30e3, 7999e6, 0 ], # left edge

   # full band, 30 kHz RBW
   [ "SHN", 50e6, 8000e6, 30e3, 51e6, 0 ], # left edge
   [ "SHN", 50e6, 8000e6, 30e3, 4001e6, 0 ], # center edge
   [ "SHN", 50e6, 8000e6, 30e3, 7998e6, 0 ], # right edge

   # oddball frequencies
   [ "SHN", 2173.2e6, 2271.1e6, 40e3, 2199e6, 0 ], # oddball 1
   [ "SHN", 1001.7e6, 1999.9e6, 40e3, 1198.1e6, 0 ], # oddball 2

   # small input changes
   [ "SHN", 2400e6, 2500e6, 10e3, 2403000e3, 0 ],
   [ "SHN", 2400e6, 2500e6, 10e3, 2403004e3, 0 ],
   [ "SHN", 2400e6, 2500e6, 10e3, 2403007e3, 0 ],
   [ "SHN", 2400e6, 2500e6, 10e3, 2403009e3, 0 ],
   [ "SHN", 2400e6, 2500e6, 10e3, 2403010e3, 0 ],
   [ "SHN", 2400e6, 2500e6, 10e3, 2403011e3, 0 ],
   [ "SHN", 2400e6, 2500e6, 10e3, 2403015e3, 0 ],
   [ "SHN", 2400e6, 2500e6, 10e3, 2403017e3, 0 ],
   [ "SHN", 2400e6, 2500e6, 10e3, 2403020e3, 0 ],

   # larger RBWs
   [ "SHN", 50e6, 8000e6, 300e3, 7998e6, 0 ], 
   [ "SHN", 50e6, 8000e6, 1e6, 7998e6, 0 ], 
   [ "SHN", 50e6, 8000e6, 3e6, 7998e6, 0 ], 

    # smaller RBWs
    [ "SHN", 2400e6, 2450e6, 10e3, 2405e6, 0 ], 
    [ "SHN", 2400e6, 2450e6, 3e3, 2405e6, 0 ], 
    [ "SHN", 2400e6, 2450e6, 1e3, 2405e6, 0 ], 
    [ "SHN", 2400e6, 2450e6, 300, 2405e6, 0 ], 
    [ "SHN", 2400e6, 2450e6, 100, 2405e6, 0 ], 
]

# connect to siggen
siggen = SignalGenerator(siggen_ip)
siggen.amplitude(-30)
siggen.freq(2450e6)
siggen.output(1)

index = 0
for (mode, fstart, fstop, rbw, fin, pin) in tests:
        index += 1
        sys.stdout.write("test #%d -- sweep=(%s, %d, %d, %d) -- input=(%d, %d) = " % (index, mode, fstart, fstop, rbw, fin, pin))
        sys.stdout.flush()

	# tune siggen
	siggen.freq(fin)
	siggen.amplitude(pin)

	# perform peakfind
	#Peak #1, -30.83 dBm @ 902990666
        cmd = [
            "./peakfind", 
            "--clkref=ext",
            "--mode=%s" % mode,
            "--peaks=1",
            "--start=%u" % fstart, 
            "--stop=%u" % fstop, 
            "--rbw=%u" % rbw,
            "%s" % wsa_ip
        ]
	peaks = subprocess.check_output(cmd)

        pamp = float(re.sub(".*Peak #1, (.*) dBm @ .*", "\\1", peaks))
        pfreq = float(re.sub(".* @ (.*) Hz, .*", "\\1", peaks))
        arbw = float(re.sub(".*RBW = (.*) Hz", "\\1", peaks))

        # test freq of peak
        if abs(fin - pfreq) > (2 * arbw):
            print "FAIL: F(peak) = %d for test case %d" % (pfreq, index)

        # test amplitude
        elif abs(pin - pamp) > (10 + siggen_external_atten):
            print "FAIL: P(peak) = %f for test case %d" % (pamp, index)

        else:
            print "PASS -- peak=(%d, %0.2f, %0.2f)" % (pfreq, pamp, arbw)

