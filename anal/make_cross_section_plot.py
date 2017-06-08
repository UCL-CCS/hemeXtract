# -*- coding: utf-8 -*-

import os, sys
import numpy as np
import math

import scipy.interpolate
import matplotlib.pyplot as plt
from matplotlib import rcParams
from matplotlib.ticker import MaxNLocator
from scipy.ndimage.filters import gaussian_filter, maximum_filter

def execute(command):
	print "Executing: " + command
	r = os.system(command);
	if r != 0:
		sys.exit("Command failed.")
	
def normalise(v):
	l = math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
	return v/l

def project(p, c, n):
	return p - np.dot((p - c), n) * n

def perp(v):
    if v[1] == 0 and v[2] == 0:
        if v[0] == 0:
            raise ValueError('zero vector')
        else:
            return np.cross(v, [0, 1, 0])
    return np.cross(v, [1, 0, 0])

def dot(a, b):
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]

if len(sys.argv) != 13:
	sys.exit("Usage: python make_cross_section_plot.py PLANE1XTR PLANE2XTR STEPLENGTH1 STEPLENGTH2 TIME SCALEA SCALEB MINEXISTENT ANNOTATE NORMAL{'None', or comma separated list} RELATIVE{'T' or 'F'} OUTPUTFILENAME")

plane1 = sys.argv[1]
plane2 = sys.argv[2]
steplength1 = str(float(sys.argv[3]))
steplength2 = str(float(sys.argv[4]))
time = str(float(sys.argv[5]))
scaleA = str(float(sys.argv[6]))
scaleB = str(float(sys.argv[7]))
minexistent = str(int(sys.argv[8]))
annotate = sys.argv[9]
normal = sys.argv[10]
if normal == "None":
	normal = ""
else:
	normal = " --project " + normal
relative = sys.argv[11]
if relative == 'T':
	relative = " --relativeErr "
elif relative == 'F':
	relative = ""
else:
	sys.exit("RELATIVE should be 'T' or 'F'")

outputfilename = sys.argv[12]

hemeXtract = "~/hemeXtract/hemeXtract"

# Extract/calc the cross sectional error using hemeXtract
execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 + " -1 " + time + " --scaleA " + scaleA + " --scaleB " + scaleB + " --minexistent " + minexistent + normal + relative + " --quiet -n 1 -o /tmp/__hemeXtract_output\n")

# Get the max vel going through the plane at the given time
execute(hemeXtract + " -C " + plane1 + " " + plane2 + " -A " + steplength1 + " -B " + steplength2 +  " --scaleA " + scaleA + " --scaleB " + scaleB + " --time1=" + time + " --quiet -n 1 --stats -o /tmp/__hemeXtract_output_maxvel\n")

# Read in the plane1 max vel value from file
max_vel_plane1 = None
with open("/tmp/__hemeXtract_output_maxvel", "r") as infile:
	for line in infile.readlines():
		if line.startswith("#") or len(line.split()) == 0 :
			continue
		max_vel_plane1 = float(line.split()[3])
		print "Maximum velocity through plane", plane1, "is", max_vel_plane1
if max_vel_plane1 == None:
	sys.exit("Could not read max velocity of plane1 from /tmp/__hemeXtract_output_maxvel")

# Read in the site coordinates and values from file
coords = []
data = []
with open("/tmp/__hemeXtract_output", "r") as infile:
	for line in infile.readlines():
		if line.startswith("#") or len(line.split()) == 0 :
			continue
		x, y, z, val = [float(i) for i in line.split()]
		coords.append(np.array([x,y,z]))
		data.append(val)

# Calculate centroid of all site coordinates
cx = 0
cy = 0
cz = 0
for coord in coords:
	cx += coord[0]
	cy += coord[1]
	cz += coord[2]
n = len(coords)
cx/=n
cy/=n
cz/=n
centroid = np.array([cx, cy, cz])

# Subtract centroid from all site coordinates, centering the cloud on the origin
for i, coord in enumerate(coords):
	coords[i] = coord - centroid;
coords = np.array(coords)

# Do a singular value decomposition of the coordinate matrix
print "Doing SVD..."
U, s, V = np.linalg.svd(coords, full_matrices=False)

# The normal of the plane is the right-most vector in the SVD
normal = V[-1]

# Try to keep seeing planes from same orientation (otherwise small variations due to resolution changes cause the SVD to give flipped normals)
test_vec1 = [1.0/math.sqrt(3.0),1.0/math.sqrt(3.0),1.0/math.sqrt(3.0)]
test_vec2 = [0.0,1.0/math.sqrt(2.0),-1.0/math.sqrt(2.0)]
thresh = 0.01
print "NORMAL", normal
dot_prod = dot(normal, test_vec1)
print "ORIG", dot_prod
if abs(dot_prod) < thresh:
	print "SWAP", dot_prod
	dot_prod = dot(normal, test_vec2) # If normal is too similar to the test normal, compare with a different (orthogonal) one
if dot_prod < 0:
	normal *= -1.0

# Get two perpendicular vectors in the plane of the points
a = perp(normal)
b = np.cross(normal, a)

# The centroid is now zero
centroid = np.array([0,0,0])

# Project the points onto the calculated plane
print "Projecting points onto plane..."
out = []
new = []
for p in coords:
	new.append(project(p, centroid, normal))
	out.append(p)

# Project points onto the two in-plane orthogonal axes
out = []
minx = float("Inf")
maxx = float("-Inf")
miny = float("Inf")
maxy = float("-Inf")
for i, p in enumerate(new):
	x = np.dot(p, a)
	y = np.dot(p, b)
	out.append([x, y, data[i]])

	if x > maxx:
		maxx = x
	elif x < minx:
		minx = x
	if y > maxy:
		maxy = y
	elif y < miny:
		miny = y
out = np.array(out)


# Estimate the Reynold's number for this plane
ddx = maxx - minx
ddy = maxy - miny
char_len = np.sqrt(ddx * ddx + ddy * ddy)
Re = max_vel_plane1 * char_len / 4e-6
print plane1, "vel", max_vel_plane1, " characteristic length", char_len, "Re =", Re

# Sort the x,y coords in ascending order
print "Sorting points..."
indices = np.lexsort((out[:, 1], out[:, 0]))
out = out[indices]

out_x = np.array(out[:,0]) * 1000.0
out_y = np.array(out[:,1]) * 1000.0
#out_err = np.array(out[:,2]) ###OLD
out_err = (np.array(out[:,2])/max_vel_plane1)*100.0

rcParams['font.family'] = 'Times New Roman'
rcParams['font.size'] = 25


if "plane_" in plane1:
       planetit = plane1.split("plane_")[1]
elif "plane" in plane1:
       planetit = plane1.split("plane")[1]
elif "/" in plane1:
       planetit = plane1.split("/")[1]
#title = planetit.replace(".xtr", "") ###OLD
reference_vel_str = str("%.1f" % max_vel_plane1) + " ms$^{-1}$"
title = 'Cross. Err. (' + annotate.replace("_", " ") + ')'
plt.title(title)

plt.ylabel(r'y (mm)')
plt.xlabel(r'x (mm)')

plt.xticks(rotation=45)

print "Meshing grid..."
xi, yi = np.linspace(out_x.min(), out_x.max(), 100), np.linspace(out_y.min(), out_y.max(), 100)
xi, yi = np.meshgrid(xi, yi)

print "Interpolating..."
zi = scipy.interpolate.griddata((out_x, out_y), out_err, (xi, yi), method='cubic')

print "Creating contour map..."
if len(relative) > 0:
	levels = MaxNLocator(nbins=15).tick_values(out_err.min(), out_err.max())
	plt.contourf(xi, yi, zi, origin='lower', extent=[0.93*(out_x.min()), 1.07*(out_x.max()), 0.93*(out_y.min()), 1.07*(out_y.max())], cmap=plt.cm.inferno, levels=levels, vmin=0, vmax=100)
	plt.colorbar(label=r'Abs. Err. (%) [' + reference_vel_str + ']')
else:
#	levels = MaxNLocator(nbins=15).tick_values(out_err.min(), out_err.max())
#	plt.contourf(xi, yi, zi, origin='lower', extent=[0.93*(out_x.min()), 1.07*(out_x.max()), 0.93*(out_y.min()), 1.07*(out_y.max())], cmap=plt.cm.inferno, levels=levels)
#	plt.colorbar(label=r'Diff. (m s$^{-1}$)')
	levels = MaxNLocator(nbins=15).tick_values(out_err.min(), out_err.max())
	plt.contourf(xi, yi, zi, origin='lower', extent=[0.93*(out_x.min()), 1.07*(out_x.max()), 0.93*(out_y.min()), 1.07*(out_y.max())], cmap=plt.cm.inferno, levels=levels)
	plt.colorbar(label=r'Abs. Err. (%) [' + reference_vel_str + ']')

plt.tight_layout()

print "Saving figure..."
plt.savefig(outputfilename + ".png", format='png', dpi=800)
print "Done."

## Write out realigned data
#with open("__tmp_cross_section", "w") as outfile:
#	for p in out:
#		outfile.write(str(p[0]) + " " + str(p[1]) + " " + str(p[2]) + "\n")
#
## Make gnuplot file
#if "plane_" in plane1:
#	planetit = plane1.split("plane_")[1]
#elif "plane" in plane1:
#	planetit = plane1.split("plane")[1]
#elif "/" in plane1:
#	planetit = plane1.split("/")[1]
#title = planetit.replace(".xtr", "")
#s = ""
#s += "unset key\n"
#s += "set view map\n"
#s += "set xtics rotate by -45\n"
#s += "set title '" + title + "'\n"
#s += "set ylabel 'y (mm)'\n"
#s += "set xlabel 'x (mm)'\n"
#s += "set term postscript eps noenhanced color font 'Times-Roman,24 lw 15'\n"
#s += "set output '" + outputfilename + ".eps'\n"
#s += "splot './__tmp_cross_section' using ($1*1000.0):($2*1000):3 with points palette pointsize 0.5 pointtype 7\n"
#with open("__tmp_gnuplot", "w") as outfile:
#	outfile.write(s)
#
#execute("gnuplot __tmp_gnuplot\n")
#execute("convert -flatten -density 300 " + outputfilename + ".eps " + outputfilename + ".png\n")
