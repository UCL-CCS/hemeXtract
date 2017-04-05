#ifndef INCLUDED_HEMELBEXTRACTIONFILE_H
#define INCLUDED_HEMELBEXTRACTIONFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rpc/xdr.h>
#include <argp.h>
#include <vector>
#include <math.h>
#include <string.h>
#include <tr1/unordered_map>
#include <string>
#include <sstream>

#include "HemeLBExtractionFileTypes.h"
#include "Snapshot.h"
#include "Vector3.h"

class HemeLBExtractionFile
{
	public:
		HemeLBExtractionFile(char *fname, double step_length, double scaling)
		{
			this->step_length = step_length;
			this->scaling = scaling;
			bool_correctly_initialised = false;
			snapshot = NULL;
			in = fopen(fname, "rb");
			if (in == NULL) {
				fprintf(stderr, "Could not open file '%s'. Does it exist?\n", fname);
				return;
			}
			xdrstdio_create(&xdrs, in, XDR_DECODE);

			has_velocity = false;
			has_pressure = false;
			has_shearstress = false;

			// Read the file header, populating the 'header' class variable. Determines whether file is "normal" or colloids
			int r;
			r = read_header();
			if(r != 0) {
				fprintf(stderr, "Problem reading header for file: '%s'.\n", fname);
				return;
			}

			// If file is a colloids file, return now (no more info to be read in)
			if(header->is_colloids_file == true) {
				bool_correctly_initialised = true;
				return;
			}

			// Otherwise, if file is "normal", read the field header
			r = read_field_header();
			if(r != 0) {
				fprintf(stderr, "Problem reading field header for file: '%s'.\n", fname);
				return;
			}

			// Allocate a snapshot of the right dimensions
			snapshot = new Snapshot(header, field_header);

			// Get the time of the next snapshot
			bool next = read_time_next();
			if(next == false) {
				bool_no_more_snapshots = true;
			} else {
				bool_no_more_snapshots = false;
				load_next_snapshot(); // Load in the first snapshot
				bool_correctly_initialised = true;
			}
		}

		~HemeLBExtractionFile()
		{
			xdr_destroy(&xdrs);
			fclose(in);
		}

		bool read_time_next()
		{
			// Check if we have reached the end of the file
			if (feof(in)) {
				fprintf(stderr, "# Reached end of file.\n");
				return false;
			}

			// Work around for problem with xdr_u_long() not working
			uint32_t dt1, dt2;
			uint64_t step;
			xdr_u_int(&xdrs, &dt1);
			xdr_u_int(&xdrs, &dt2);
			step = ((uint64_t)dt1)<<32 | dt2;

			time_next = step * step_length;
			return true;
		}

		int load_next_snapshot()
		{
			if(bool_no_more_snapshots == true) {
				fprintf(stderr, "Warning: No more snapshots left in file.\n");
				return 1;
			}

			uint32_t gridposx, gridposy, gridposz;
			float value;

			snapshot->set_timestep(time_next);
			for(unsigned int s = 0; s < header->num_sites; s++) {
				xdr_u_int(&xdrs, &gridposx);
				xdr_u_int(&xdrs, &gridposy);
				xdr_u_int(&xdrs, &gridposz);
				snapshot->site_set(s, gridposx, gridposy, gridposz);
				for(unsigned int i = 0; i < header->num_columns; i++) {
					// Read value and add it to the appropriate data column (scaling it by the specified scaling)
					xdr_float(&xdrs, &value);
					snapshot->column_set_plus_offset(i, s, value * this->scaling);
				}
			}
			bool next = read_time_next();
			if(next == false) {
				bool_no_more_snapshots = true;
			}
			return 0;
		}

		// Workaround for XDR's broken long reading ability
		void xdr_long(XDR *xdrs, uint64_t *ret)
		{
			uint32_t half1, half2;
			xdr_u_int(xdrs, &half1);
			xdr_u_int(xdrs, &half2);
			*ret = ((uint64_t)half1)<<32 | half2; // Assume big-endianness
		}


		void read_and_print_colloids(FILE *outfile)
		{
			uint32_t headerLen, recordLen;
			uint64_t dsetLen;
			double timeStep;
			uint64_t id, rank;
			double A0, Ah, X, Y, Z;
			uint32_t count = 0;

			while(!feof(in)) {

				fprintf(outfile, "# Snapshot number %u\n", count);

				// Read timestep header
				xdr_u_int(&xdrs, &headerLen);
				xdr_u_int(&xdrs, &recordLen);
				xdr_long(&xdrs, &dsetLen);
				xdr_double(&xdrs, &timeStep);
				fprintf(outfile, "# headerLen: %u recordLen %u dsetLen %ld timeStep: %e\n", headerLen, recordLen, dsetLen, timeStep);

				// Calc. num particles from the record length data
				uint32_t num_particles = dsetLen/recordLen;

				for (uint32_t i = 0; i < num_particles; i++) {
					// Read particle data
					xdr_long(&xdrs, &id);
					xdr_long(&xdrs, &rank);
					xdr_double(&xdrs, &A0);
					xdr_double(&xdrs, &Ah);
					xdr_double(&xdrs, &X);
					xdr_double(&xdrs, &Y);
					xdr_double(&xdrs, &Z);

					// Rescale the positions by the scaling factor provided to hemeXtract
					X *= this->scaling;
					Y *= this->scaling;
					Z *= this->scaling;

					fprintf(outfile, "ID: %ld RANK: %ld A0: %e Ah: %e X: %e Y: %e Z: %e\n", id, rank, A0, Ah, X, Y, Z);
				}
				count++;
			}
			fprintf(stderr, "# Reached end of file.\n");
		}

		bool correctly_initialised()
		{
			return bool_correctly_initialised;
		}

		bool is_colloids_file()
		{
			return header->is_colloids_file;
		}

		bool no_more_snapshots() {
			return bool_no_more_snapshots;
		}

		uint64_t get_num_sites()
		{
			return header->num_sites;
		}

		double get_voxelsz()
		{
			return header->voxelsz;
		}

		bool hasVelocity()
		{
			return has_velocity;
		}

		bool hasShearStress()
		{
			return has_shearstress;
		}

		bool hasPressure()
		{
			return has_pressure;
		}

		double get_scaling()
		{
			return scaling;
		}

		double get_scalar_quantity(uint32_t column_index, uint64_t site_index)
		{
			return snapshot->get(column_index, site_index);
		}

		double get_interpolated_scalar_quantity(uint32_t column_index, lattice_map *map)
		{
			double average_existing = 0;
			uint32_t num_existing = 0;
			for(uint32_t i = 0; i < 8; i++) {
				if(map->index[i].exists == true) {
					average_existing += get_scalar_quantity(column_index, map->index[i].index);
					num_existing++;
				}
			}
			if(num_existing == 0) {
				return 0;
			}
			average_existing /= num_existing;
			
			// Get the scalars at the 8 lattice sites forming the cube
			double s[8];
			for(uint32_t i = 0; i < 8; i++) {
				// If there was no corresponding site_index, the site was not measured, so set it to the average
				// value of the existing sites in the map
				if(map->index[i].exists == false) {
					s[i] = average_existing;
				} else {
					s[i] = get_scalar_quantity(column_index, map->index[i].index);
				}
			}

			// Trilinear interpolation within the cube
			return	  s[0] * (1 - map->a) * (1 - map->b) * (1 - map->c)
				+ s[1] * (map->a * (1 - map->b) * (1 - map->c))
				+ s[2] * ((1 - map->a) * map->b * (1 - map->c))
				+ s[3] * ((1 - map->a) * (1 - map->b) * map->c)
				+ s[4] * (map->a * (1 - map->b) * map->c)
				+ s[5] * ((1 - map->a) * map->b * map->c)
				+ s[6] * (map->a * map->b * (1 - map->c))
				+ s[7] * (map->a * map->b * map->c);
		}
		void get_vector_quantity(uint32_t column_index, uint64_t site_index, Vector3 *returned_val)
		{
			double vx, vy, vz;
			vx = get_scalar_quantity(column_index, site_index);
			vy = get_scalar_quantity(column_index + 1, site_index);
			vz = get_scalar_quantity(column_index + 2, site_index);
			returned_val->set(vx, vy, vz);
		}

		void get_interpolated_vector_quantity(uint32_t column_index, lattice_map *map, Vector3 *returned_val)
		{
			double vx, vy, vz;
			vx = get_interpolated_scalar_quantity(column_index, map);
			vy = get_interpolated_scalar_quantity(column_index + 1, map);
			vz = get_interpolated_scalar_quantity(column_index + 2, map);
			returned_val->set(vx, vy, vz);
		}

		void get_pressure(uint64_t site_index, double *returned_val)
		{
			*returned_val = get_scalar_quantity(column_pressure, site_index);
		}

		void get_velocity(uint64_t site_index, Vector3 *returned_val)
		{
			get_vector_quantity(column_velocity, site_index, returned_val);
		}

		void get_shearstress(uint64_t site_index, double *returned_val)
		{
			*returned_val = get_scalar_quantity(column_shearstress, site_index);
		}

		void get_interpolated_pressure(lattice_map *map, double *returned_val)
		{
			*returned_val = get_interpolated_scalar_quantity(column_pressure, map);
		}

		void get_interpolated_velocity(lattice_map *map, Vector3 *returned_val)
		{
			get_interpolated_vector_quantity(column_velocity, map, returned_val);
		}

		void get_interpolated_shearstress(lattice_map *map, double *returned_val)
		{
			*returned_val = get_interpolated_scalar_quantity(column_shearstress, map);
		}

		Site * get_sites()
		{
			return snapshot->get_sites();
		}

		SiteIndex * get_site_indices(Site *list, uint64_t list_size)
		{
			return snapshot->get_site_indices(list, list_size);
		}

		SiteIndex * get_site_indices_hashed_lookup(Site *list, uint64_t list_size)
		{
			return snapshot->get_site_indices_hashed_lookup(list, list_size);
		}

		double get_time()
		{
			return snapshot->get_timestep();
		}

		double get_time_next()
		{
			return time_next;
		}

		void print_header(FILE *outfile)
		{
			fprintf(outfile, "\n# HEADER:\n# -------\n");
			fprintf(outfile, "# version=%u\n", header->version);
			fprintf(outfile, "# voxelsz=%f\n", header->voxelsz);
			fprintf(outfile, "# originx=%f\n", header->originx);
			fprintf(outfile, "# originy=%f\n", header->originy);
			fprintf(outfile, "# originz=%f\n", header->originz);
			fprintf(outfile, "# num_sites=%lu\n", header->num_sites);
			fprintf(outfile, "# field_count=%u\n", header->field_count);
			fprintf(outfile, "# field_header_length=%u\n", header->field_header_length);
		}

		void print_field_header(FILE *outfile)
		{
			fprintf(outfile, "\n# FIELD HEADERS:\n# -------\n");
			for(unsigned int i = 0; i < header->field_count; i++) {
				fprintf(outfile, "# fieldname=%s\n# num_floats=%u\n# offset=%lf\n# ---\n", field_header[i].name, field_header[i].num_floats, field_header[i].offset);
			}
			fprintf(outfile, "# Number of 'columns' per snapshot = %d\n", header->num_columns);
		}

		void print_column_headings(FILE *outfile)
		{
			fprintf(outfile, "# step | grid_x | grid_y | grid_z");
			for(unsigned int i = 0; i < header->field_count; i++) {
				if(field_header[i].num_floats == 1) {
					fprintf(outfile, " | %s", field_header[i].name);
				} else {
					for(unsigned int j = 0; j < field_header[i].num_floats; j++) {
						fprintf(outfile, " | %s(%d)", field_header[i].name, j);
					}
				}
			}
			fprintf(outfile, "\n");
		}
		void print_stats_column_headings(FILE *outfile)
		{
			fprintf(outfile, "# step");
			for(unsigned int i = 0; i < header->field_count; i++) {
				if(field_header[i].num_floats == 1) {
					fprintf(outfile, " | %s [average | stdev | max | min]", field_header[i].name);
				} else {
					for(unsigned int j = 0; j < field_header[i].num_floats; j++) {
						fprintf(outfile, " | %s(%d) [average | stdev | max | min]", field_header[i].name, j);
					}
				}
			}
			fprintf(outfile, "\n");
		}

		void print_all(FILE *outfile)
		{
			snapshot->print(outfile);
		}

		void print_stats(FILE *outfile)
		{
			snapshot->print_stats(outfile);
		}

	private:
		/* True if file has been opened and the headers read without problem */
		bool bool_correctly_initialised;

		/* All info contained in the file header */
		HEADER *header;
		FIELD_HEADER *field_header;

		/** The step length for this file (has to be provided by the user since Heme extraction files don't store this value...) */
		double step_length;

		/** The velocity scaling factor */
		double scaling;

		/** The currently loaded snapshot */
		Snapshot *snapshot;

		/** The open extraction file's stream */
		FILE *in;

		/** The XDR reader for this file */
		XDR xdrs;

		/** The time of the next snapshot (next to be loaded) */
		double time_next;

		/** True when no more snapshots can be loaded */
		bool bool_no_more_snapshots;

		/** True if the file contains these field types */
		bool has_velocity, has_shearstress, has_pressure;

		/** The column indices in which to find each field type */
		uint32_t column_velocity;
		uint32_t column_pressure;
		uint32_t column_shearstress;

		/** Magic numbers designating a heme file, a heme extraction file and a colloid file */
	 	static const uint32_t heme_magic = 0x686C6221; // hlb!
		static const uint32_t extract_magic = 0x78747204; // xtr
		static const uint32_t colloid_magic = 0x636F6C04; // colloid

		/** Reads a HemeLB extraction file header. Checks that the magic numbers are correct. */
		int read_header()
		{
			uint32_t magic1 = 0, magic2 = 0;
			uint32_t num_sites1, num_sites2;

			// Read heme magic number
			xdr_u_int(&xdrs, &magic1);
			if(magic1 != heme_magic) {
				fprintf(stderr, "First uint32_t does not match heme magic number.\n");
				return 1;
			}

			header = new HEADER();

			// Read extraction magic number
			xdr_u_int(&xdrs, &magic2);
			if(magic2 == extract_magic) {
				fprintf(stderr, "Reading a normal extraction file.\n");

				header->is_colloids_file = false;

				xdr_u_int(&xdrs, &(header->version));
				xdr_double(&xdrs, &(header->voxelsz));
				xdr_double(&xdrs, &(header->originx));
				xdr_double(&xdrs, &(header->originy));
				xdr_double(&xdrs, &(header->originz));

				// There is something wrong with either Heme or XDR which means that xdr_u_long is not working (possibly even reading more/less than 8 bytes...)
				xdr_u_int(&xdrs, &num_sites1);
				xdr_u_int(&xdrs, &num_sites2);

				// Combine the two ints into a long (assuming bigendian-ness)
				header->num_sites = ((uint64_t)num_sites1)<<32 | num_sites2;

				xdr_u_int(&xdrs, &(header->field_count));
				xdr_u_int(&xdrs, &(header->field_header_length));

			} else if (magic2 == colloid_magic) {
				fprintf(stderr, "Reading a colloids extraction file.\n");
				header->is_colloids_file = true;

				// Check version
				uint32_t version=0;
				xdr_u_int(&xdrs, &version);
				printf("Colloids version %u\n", version);
			} else {
				fprintf(stderr, "Unknown format: Second uint32_t does not match extraction file or colloids file magic number.\n");
				return 1;
			}

			// Everything went well
			return 0;
		}

		/** Reads the field headers and remembers which columns correspond to velocity, pressure and shearstress. */
		int read_field_header()
		{
			printf("Reading field header...\n");
			int const maxlength = 100; // surely it can't be bigger than this...?
			field_header = new FIELD_HEADER[header->field_count];
			for(unsigned int i = 0; i < header->field_count; i++) {
				field_header[i].name = new char[maxlength];
				xdr_string(&xdrs, &(field_header[i].name), maxlength);
				xdr_u_int(&xdrs, &(field_header[i].num_floats));
				xdr_double(&xdrs, &(field_header[i].offset));
			}
			printf("Finished reading field header...\n");

			// Calc. number of 'columns' needed to represent a snapshot
			uint32_t num_columns = 0;
			for(unsigned int i = 0; i < header->field_count; i++) {
				// If file contains velocity, pressure or shearstress, remember the corresponding column index for that field type.
				if(strcmp(field_header[i].name, "velocity") == 0) {
					has_velocity = true;
					column_velocity = num_columns;
				} else if(strcmp(field_header[i].name, "pressure") == 0) {
					has_pressure = true;
					column_pressure = num_columns;
				} else if(strcmp(field_header[i].name, "shearstress") == 0) {
					has_shearstress = true;
					column_shearstress = num_columns;
				} else if(strcmp(field_header[i].name, "d_shearstress") == 0) { // For "legacy" reasons only. Some old MCA runs have this field.
					has_shearstress = true;
					column_shearstress = num_columns;
				}

				// This field has 'num_floats' columns to it
				num_columns += field_header[i].num_floats;
			}
			header->num_columns = num_columns;

			// Everything went well
			return 0;
		}
};

#endif
