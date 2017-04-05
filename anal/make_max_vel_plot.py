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
	
if len(sys.argv) != 10:
	sys.exit("Usage: python make_max_vel_plot.py PLANE1XTR PLANE2XTR STEPLENGTH1 STEPLENGTH2 SCALEA SCALEB TSTART TEND OUTPUTFILENAME")

plane1 = sys.argv[1]
plane2 = sys.argv[2]
steplength1 = str(float(sys.argv[3]))
steplength2 = str(float(sys.argv[4]))
scaleA = str(float(sys.argv[5]))
scaleB = str(float(sys.argv[6]))
timeStart = sys.argv[7]
timeEnd = sys.argv[8]
outputfilename = sys.argv[9]

hemeXtract = "~/hemeXtract/hemeXtract"

execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 +  " --scaleA " + scaleA + " --scaleB " + scaleB + " --time1=" + timeStart + " --time2=" + timeEnd + " --stats -o __hemeXtract_output\n")

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
rcParams['font.size'] = 19

(_,caps1,_) = plt.errorbar(vel[:,0], vel[:,3], label='High vel.', c='red', linewidth=1.2, linestyle='-', marker='')
(_,caps2,_) = plt.errorbar(vel[:,1], vel[:,4], label='Low vel.', c='black', linewidth=1.2, linestyle='-', marker='')

legend=plt.legend(loc=1, numpoints=1)

plt.title(title)
plt.ylabel(r'Velocity (ms$^-1$)')
plt.xlabel(r'Time (s)')

print "Saving figure..."
plt.savefig(outputfilename + ".png", dpi=800)
print "Done."

#s = ""
#s += "set title '" + title + "'\n"
##s += "set yrange [0:1.4]\n"
#s += "set ylabel 'Velocity (m/s)'\n"
#s += "set xlabel 'Time (s)'\n"
#s += "set term postscript eps noenhanced dashed color font 'Times-Roman,35 lw 15'\n"
#s += "set output '" + outputfilename + ".eps'\n"
#s += "plot './__hemeXtract_output' using 1:4 w l lw 5 t 'High vel.', './__hemeXtract_output' using 2:5 w l lw 5 lt 3 t 'Low vel.'\n"
##s += "plot './__hemeXtract_output' using 1:($4/$5) w l t 'Division'\n"
#with open("__tmp_gnuplot", "w") as outfile:
#	outfile.write(s)
#
#execute("gnuplot __tmp_gnuplot\n")
#execute("convert -flatten -density 400 " + outputfilename + ".eps " + outputfilename + ".png\n")
