import os, sys
import numpy as np
import math

def execute(command):
	print "Executing: " + command
	r = os.system(command);
	if r != 0:
		sys.exit("Command failed.")
	
if len(sys.argv) != 8:
	sys.exit("Usage: python make_max_vel_plot.py PLANE1XTR PLANE2XTR STEPLENGTH1 STEPLENGTH2 SCALEA SCALEB OUTPUTFILENAME")

plane1 = sys.argv[1]
plane2 = sys.argv[2]
steplength1 = str(float(sys.argv[3]))
steplength2 = str(float(sys.argv[4]))
scaleA = str(float(sys.argv[5]))
scaleB = str(float(sys.argv[6]))
outputfilename = sys.argv[7]

hemeXtract = "~/hemeXtract/hemeXtract"

execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 +  " --scaleA " + scaleA + " --scaleB " + scaleB + " --stats -o __hemeXtract_output\n")

# Make gnuplot file
if "plane_" in plane1:
	planetit = plane1.split("plane_")[1]
elif "plane" in plane1:
	planetit = plane1.split("plane")[1]
elif "/" in plane1:
	planetit = plane1.split("/")[1]
title = planetit.replace(".xtr", "")
s = ""
s += "set title '" + title + "'\n"
s += "set yrange [0:1.4]\n"
s += "set ylabel 'Velocity (m/s)'\n"
s += "set xlabel 'Time (s)'\n"
s += "set term postscript eps noenhanced color font 'Times-Roman,24 lw 15'\n"
s += "set output '" + outputfilename + ".eps'\n"
s += "plot './__hemeXtract_output' using 1:4 w l t 'A', './__hemeXtract_output' using 1:5 w l t 'B'\n"
with open("__tmp_gnuplot", "w") as outfile:
	outfile.write(s)

execute("gnuplot __tmp_gnuplot\n")
execute("convert -flatten -density 400 " + outputfilename + ".eps " + outputfilename + ".png\n")
