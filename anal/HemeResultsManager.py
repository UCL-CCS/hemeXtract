import os, sys, glob, xml.etree.cElementTree as ET

import Image
import ImageDraw
import ImageFont

def execute(command):
	print "Executing: " + command
	r = os.system(command);
	if r != 0:
		sys.exit("Command failed.")

class HemeResultsManager:
	def __init__(self):
		self.results = {}
		self.reset_montage_list()
		self.work_folder = "work/"
		execute("mkdir -p " + self.work_folder + "\n")

	class HemeResultsEntry:
		def __init__(self, path, dt, scale, config):
			self.path = path
			self.dt = dt
			self.scale = scale
			self.config = config
		def xtr_path(self, name, ext='.xtr'):
			return self.path + "results/Extracted/" + name + ext
		def toString(self):
			return " ".join([self.path, str(self.dt), str(self.scale), str(self.config)])

	def reset_montage_list(self):
		self.montage_list = []

	def add(self, key=None, path=None, configfname="config.xml", scale=1.0):
		if key == None:
			sys.exit("Please specify a key (a string which will be used to refer to it).")
		if key in self.results.keys():
			sys.exit("Key already exists. Please choose another (unique) one.")
		if path == None:
			sys.exit("Please supply the path to this results folder.")
		if path[-1] != '/':
			path += '/'

		# Get timestep length from config file
		config = path + configfname
		tree = ET.ElementTree(file=config)
		for elem in tree.iter(tag='step_length'):
			dt = float(elem.attrib['value'])
			print dt
			break
		self.results[key] = self.HemeResultsEntry(path, dt, scale, config)

	# Cross-section for a single file
	def make_cross_section_plot_single(self, key1, plane, time, minexistent=1, annotate='_', normal=None, relative=False, normvel=True):
		r1 = self.results[key1]
		if normal == None:
			normal = "None"
		else:
			normal = ",".join([str(i) for i in normal])
		if relative == True:
			relative_str = "T"
		else:
			relative_str = "F"
		if normvel == True:
			normvel_str = "T"
		else:
			normvel_str = "F"

		outfile = self.work_folder + plane + "__" + key1 + "__" + str(time) + "_cross"
		self.montage_list.append(outfile + ".png")
		execute("python ~/hemeXtract/anal/make_cross_section_plot.py " + " ".join([r1.xtr_path(plane), r1.xtr_path(plane), str(r1.dt), str(r1.dt), str(time), str(r1.scale), '0.0', str(minexistent), annotate, normal, "None", relative_str, normvel_str, outfile]) + "\n")

	# Cross-section for a comparison between two files
	def make_cross_section_plot(self, key1, key2, plane1, time, plane2=None, minexistent=1, annotate='_', normal=None, relative=False, translate=None, normvel=True):
		r1 = self.results[key1]
		r2 = self.results[key2]
		if translate == None:
			translate = "None"
		else:
			translate = ",".join([str(i) for i in translate])
		if plane2 == None:
			plane2 = plane1
		if normal == None:
			normal = "None"
		else:
			normal = ",".join([str(i) for i in normal])
		if relative == True:
			relative_str = "T"
		else:
			relative_str = "F"
		if normvel == True:
			normvel_str = "T"
		else:
			normvel_str = "F"

		outfile = self.work_folder + plane1 + "__" + plane2 + "__" + key1 + "__" + key2 + "__" + str(time) + "_cross"
		self.montage_list.append(outfile + ".png")
		execute("python ~/hemeXtract/anal/make_cross_section_plot.py " + " ".join([r1.xtr_path(plane1), r2.xtr_path(plane2), str(r1.dt), str(r2.dt), str(time), str(r1.scale), str(r2.scale), str(minexistent), annotate, normal, translate, relative_str, normvel_str, outfile]) + "\n")

	def make_all_planes_error_plot(self, key_list, planes, timeStart, timeEnd, cross=False, comparisonlabels=None):

		# convert key list into paths, and store in file
		paramsfilename = self.work_folder + "all_planes_error_plot_params_list"
		with open(paramsfilename, "w") as outfile:
			counter = 0
			for key_pair in key_list:
				if comparisonlabels == None:
					comparisonlabel = key_pair[0] + "vs" + key_pair[1]
				else:
					comparisonlabel = comparisonlabels[counter]
				r1 = self.results[key_pair[0]]
				r2 = self.results[key_pair[1]]
				for plane in planes:
					outfile.write(" ".join([comparisonlabel, r1.xtr_path(plane), r2.xtr_path(plane), str(r1.dt), str(r2.dt), str(r1.scale), str(r2.scale), str(timeStart), str(timeEnd)]) + "\n")
				counter += 1
		if cross == False:
			cross_str = 'F'
		else:
			cross_str = 'T'

		plotfile = self.work_folder + "__" + ".".join([i for pair in key_list for i in pair]) + "cross_" + cross_str + "_all_planes_error_diff"
		self.montage_list.append(plotfile + ".png")
		execute("python ~/hemeXtract/anal/make_all_planes_error_plot.py " + paramsfilename + " " + cross_str + " " + plotfile + "\n")

	def make_max_vel_diff_plot(self, key_list, plane, timeStart, timeEnd, comparisonlabels=None):

		# convert key list into paths, and store in file
		paramsfilename = self.work_folder + "max_vel_diff_plot_params_list_" + plane
		with open(paramsfilename, "w") as outfile:
			counter = 0
			for key_pair in key_list:
				if comparisonlabels == None:
					comparisonlabel = key_pair[0] + "vs" + key_pair[1]
				else:
					comparisonlabel = comparisonlabels[counter]
				r1 = self.results[key_pair[0]]
				r2 = self.results[key_pair[1]]
				outfile.write(" ".join([comparisonlabel, r1.xtr_path(plane), r2.xtr_path(plane), str(r1.dt), str(r2.dt), str(r1.scale), str(r2.scale), str(timeStart), str(timeEnd)]) + "\n")
				counter += 1
		plotfile = self.work_folder + plane + "__" + ".".join([i for pair in key_list for i in pair]) + "_max_vel_diff"
		self.montage_list.append(plotfile + ".png")
		execute("python ~/hemeXtract/anal/make_max_vel_diff_plot.py " + paramsfilename + " " + plotfile + "\n")

	def make_max_vel_plot(self, key1, key2, plane, timeStart, timeEnd):
		r1 = self.results[key1]
		r2 = self.results[key2]
		outfile = self.work_folder + plane + "__" + key1 + "__" + key2 + "_max_vel"
		self.montage_list.append(outfile + ".png")
		execute("python ~/hemeXtract/anal/make_max_vel_plot.py " + " ".join([r1.xtr_path(plane), r2.xtr_path(plane), str(r1.dt), str(r2.dt), str(r1.scale), str(r2.scale), str(timeStart), str(timeEnd), outfile]) + "\n")

	def make_single_max_vel_plot(self, key, plane, timeStart, timeEnd):
		r = self.results[key]
		outfile = self.work_folder + plane + "__" + key + "__single_max_vel"
		self.montage_list.append(outfile + ".png")
		execute("python ~/hemeXtract/anal/make_single_max_vel_plot.py " + " ".join([r.xtr_path(plane), str(r.dt), str(r.scale), str(timeStart), str(timeEnd), outfile]) + "\n")

	def make_correl_plot(self, key1, key2, plane, timeStart, timeEnd, normalize_vectors=False, auto_ext='.xtr'):
		r1 = self.results[key1]
		r2 = self.results[key2]
		if normalize_vectors == True:
			nv_str = "T"
		else:
			nv_str = "F"
		outfile = self.work_folder + plane + "__" + key1 + "__" + key2 + "_correl"
		self.montage_list.append(outfile + ".png")
		execute("python ~/hemeXtract/anal/make_correl_plot.py " + " ".join([r1.xtr_path(plane,ext=auto_ext), r2.xtr_path(plane,ext=auto_ext), str(r1.dt), str(r2.dt), str(r1.scale), str(r2.scale), str(timeStart), str(timeEnd), nv_str, outfile]) + "\n")

	def make_correl_matrix(self, key_list, plane, timeStart, timeEnd, normalize_vectors=False, auto_ext='.xtr', comparisonlabels=None):
		paramsfilename = self.work_folder + "_correl_matrix_params_file"
		with open(paramsfilename, "w") as outfile:
			counter = 0
			for key in key_list:
				if comparisonlabels == None:
					comparisonlabel = key
				else:
					comparisonlabel = comparisonlabels[counter]
				r = self.results[key]
				outfile.write(" ".join([comparisonlabel, r.xtr_path(plane, auto_ext), str(r.dt), str(r.scale)]) + '\n')
				counter += 1

		if normalize_vectors == True:
			nv_str = "T"
		else:
			nv_str = "F"
		outfile = self.work_folder + plane + "__" + "+".join(key_list) + "_correl_matrix"
		self.montage_list.append(outfile + ".png")
		execute("python ~/hemeXtract/anal/make_correl_matrix.py " + paramsfilename + " " + " ".join([str(timeStart), str(timeEnd), nv_str, outfile]) + "\n")

	def compare_WSS_absolute(self, key1, plane, time, minexistent, WSS_tuple=None):
		r1 = self.results[key1]
		outfile = self.work_folder + "compare_WSS_absolute" + plane + "__" + "-".join([key1, str(time), str(minexistent)]) + ".png"
		self.montage_list.append(outfile)

		if WSS_tuple == None:
			execute("python ~/hemeXtract/anal/compare_WSS.py " + " ".join([r1.xtr_path(plane), r1.xtr_path(plane), str(r1.dt), str(r1.dt), str(time), str(r1.scale), "0.0", str(minexistent), 'F', outfile]) + "\n")
		else:
			execute("python ~/hemeXtract/anal/compare_WSS.py " + " ".join([r1.xtr_path(plane), r1.xtr_path(plane), str(r1.dt), str(r1.dt), str(time), str(r1.scale), "0.0", str(minexistent), 'F', outfile, str(WSS_tuple[0]), str(WSS_tuple[1])]) + "\n")

	def compare_WSS(self, key1, key2, plane, time, minexistent, WSS_tuple=None, relative=False):
		r1 = self.results[key1]
		r2 = self.results[key2]
		outfile = self.work_folder + "compare_WSS" + plane + "__" + "-".join([key1, key2, str(time), str(minexistent)]) + ".png"
		self.montage_list.append(outfile)

                if relative == True:
                        relative_str = "T"
                else:
                        relative_str = "F"

		if WSS_tuple == None:
			execute("python ~/hemeXtract/anal/compare_WSS.py " + " ".join([r1.xtr_path(plane), r2.xtr_path(plane), str(r1.dt), str(r2.dt), str(time), str(r1.scale), str(r2.scale), str(minexistent), relative_str, outfile]) + "\n")
		else:
			execute("python ~/hemeXtract/anal/compare_WSS.py " + " ".join([r1.xtr_path(plane), r2.xtr_path(plane), str(r1.dt), str(r2.dt), str(time), str(r1.scale), str(r2.scale), str(minexistent), relative_str, outfile, str(WSS_tuple[0]), str(WSS_tuple[1])]) + "\n")

	def add_text_image(self, text):
		image = Image.new("RGBA", (400,200), (255,255,255))
		draw = ImageDraw.Draw(image)
		font = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', 72)
		draw.text((0, 10), text, (0,0,0), font=font)
		txtimgfname = self.work_folder + "TEXT" + "".join(text.split()) + ".png"
		self.montage_list.append("'" + txtimgfname + "'")
		image.save(txtimgfname)

	def make_montage(self, name, tile=5, size=3000):
		execute("montage -geometry " + str(size) + " -density 800 -tile " + str(tile) + " " + " ".join(self.montage_list) + " " + name + "\n")
		self.reset_montage_list()

	def print_info(self, key):
		print "\n\n\n***"
		print key, "info:"
		print "From config file:", self.results[key].config
		with open(self.results[key].config, "r") as infile:
			for line in infile.readlines():
				if "step_length" in line:
					print "HRM:", line
				if "voxel_size" in line:
					print "HRM:", line
				if '<path value="' in line:
					start = line.find('value="') + 7
					end = line.find('.txt"') + 4
					velfname = self.results[key].path + line[start:end].replace("./", "")
					print "HRM:", "Velocity inlet file:", velfname
					with open(velfname, "r") as velfile:
						maxv = float("-Inf")
						for vel in velfile.readlines():
							t, v = vel.split()
							if v > maxv:
								maxv = v
						print "HRM:", "Max. Vel. =", maxv
					weightsfname = velfname + ".weights.txt"
					print "HRM:", "Velocity weights file:", weightsfname
					with open(weightsfname, "r") as weightsfile:
						sum_weights = 0
						n = 0
						for line in weightsfile.readlines():
							weight = float(line.split()[3])
							sum_weights += weight
							n+=1
						print "HRM:", "Average weight =", sum_weights/n
		print "HRM:", "User has specified: " + self.results[key].toString()

