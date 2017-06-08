# -*- coding: utf-8 -*-

import os, sys
import numpy as np
import math

import matplotlib.pyplot as plt
from matplotlib import rcParams
from pylab import genfromtxt

def execute(command):
        print "Executing: " + command
        r = os.system(command);
        if r != 0:
                sys.exit("Command failed.")

if len(sys.argv) != 11 and len(sys.argv) != 13:
        sys.exit("Usage: python compare_WSS.py PLANE1XTR PLANE2XTR STEPLENGTH1 STEPLENGTH2 TIME SCALEA SCALEB MINEXISTENT RELATIVE{T or F} OUTPUTFILENAME OPTIONAL{minWSS maxWSS}")

plane1 = sys.argv[1]
plane2 = sys.argv[2]
steplength1 = str(float(sys.argv[3]))
steplength2 = str(float(sys.argv[4]))
time = str(float(sys.argv[5]))
scaleA = str(float(sys.argv[6]))
scaleB = str(float(sys.argv[7]))
minexistent = str(int(sys.argv[8]))

relative = sys.argv[9]
if relative == 'T':
        relative = " --relativeErr "
elif relative == 'F':
        relative = ""
else:
        sys.exit("RELATIVE should be 'T' or 'F'")

outputfilename = sys.argv[10]

minWSS = None
maxWSS = None
if len(sys.argv) == 13:
	minWSS = float(sys.argv[11])
	maxWSS = float(sys.argv[12])

hemeXtract = "~/hemeXtract/hemeXtract"

tempout = '/tmp/__hemeXtract_tmp_WSS_out.txt'


# Extract/calc the cross sectional error using hemeXtract
execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 + " -1 " + time + " --scaleA " + scaleA + " --scaleB " + scaleB + " --minexistent " + minexistent + relative + " -n 1 --quiet -o " + tempout + "\n")

# Get the min and max WSS
if minWSS == None:
	WSStxt = genfromtxt(tempout)

	minWSS = np.min(WSStxt[:,3])
	maxWSS = np.max(WSStxt[:,3])

	print "minWSS", minWSS
	print "maxWSS", maxWSS

# Render with paraview
execute("pvpython ~/hemeXtract/anal/paraview_WSS.py " + tempout + " " + outputfilename + " " + " ".join([str(minWSS), str(maxWSS), "1000", "1000"]) + "\n")
