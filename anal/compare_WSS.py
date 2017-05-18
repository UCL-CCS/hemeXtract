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

if len(sys.argv) != 10:
        sys.exit("Usage: python compare_WSS.py PLANE1XTR PLANE2XTR STEPLENGTH1 STEPLENGTH2 TIME SCALEA SCALEB MINEXISTENT OUTPUTFILENAME")

plane1 = sys.argv[1]
plane2 = sys.argv[2]
steplength1 = str(float(sys.argv[3]))
steplength2 = str(float(sys.argv[4]))
time = str(float(sys.argv[5]))
scaleA = str(float(sys.argv[6]))
scaleB = str(float(sys.argv[7]))
minexistent = str(int(sys.argv[8]))
outputfilename = sys.argv[9]

hemeXtract = "~/hemeXtract/hemeXtract"

tempout = '/tmp/__hemeXtract_tmp_WSS_out.txt'


# Extract/calc the cross sectional error using hemeXtract
execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 + " -1 " + time + " --scaleA " + scaleA + " --scaleB " + scaleB + " --minexistent " + minexistent + " -n 1 --quiet -o " + tempout + "\n")

# Get the min and max WSS
WSStxt = genfromtxt(tempout)

minWSS = np.min(WSStxt[:3])
maxWSS = np.max(WSStxt[:3])

print "minWSS", minWSS
print "maxWSS", maxWSS

# Render with paraview
execute("pvpython ~/hemeXtract/anal/paraview_WSS.py " + tempout + " " + outputfilename + " " + " ".join([str(minWSS), str(maxWSS), "800", "800"]) + "\n")
