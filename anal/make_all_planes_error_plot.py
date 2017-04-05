# -*- coding: utf-8 -*-

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
	
if len(sys.argv) != 4:
	sys.exit("Usage: python make_all_planes_error_plot.py PARAMSFILE CROSS{T or F} OUTPUTFILENAME")

paramsfile = sys.argv[1]
cross = sys.argv[2]
outputfilename = sys.argv[3]

if cross == 'T':
	cross = True
elif cross == 'F':
	cross = False
else:
	sys.exit("CROSS should be T for True, or F for False.")

params = []
counter = 0
last_ct = None
tfiles = []
with open(paramsfile, "r") as infile:
	for line in infile.readlines():
		sline = line.split()
		if len(sline) > 0:
			if sline[0] != last_ct:
				counter += 1
				last_ct = sline[0]
				tfiles.append("/tmp/__hemeXtract_output_vel_diff_" + str(counter))
			sline.append("/tmp/__hemeXtract_output_vel_diff_" + str(counter))
			sline.append(counter)
			execute("echo '' > " + sline[9] + "\n") # Make sure tmp output file is empty before appending results to it
			params.append(sline)
		
hemeXtract = "~/hemeXtract/hemeXtract"

xlabels = ["" for i in range(counter+1)]
for paramsline in params:
	comparison_txt = paramsline[0].replace("um", u'μm')
	plane1 = paramsline[1]
	plane2 = paramsline[2]
	steplength1 = paramsline[3]
	steplength2 = paramsline[4]
	scaleA = paramsline[5]
	scaleB = paramsline[6]
	timeStart = paramsline[7]
	timeEnd = paramsline[8]
	tempout = paramsline[9]
	counter = paramsline[10]

	print int(counter)
	print xlabels
	xlabels[int(counter)] = (comparison_txt.replace("_", "\n")) # change underscores to newlines
	
	# If cross is true, we use the max-vel diff from the crosssectional comparison
	# else we use the max vel going through the plane
	if cross == False:
		execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 +  " --scaleA " + scaleA + " --scaleB " + scaleB + " --time1=" + timeStart + " --time2=" + timeEnd + " --stats | awk 'BEGIN{MAX_DIFF=-10000.0; MAX_VEL=-10000.0} NR>1 {diff = $4-$5; if(MAX_DIFF < diff) MAX_DIFF=diff; if(MAX_VEL < $4) MAX_VEL = $4} END{print " + str(counter) + ", MAX_DIFF/MAX_VEL}' >> " + tempout + "\n")
	else:
		# Get the maximum velocity at the specified time
		execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 +  " --scaleA " + scaleA + " --scaleB " + scaleB + " --time1=" + timeStart + " -n 1 --stats -o /tmp/__hemeXtract_output_maxvel\n")

		# Read in the plane1 max vel value from file
		max_vel = None
		with open("/tmp/__hemeXtract_output_maxvel", "r") as infile:
			for line in infile.readlines():
				if line.startswith("#"):
					continue
				max_vel = float(line.split()[3])
			print "Maximum velocity through plane", plane1, "is", max_vel
		if max_vel == None:
			sys.exit("Could not read max velocity of plane1 from /tmp/__hemeXtract_output_maxvel")

		# Get the characteristic lengthscale
		execute(hemeXtract + " -X " + plane1 + " -A " + steplength1 + " --scaleA " + scaleA + " --time1=" + timeStart + " -n 1 -o /tmp/__hemeXtract_output_dimensions\n")
		dim = genfromtxt("/tmp/__hemeXtract_output_dimensions")
		dimx = max(dim[:,1]) - min(dim[:,1])
		dimy = max(dim[:,2]) - min(dim[:,2])
		dimz = max(dim[:,3]) - min(dim[:,3])
		char_len = np.sqrt(dimx*dimx + dimy*dimy + dimz*dimz)
		Re = max_vel * char_len / 4e-6
		print "Characteristic length =", char_len, "Re =", Re

		execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 +  " --scaleA " + scaleA + " --scaleB " + scaleB + " --time1=" + timeStart + " -n 1 | awk 'BEGIN{MAX_DIFF=-10000.0; MAX_VEL=-10000.0} NR>1 {diff = $4; if(MAX_DIFF < diff) MAX_DIFF=diff;} END{print " + str(counter) + ", MAX_DIFF/" + str(max_vel) + ", " + str(max_vel) + "," + str(Re) + "}' >> " + tempout + "\n")

# Make matplotlib graph
if cross == False:
	title = "Difference in max vel. (all planes)"
	marker_choice = 'o'
else:
	title = "Cross-sectional max velocity difference (all planes)"
	marker_choice = ['o', 'p', 's', 'v', '^', 'o']
	colour_choice = ['blue', 'green', 'yellow', 'orange', 'red']

rcParams['font.family'] = 'Times New Roman'
rcParams['font.size'] = 32
rcParams['figure.figsize'] = 24, 12

plt.title(title)

plt.subplot(121)
counter = 0
for f in tfiles:
	diff=genfromtxt(f)
	print diff
	plt.errorbar(diff[:,3], diff[:,1]*100.0, marker=marker_choice[counter], linestyle='', markersize=18, label=xlabels[counter+1], c=colour_choice[counter])

#	Trend line
#	z = np.polyfit(diff[:,2], diff[:,1]*100.0, 1)
#	p = np.poly1d(z)
#	plt.plot(diff[:,2],p(diff[:,2]),"--", c=colour_choice[counter])

	counter += 1
plt.ylabel(r'Max. Cross. Err. (%)')
plt.xlabel('Re')

x0, x1, y0, y1 = plt.axis()
plt.axis((x0 - 0.05, x1 + 0.1, y0, y1))

handles, labels = plt.gca().get_legend_handles_labels()
plt.legend(handles[::-1], labels[::-1], loc=1, numpoints=1, prop={'size':23})

#legend=plt.legend(loc=1, numpoints=1, prop={'size':24})
plt.ylim(0.0,80.0)

plt.tight_layout()

plt.subplot(122)
counter = 0
data = []
for f in tfiles:
	diff=genfromtxt(f)
	vel_err = diff[:,1] * 100.0
	print vel_err
	print np.mean(vel_err), np.std(vel_err)
	data.append(vel_err)

#	(_,caps,_) = plt.errorbar(counter, np.mean(diff[:,1] * 100.0), yerr=np.std(diff[:,1] * 100.0), marker=marker_choice[counter], linestyle='', markersize=18, label=xlabels[counter+1], c=colour_choice[counter], capsize=5, elinewidth=2, ecolor='black')
#	for cap in caps:
#		cap.set_markeredgewidth(2)
	counter += 1

bp = plt.boxplot(data, patch_artist=True, whis=100)

for counter, box in enumerate(bp['boxes']):
	box.set( color='black', linewidth=2)
	box.set_facecolor(colour_choice[counter])
for whisker in bp['whiskers']:
	whisker.set(color='#000000', linewidth=2)
for cap in bp['caps']:
	cap.set(color='#000000', linewidth=2)
for median in bp['medians']:
	median.set(color='#000000', linewidth=2)

ax = plt.gca()
ax.set_xticklabels(xlabels[1:])
ax.set_yticklabels([])
#plt.xticks(rotation=45)
plt.margins(x=0.1)
#plt.ylabel(r'Velocity difference (%)')
plt.ylabel(r'')
plt.xlabel(r'')
plt.ylim(0.0,80.0)

plt.tight_layout()

print "Saving figure..."
plt.savefig(outputfilename + ".png", format='png', dpi=800)
print "Done."

