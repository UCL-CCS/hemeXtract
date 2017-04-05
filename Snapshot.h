#ifndef INCLUDED_SNAPSHOT_H
#define INCLUDED_SNAPSHOT_H

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
#include "Site.h"
#include "Mapping.h"

class Column {
	public:
		Column(unsigned int num_records, double offset)
		{
			this->num_records = num_records;
			this->offset = offset;
			stats_calcd = false;
			records = new double[num_records];
		}
		~Column()
		{
			delete[] records;
		}

		double get(uint64_t index)
		{
			if(index >= num_records) {
				fprintf(stderr, "Error: index (%lu) exceeds number of records in column (%lu)\n", index, num_records);
				exit(1);
			}
			return records[index];
		}
		void set_plus_offset(uint64_t index, float value)
		{
			if(index >= num_records) {
				fprintf(stderr, "Error: index (%lu) exceeds number of records in column (%lu)\n", index, num_records);
				exit(1);
			}
			records[index] = value + offset;
			stats_calcd = false;
		}
		void print(FILE *outfile, uint64_t index) {
			if(index >= num_records) {
				fprintf(stderr, "Error: index (%lu) exceeds number of records in column (%lu)\n", index, num_records);
				exit(1);
			}
			fprintf(outfile, "%e", records[index]);
		}
		void print_stats(FILE *outfile) {
			calc_stats();
			fprintf(outfile, "%e %e %e %e", average, stdev, max, min);
		}

	private:
		uint64_t num_records;
		double *records;
		double offset;
		bool stats_calcd;
		double average;
		double stdev;
		double max, min;
		void calc_stats()
		{
			if(!stats_calcd) {
				double sum = 0, sum2 = 0;
				max = -INFINITY;
				min = INFINITY;
				for(uint64_t i = 0; i < num_records; i++) {
					sum += records[i];
					sum2 += records[i] * records[i];
					if(max < records[i]) {
						max = records[i];
					}
					if(min > records[i]) {
						min = records[i];
					}
				}
				average = sum / num_records;
				stdev = sqrt(sum2/num_records - average*average);
				stats_calcd = true;
			}
		}
};

class Snapshot {
	public:
		Snapshot(HEADER *header, FIELD_HEADER *field_header)
		{
			this->num_sites = header->num_sites;
			this->num_columns = header->num_columns;
			this->voxelsz = header->voxelsz;
			sites = new Site[num_sites];
			columns = new Column*[num_columns];
			unsigned int c = 0;
			for(unsigned int i = 0; i < header->field_count; i++) {
				for(unsigned int bro=0; bro < field_header[i].num_floats; bro++) {
					columns[c] = new Column(num_sites, field_header[i].offset);
					c++;
				}
			}
		}

		void set_timestep(double timestep)
		{
			this->timestep = timestep;
		}
		double get_timestep()
		{
			return timestep;
		}

		void site_set(unsigned int site_index, uint32_t x, uint32_t y, uint32_t z)
		{
			sites[site_index].set(x, y, z);
		}

		void column_set_plus_offset(uint32_t column_index, uint64_t site_index, double value)
		{
			if(column_index >= num_columns) {
				fprintf(stderr, "Error: column index (%u) exceeds number of columns (%u)\n", column_index, num_columns);
				exit(1);
			}
			columns[column_index]->set_plus_offset(site_index, value);
		}

		double get(uint32_t column_index, uint64_t site_index)
		{
			return columns[column_index]->get(site_index);
		}

		bool get_site_index(uint32_t a, uint32_t b, uint32_t c, uint64_t *site_index)
		{
			for(unsigned int i = 0; i < num_sites; i++) {
				if(sites[i].equals(a, b, c)) {
					*site_index = i;
					return true;
				}
			}

			// If no such site can be found, return false
			return false;
		}

		int get_site_index(Site *s, uint64_t *site_index)
		{
			return get_site_index(s->x, s->y, s->z, site_index);
		}

		Site * get_sites()
		{
			return sites;
		}

		/* Very inefficient calculation of site_index corresponding to the given site. Only suitable for small data sets. */
		SiteIndex * get_site_indices(Site *list, uint64_t list_size)
		{
			SiteIndex *indices = new SiteIndex[list_size];
			for(uint64_t i = 0; i < list_size; i++) {
				indices[i].exists = get_site_index(&list[i], &(indices[i].index));
			}
			return indices;
		}

		/** Builds hashtable to get indices corresponding to the given site. Necessary for very large data sets. */
		SiteIndex * get_site_indices_hashed_lookup(Site *list, uint64_t list_size)
		{
			// Build the hash table
			fprintf(stderr, "# Building hashtable...\n");
			std::tr1::unordered_map<std::string, uint64_t> hashtable;
			for(uint64_t i = 0; i < num_sites; i++) {
				std::ostringstream oss;
				oss << sites[i].x << "," << sites[i].y << "," << sites[i].z;
				std::string hashstr = oss.str();
				hashtable[hashstr] = i;
			}
			fprintf(stderr, "# ...done building hashtable\n");

			fprintf(stderr, "# Calculating mapping...\n");
			SiteIndex *indices = new SiteIndex[list_size];
			for(uint64_t i = 0; i < list_size; i++) {
				std::ostringstream oss;
				oss << list[i].x << "," << list[i].y << "," << list[i].z;
				std::string hashstr = oss.str();
				if(hashtable.find(hashstr) != hashtable.end()) {
					indices[i].exists = true;
					indices[i].index = hashtable[hashstr];
				} else {
					indices[i].exists = false;
				}
			}
			fprintf(stderr, "# ... done calculating mapping\n");
			return indices;
		}

		void print(FILE *outfile)
		{
			for(uint64_t s = 0; s < num_sites; s++) {
				fprintf(outfile, "%f ", timestep);
				sites[s].print(outfile, this->voxelsz);
				for(uint32_t c = 0; c < num_columns; c++) {
					fprintf(outfile, " ");
					columns[c]->print(outfile, s);
				}
				fprintf(outfile, "\n");
			}
			fprintf(outfile, "\n");
		}
		void print_stats(FILE *outfile)
		{
			fprintf(outfile, "%f", timestep);
			for(uint32_t c = 0; c < num_columns; c++) {
				fprintf(outfile, " ");
				columns[c]->print_stats(outfile);
			}
			fprintf(outfile, "\n");
		}
	private:
		double timestep;
		uint64_t num_sites;
		uint32_t num_columns;
		double voxelsz;
		Column **columns;
		Site *sites;
};

#endif
