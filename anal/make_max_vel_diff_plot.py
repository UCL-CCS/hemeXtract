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
	
if len(sys.argv) != 3:
	sys.exit("Usage: python make_max_vel_diff_plot.py PARAMSFILE OUTPUTFILENAME")

paramsfile = sys.argv[1]
outputfilename = sys.argv[2]

params = []
counter = 0
with open(paramsfile, "r") as infile:
	for line in infile.readlines():
		counter += 1
		sline = line.split()
		if len(sline) > 0:
			sline.append("/tmp/__hemeXtract_output_vel_diff_" + str(counter))
			params.append(sline)
		

hemeXtract = "~/hemeXtract/hemeXtract"

minTimeStart = float("Inf")
maxTimeEnd = float("-Inf")
for paramsline in params:
	comparison_txt = paramsline[0]
	plane1 = paramsline[1]
	plane2 = paramsline[2]
	steplength1 = paramsline[3]
	steplength2 = paramsline[4]
	scaleA = paramsline[5]
	scaleB = paramsline[6]
	timeStart = paramsline[7]
	timeEnd = paramsline[8]
	tempout = paramsline[9]

	if float(timeStart) < minTimeStart:
		minTimeStart = float(timeStart)
	if float(timeEnd) > maxTimeEnd:
		maxTimeEnd = float(timeEnd)

	execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 +  " --scaleA " + scaleA + " --scaleB " + scaleB + " --time1=" + timeStart + " --time2=" + timeEnd + " --stats -o " + tempout + "\n")


print "minTimeStart", minTimeStart
print "maxTimeEnd", maxTimeEnd


# Make matplotlib graph
title = plane1.split('/')[-1].replace('.xtr','').replace('plane_','') + " Max. Vel. Err."
rcParams['font.family'] = 'Times New Roman'
rcParams['font.size'] = 25

linestylelist = ['-', '--', ':', '-.']
counter = 0
for paramsline in params:
	vel = genfromtxt(paramsline[9]);
	comparison_txt = paramsline[0].replace("_", " ")
#	print "AAAAAAAAA", comparison_txt, vel[:,3]/vel[:,4]
	print "Max vel primary", max(vel[:,3])
	print "Max vel secondary", max(vel[:,4])
	plt.errorbar(vel[:,0], 100.0*(vel[:,3]-vel[:,4])/max(vel[:,3]), label=comparison_txt, c='black', linewidth=3, linestyle=linestylelist[counter], marker='')
	plt.xlim(minTimeStart, maxTimeEnd)
	counter += 1

reference_vel_str = str("%.1f" % max(vel[:,3])) + " ms$^{-1}$"
#legend=plt.legend(loc=1, numpoints=1, prop={'size':25})
plt.title(title)
plt.ylabel(r'Rel. Err. (%) [' + reference_vel_str + ']')
plt.xlabel(r'Time (s)')
plt.gca().tick_params(axis='x', pad=15)

plt.tight_layout()

print "Saving figure..."
plt.savefig(outputfilename + ".png", dpi=800)
print "Done."
