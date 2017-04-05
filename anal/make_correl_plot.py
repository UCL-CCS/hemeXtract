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
	
if len(sys.argv) != 11:
	sys.exit("Usage: python make_correl_plot.py PLANE1XTR PLANE2XTR STEPLENGTH1 STEPLENGTH2 SCALEA SCALEB TSTART TEND NORMVEC{'T' or 'F'} OUTPUTFILENAME")

plane1 = sys.argv[1]
plane2 = sys.argv[2]
steplength1 = str(float(sys.argv[3]))
steplength2 = str(float(sys.argv[4]))
scaleA = str(float(sys.argv[5]))
scaleB = str(float(sys.argv[6]))
timeStart = sys.argv[7]
timeEnd = sys.argv[8]
normvec = sys.argv[9]
if normvec == 'T':
	normvec_str = ' -N '
elif normvec == 'F':
	normvec_str = ''
else:
	sys.exit("NORMVEC should be T or F. Not '" + normvec  + "'.")
outputfilename = sys.argv[10]

hemeXtract = "~/hemeXtract/hemeXtract"

execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 +  " --scaleA " + scaleA + " --scaleB " + scaleB + " --time1=" + timeStart + " --time2=" + timeEnd + normvec_str + " --stats -o /tmp/__hemeXtract_output\n")

# Make matplotlib graph
if "plane_" in plane1:
	planetit = plane1.split("plane_")[1]
elif "plane" in plane1:
	planetit = plane1.split("plane")[1]
elif "/" in plane1:
	planetit = plane1.split("/")[1]
title = planetit.replace(".xtr", "") + " correlation"


correl = genfromtxt("/tmp/__hemeXtract_output");

rcParams['font.family'] = 'Times New Roman'
rcParams['font.size'] = 19

plt.errorbar(correl[:,0], correl[:,2], c='black', linewidth=1.2, linestyle='-', marker='')

print correl

legend=plt.legend(loc=1, numpoints=1)

plt.title(title)
plt.ylabel(r'Correl.')
plt.xlabel(r'Time (s)')

plt.ylim(0.0,1.0)

print "Saving figure..."
plt.savefig(outputfilename + ".png")
print "Done."
