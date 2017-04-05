import os, sys
import numpy as np
import math
import matplotlib.pyplot as plt
from matplotlib import rcParams
from pylab import genfromtxt;

def execute(command):
	print "Executing: " + command
	r = os.system(command);
	if r != 0:
		sys.exit("Command failed.")
	
if len(sys.argv) != 7:
	sys.exit("Usage: python make_single_max_vel_plot.py PLANEXTR STEPLENGTH SCALE TSTART TEND OUTPUTFILENAME")

plane1 = sys.argv[1]
steplength1 = str(float(sys.argv[2]))
scaleA = str(float(sys.argv[3]))
timeStart = float(sys.argv[4])
timeEnd = float(sys.argv[5])
outputfilename = sys.argv[6]

hemeXtract = "~/hemeXtract/hemeXtract"

execute(hemeXtract + " -C " + plane1 + " " + plane1 + " -A " + steplength1 + " -B " + steplength1 +  " --scaleA " + scaleA + " --scaleB " + scaleA + " --time1=" + str(timeStart) + " --time2=" + str(timeEnd) + " --stats -o __hemeXtract_output\n")

# Make matplotlib graph
if "plane_" in plane1:
	planetit = plane1.split("plane_")[1]
elif "plane" in plane1:
	planetit = plane1.split("plane")[1]
elif "/" in plane1:
	planetit = plane1.split("/")[1]
title = planetit.replace(".xtr", "")

vel = genfromtxt("__hemeXtract_output");

rcParams['font.family'] = 'Times New Roman'
rcParams['font.size'] = 24

(_,caps1,_) = plt.errorbar(vel[:,0], vel[:,3], c='black', linewidth=2.5, linestyle='-', marker='')

plt.title(title)
plt.ylabel(r'Velocity (ms$^{-1}$)')
plt.xlabel(r'Time (s)')
plt.xlim(timeStart, timeEnd)

plt.tight_layout()

print "Saving figure..."
plt.savefig(outputfilename + ".png", dpi=800)
print "Done."
