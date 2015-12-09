import os, sys, glob

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
		def __init__(self, path, dt):
			self.path = path
			self.dt = dt
		def xtr_path(self, name):
			return self.path + "results/Extracted/" + name + ".xtr"

	def reset_montage_list(self):
		self.montage_list = []

	def add(self, key=None, path=None, dt=None):
		if key == None:
			sys.exit("Please specify a key (a string which will be used to refer to it).")
		if key in self.results.keys():
			sys.exit("Key already exists. Please choose another (unique) one.")			
		if path == None:
			sys.exit("Please supply the path to this results folder.")
		if dt == None:
			sys.exit("Please specify the time step this result folder's job was run with.")
		self.results[key] = self.HemeResultsEntry(path, dt)

	def make_cross_section_plot(self, key1, key2, plane, time):
		r1 = self.results[key1]
		r2 = self.results[key2]
		outfile = self.work_folder + plane + "_cross"
		self.montage_list.append(outfile + ".png")
		execute("python make_cross_section_plot.py " + r1.xtr_path(plane) + " " + r2.xtr_path(plane) + " " + str(r1.dt) + " " + str(r2.dt) + " " + str(time) + " " + outfile + "\n")

	def make_montage(self, name):
		execute("montage -geometry 1000x600 " + " ".join(self.montage_list) + " " + name + "\n")
		self.reset_montage_list()

hrm = HemeResultsManager()
hrm.add(key='40_50', path="results/40_50/", dt=4.48e-6)
hrm.add(key='40_25', path="results/40_25/", dt=4.48e-6)
hrm.add(key='80_25', path="results/80_25/", dt=1.792e-5)
hrm.add(key='80_12.5', path="results/80_12.5/", dt=1.792e-5)

planes = [
"plane_ACommA",
"plane_LACA-A1",
"plane_LICA",
"plane_LPCA-A1",
"plane_LPCommA",
"plane_RACA-A2",
"plane_RMCA",
"plane_RPCA-A2",
"plane_BA",
"plane_LACA-A2",
"plane_LMCA",
"plane_LPCA-A2",
"plane_RACA-A1",
"plane_RICA",
"plane_RPCA-A1",
"plane_RPCommA"
]

for plane in planes:
	hrm.make_cross_section_plot('40_50', '40_25', plane, 2.9)
hrm.make_montage("40micron.jpg")

for plane in planes:
	hrm.make_cross_section_plot('40_25', '80_25', plane, 2.9)
hrm.make_montage("40-80micron.jpg")
