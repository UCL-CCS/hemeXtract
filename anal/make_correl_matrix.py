# -*- coding: utf-8 -*-

import os, sys
import numpy as np
import math
import matplotlib.pyplot as plt
import matplotlib.colors as colors
from matplotlib import rcParams
from pylab import genfromtxt;

def execute(command):
	print "Executing: " + command
	r = os.system(command);
	if r != 0:
		sys.exit("Command failed.")

import numpy as np

# From http://stackoverflow.com/questions/18926031/how-to-extract-a-subset-of-a-colormap-as-a-new-colormap-in-matplotlib
def truncate_colormap(cmap, minval=0.0, maxval=1.0, n=100):
    new_cmap = colors.LinearSegmentedColormap.from_list(
        'trunc({n},{a:.2f},{b:.2f})'.format(n=cmap.name, a=minval, b=maxval),
        cmap(np.linspace(minval, maxval, n)))
    return new_cmap
	
if len(sys.argv) != 6:
	sys.exit("Usage: python make_correl_matrix.py PARAMSFILE TIMESTART TIMEEND NORMVEC{T or F} OUTPUTFILENAME")

paramsfile = sys.argv[1]
timeStart = float(sys.argv[2])
timeEnd = float(sys.argv[3])
normvec = sys.argv[4]
if normvec == 'T':
        normvec_str = ' -N '
elif normvec == 'F':
        normvec_str = ''
else:
        sys.exit("NORMVEC should be T or F. Not '" + normvec  + "'.")
outputfilename = sys.argv[5]

params = []
with open(paramsfile, "r") as infile:
	for line in infile.readlines():
		sline = line.split()
		# Make u into a mu, and underscore into a space (not ideal, but quick fix...)
		sline[0] = sline[0].replace("um", u'μm').replace("_", "\n")
		if len(sline) > 0:
			params.append(sline)
		

hemeXtract = "~/hemeXtract/hemeXtract"
tempout = "/tmp/__hemeXtract_correl_matrix_output"

#execute("echo '' > " + tempout + "\n")
#for i, paramsline in enumerate(params):
#	label1 = paramsline[0]
#	plane1 = paramsline[1]
#	steplength1 = paramsline[2]
#	scaleA = paramsline[3]
#
#	for j, paramsline in enumerate(params[i:]):
#		label2 = paramsline[0]
#		plane2 = paramsline[1]
#		steplength2 = paramsline[2]
#		scaleB = paramsline[3]
#
#		execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 +  " --scaleA " + scaleA + " --scaleB " + scaleB + " --time1=" + str(timeStart) + " --time2=" + str(timeEnd) + " --stats" + normvec_str + " | tail -n1 | awk '{print " + str(i) + "," + str(j + i) + ", $3}' >> " + tempout + "\n")


# Make matplotlib graph
title = " Correlation matrix "
rcParams['font.family'] = 'Times New Roman'
rcParams['font.size'] = 15
plt.figure(figsize=(100,100))
plt.subplot(111)

N = len(params)
correl = np.array([[float("NaN") for i in range(N)] for j in range(N)]) # initialise empty matrix
print correl

masked_correl = np.ma.array(correl, mask=np.isnan(correl))
trunc_cmap = truncate_colormap(plt.cm.Blues, 0.5, 0.05, 100)
trunc_cmap.set_bad('white',1.)

results = genfromtxt(tempout)
for r in results:
	i = r[0]
	j = r[1]
	c = r[2]
	correl[j][i] = c

print correl

plt.matshow(correl, cmap=trunc_cmap)
for (j,i),label in np.ndenumerate(correl):
	if j >= i:
		plt.text(i,j,label,ha='center',va='center', fontsize=12, color='black')

# Set to only one half of matrix (since symmetric)
ax = plt.gca()
ax.spines['right'].set_visible(False)
ax.spines['top'].set_visible(False)
ax.yaxis.set_ticks_position('left')
ax.xaxis.set_ticks_position('bottom')

plt.tick_params(axis='x', which='both', bottom='off', top='off', labelbottom='on')
plt.tick_params(axis='y', which='both', left='off', right='off', labelleft='on')

# Set axis labels
labels_loc = []
labels = []
for i, paramsline in enumerate(params):
	labels_loc.append(i)
	labels.append(paramsline[0])

plt.xticks(labels_loc, labels)
plt.yticks(labels_loc, labels)
#plt.xticks(rotation=45)

# Improve margin
x1,x2,y1,y2 = plt.axis()
print x1,x2,y1,y2
plt.axis((x1,x2,y1,y2))


print "Saving figure..."
plt.savefig(outputfilename + ".png", format='png', dpi=800, bbox_inches='tight')
print "Done."
