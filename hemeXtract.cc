#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include <cmath>
#include <vector>
#include <sstream> 
#include <string>

#include <rpc/types.h>
//#include <rpc/xdr.h>
#include <argp.h>

#include "Mapping.h"
#include "Compare.h"
#include "HemeLBExtractionFile.h"

/* Command line parsing stuff */
const char *argp_program_version = "hemeXtract 0.0.1";
const char *argp_program_bug_address = "<robin.richardson@ucl.ac.uk>";
static char doc[] = "Extracts from one HemeLB Extraction File (.xtr) or compares two HemeLB Extraction files. If no output file is specified, output is written to stdout.";
static char args_doc[] = "MODE{-X OR -C} FILENAME1 [FILENAME2]";
static struct argp_option options[] = {
	{ "input",		'i', "inputfilename", 0, "The name of the .xtr file to be processed."},
	{ "output",		'o', "outputfilename", 0, "Output file name."},
	{ "steplengthA",	'A', "time", 0, "Provide the step length (not provided by xtr files) for input file A."},
	{ "steplengthB",	'B', "time", 0, "Provide the step length (not provided by xtr files) for input file B."},
	{ "time1",		'1', "realtime", 0, "The simulation time at which to start processing snapshots."},
	{ "time2",		'2', "realtime", 0, "The simulation time at which to stop processing snapshots."},
	{ "numsnapshots",	'n', "number", 0, "The maximum number of snapshots to output."},
	{ "extract",		'X', 0, 0, "Extraction mode - Prints out data from the input file."},
	{ "compare",		'C', 0, 0, "Comparison mode - Compares the two input files (correlation, L2 norm, max/min velocity etc)."},
	{ "minexistent",	'm', "numsites", 0, "Only perform comparisons for sites where AT LEAST this number of neighbours exist. (numsites should be between 0 and 8; default is 1)"},
	{ "verbose",		'v', 0, 0, "Print out lots of information, such as file headers etc. both to file and to stderr."},
	{ "quiet",		'q', 0, 0, "Opposite of verbose option. No headers or info output. Only error messages will be shown."},
	{ "scaleA",		'a', "scaling", 0, "Velocity scaling for first input file."},
	{ "scaleB",		'b', "scaling", 0, "Velocity scaling for second input file."},
	{ "project",		'p', "commaSeparatedVector", 0, "If specified, the COMPARE mode diff will calculate the projection of the difference vector against this vector."},
	{ "translate",		't', "commaSeparatedVector", 0, "If specified (only meaningful in COMPARE mode) the second xtr input file will be translated by this vector before any comparison/analysis."},
	{ "stats",		's', 0, 0, "In EXTRACT mode, prints out column statistics rather than all data points (Extraction mode only). In COMPARE mode, calculates stats such as the correlation in velocity, WSS, etc."},
	{ "normcorrel",		'N', 0, 0, "In COMPARE mode, calculates correlation in velocity field using normalized velocity vectors"},
	{ "relativeErr",	'r', 0, 0, "In COMPARE mode, causes all errors to be measured relative (inputfile B relative to inputfile A)"},
	{ 0 }
};
enum Mode {UNSET, EXTRACT, COMPARE};
struct arguments {
	char *inputA, *inputB, *output;
	double time1, time2, steplengthA, steplengthB;
	int minexistent;
	Mode mode;
	int numsnapshots;
	bool verbose;
	bool do_stats;
	double scaleA, scaleB;
	Vector3 *project;
	Vector3 *translateB;
	bool relativeErr;
	bool normalize_correl;
};
static Vector3 * parse_vector3(char* arg) {
	std::stringstream ss(arg);
	std::vector<std::string> result;
	while(ss.good()) {
		std::string substr;
		getline(ss, substr, ',');
		result.push_back(substr);
	}
	if(result.size() != 3) {
		fprintf(stderr, "Error: 3 comma-separated floats should be specified for --project or --translate options.\n");
		return NULL;
	}
	return new Vector3(atof(result[0].c_str()), atof(result[1].c_str()), atof(result[2].c_str()));
}
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *arggs = reinterpret_cast<arguments*>(state->input);

	switch (key) {
		case 'A': arggs->steplengthA = atof(arg); break;
		case 'B': arggs->steplengthB = atof(arg); break;
		case '1': arggs->time1 = atof(arg); break;
		case '2': arggs->time2 = atof(arg); break;
		case 'a': arggs->scaleA = atof(arg); break;
		case 'b': arggs->scaleB = atof(arg); break;
		case 'n': arggs->numsnapshots = atoi(arg); break;
		case 'm': arggs->minexistent = atoi(arg); break;
		case 'X':
			arggs->mode = EXTRACT;
			break;
		case 'C':
			arggs->mode = COMPARE;
			break;
		case 'v':
			arggs->verbose = true;
			break;
		case 'q':
			arggs->verbose = false;
			break;
		case 's':
			arggs->do_stats = true;
			break;
		case 'o':
			arggs->output = arg;
			break;
		case 'r':
			arggs->relativeErr = true;
			break;
		case 'N':
			arggs->normalize_correl = true;
			break;
		case 'p':
			arggs->project = parse_vector3(arg);
			if(arggs->project == NULL) {
				return ARGP_ERR_UNKNOWN;
			}
			break;
		case 't':
			arggs->translateB = parse_vector3(arg);
			if(arggs->translateB == NULL) {
				return ARGP_ERR_UNKNOWN;
			}
			break;
		case 'i':
		case ARGP_KEY_ARG:
			if(arggs->inputA == NULL) {
				arggs->inputA = arg;
			} else if(arggs->inputB == NULL) {
				arggs->inputB = arg;
			} else {
				fprintf(stderr, "Error: Both input files already specified.\n");
				return ARGP_ERR_UNKNOWN;
			}
		break;

		default: return ARGP_ERR_UNKNOWN;
	}

	return 0;
}
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

/** Simple pointer swapper */
void swap(HemeLBExtractionFile **i, HemeLBExtractionFile **j) {
	HemeLBExtractionFile *temp = *i;
	*i = *j;
	*j = temp;
}

/** Parses the command line options and either '-X'tracts from a single file, or '-C'ompares two files. */
int main(int argc, char **argv)
{
	FILE *outfile = NULL;
	lattice_map * mapping_A_B = NULL;
	HemeLBExtractionFile *hefA = NULL, *hefB = NULL;

	// Set default command line argument values
	struct arguments arguments;
	arguments.mode = UNSET;
	arguments.inputA = NULL;
	arguments.inputB = NULL;
	arguments.output = NULL;
	arguments.steplengthA = 1;
	arguments.steplengthB = 1;
	arguments.time1 = 0; // By default, start at zero time
	arguments.time2 = INFINITY; // By default, continue for all snapshot times
	arguments.numsnapshots = INT_MAX;
	arguments.verbose = true;
	arguments.minexistent = 1;
	arguments.do_stats = false;
	arguments.scaleA = 1.0;
	arguments.scaleB = 1.0;
	arguments.project = NULL;
	arguments.translateB = NULL;
	arguments.relativeErr = false;
	arguments.normalize_correl = false;

	// Parse the command line arguments
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if(arguments.verbose == true) {
		fprintf(stderr, "outputfname = %s\n", arguments.output);
	}

	// Make sure a mode has been specified by the user
	if(arguments.mode == UNSET) {
		fprintf(stderr, "Error: Mode is unset. Use -X for 'Extract' or -C for 'Compare'.\n");
		return 1;
	}

	// Complain if relativeErr is set when not in COMPARE mode
	if(arguments.relativeErr == true && arguments.mode == EXTRACT) {
		fprintf(stderr, "Error: relativeErr can only be used with COMPARE mode (it makes no sense in EXTRACT mode)'.\n");
		return 1;
	}

	// Open output file if specified. Otherwise, set to stdout.
	if(arguments.output != NULL) {
		outfile = fopen(arguments.output, "w");
	} else {
		outfile = stdout;
	}

	// Open input file, and load first snapshot
	if(arguments.inputA == NULL) {
		fprintf(stderr, "Missing input file.\n");
		return 1;
	}
	hefA = new HemeLBExtractionFile(arguments.inputA, arguments.steplengthA, arguments.scaleA, arguments.verbose, NULL);
	if(hefA->correctly_initialised() == false) {
		return 1;
	}

	// If file is a colloids file, read it and dump it out now. (Remaining hemeXtract features are for "normal" files only)
	if(hefA->is_colloids_file() == true) {
		if(arguments.verbose == true) {
			fprintf(stderr, "Reading and dumping colloids file...\n");
		}
		hefA->read_and_print_colloids(outfile);
		if(arguments.verbose == true) {
			fprintf(stderr, "Done reading colloids.\n");
		}
		return 0;
	}
	
	if(arguments.verbose == true) {
		hefA->print_header(outfile);
		hefA->print_field_header(outfile);
	}


	// If in COMPARE mode, open the second input file and load first snapshot.
	if(arguments.mode == COMPARE) {
		if(arguments.inputB == NULL) {
			fprintf(stderr, "Missing second input file. (Comparison mode requires TWO input files to be specified)\n");
			return 1;
		}
		hefB = new HemeLBExtractionFile(arguments.inputB, arguments.steplengthB, arguments.scaleB, arguments.verbose, arguments.translateB);
		if(hefB->correctly_initialised() == false) {
			return 1;
		}
		if(arguments.verbose == true) {
			hefB->print_header(outfile);
			hefB->print_field_header(outfile);
		}

		// Check which input file has the highest resolution - put this as input file B (for accuracy of interpolation)
		if(hefA->get_voxelsz() > hefB->get_voxelsz()) {
			swap(&hefA, &hefB);
		}

		// Work out mapping between the two lattices
		mapping_A_B = get_mapping(hefA, hefB);

		// Print the column headings output by COMPARE mode
		if(arguments.verbose == true) {
			if(arguments.do_stats == true) {
				fprintf(outfile, "# timeA | timeB | Vel. Correl. | Max. Vel. A | Max. Vel. B | Shear stress Crosscorel. | Shear stress diff. L2 | Pressure Crosscorrel. | Pressure diff. L2\n");
			} else {
				fprintf(outfile, "# X | Y | Z | abs. diff.\n");
			}
		}
	}

	if(arguments.mode == EXTRACT) {
		// Print the column headings output by EXTRACT mode
		if(arguments.verbose == true) {
			if(arguments.do_stats == true) {
				hefA->print_stats_column_headings(outfile);
			} else {
				hefA->print_column_headings(outfile);
			}
		}
	}

	// Loop through all the snapshots in input file A
	int num_snapshots_considered = 0;
	while(hefA->no_more_snapshots() == false) {

		// Only consider snapshots from A between time1 and time2
		double timeA = hefA->get_time();
		bool skip_snapshot = false;
		if(timeA < arguments.time1) {
			skip_snapshot = true;
		} else if(timeA > arguments.time2) {
			break;
		}
		if(arguments.mode == COMPARE) {
			// Make sure the snapshot loaded in B is the closest possible in time to that of the current snapshot in A.
			// If the next is closer then load it.
			double time_difference_now = fabs(hefB->get_time() - timeA);
			double time_difference_next = fabs(hefB->get_time_next() - timeA);

			while(time_difference_now > time_difference_next) {
				hefB->load_next_snapshot();
				time_difference_now = fabs(hefB->get_time() - timeA);
				time_difference_next = fabs(hefB->get_time_next() - timeA);
			}	
			if(hefB->no_more_snapshots() == true) {
				if(arguments.verbose == true) {
					fprintf(stderr, "B has no more snapshots.\n");
				}
				break;
			}
		}
		if(num_snapshots_considered >= arguments.numsnapshots) {
			break;
		}


		// Perform the requested calculations, in accordance with the set mode
		if(skip_snapshot == false) {
			switch(arguments.mode)
			{
				case EXTRACT:
					if(arguments.do_stats == true) {
						hefA->print_stats(outfile);
					} else {
						hefA->print_all(outfile);
					}
					break;

				case COMPARE:
					if(arguments.do_stats == true) {
						compare(outfile, mapping_A_B, hefA, hefB, arguments.minexistent, arguments.normalize_correl);
					} else {
						diff(outfile, mapping_A_B, hefA, hefB, arguments.minexistent, arguments.project, arguments.relativeErr, arguments.verbose);
					}
					break;

				default:
					fprintf(stderr, "Error: Mode unsupported...\n");
					return 1;
			}
			num_snapshots_considered++;
		}

		// Load next snapshot in A
		hefA->load_next_snapshot();
	}
	fclose(outfile);
	return 0;
}
