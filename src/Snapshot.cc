#include "Snapshot.h"

#include <cstdlib>
#include <cmath>
#include <unordered_map>
#include <string>
#include <sstream>
#include <vector>

// Calculate statistics (helper function)
inline void calc_stats(
		const std::vector<double> &records, 
		double &average, double &stdev, double &max, double &min)
{
	double sum{0}, sum2{0};
	max = -INFINITY;
	min =  INFINITY;
	for(const double& rec : records) {
		sum += rec;
		sum2 += rec * rec;
		if(max < rec) max = rec;
		if(min > rec) min = rec;
	}
	average = sum/records.size();
	stdev = std::sqrt(sum2/records.size() - average*average);
}

//
// Column (helper class)
//
class Column 
{
	public:
		Column(unsigned int num_records_, double offset_)
			: records(num_records_), offset(offset_) {}

		double get(uint64_t index)
		{
			if(index >= records.size()) {
				fprintf(stderr, "Error: index (%lu) exceeds number of records in column (%lu)\n", index, records.size());
				exit(1);
			}
			return records[index];
		}
		void set_plus_offset(uint64_t index, float value)
		{
			if(index >= records.size()) {
				fprintf(stderr, "Error: index (%lu) exceeds number of records in column (%lu)\n", index, records.size());
				exit(1);
			}
			records[index] = value + offset;
			stats_calcd = false;
		}
		void print(FILE *outfile, uint64_t index) {
			if(index >= records.size()) {
				fprintf(stderr, "Error: index (%lu) exceeds number of records in column (%lu)\n", index, records.size());
				exit(1);
			}
			fprintf(outfile, "%e", records[index]);
		}
		void print_stats(FILE *outfile) {
			if (!stats_calcd) {
				calc_stats(records,average,stdev,max,min);
				stats_calcd = true;
			}
			fprintf(outfile, "%e %e %e %e", average, stdev, max, min);
		}

	private:
		std::vector<double> records;
		double offset;
		bool stats_calcd{false};
		double average;
		double stdev;
		double max, min;
};

Snapshot::Snapshot(HEADER *header, FIELD_HEADER *field_header)
	: num_sites(header->num_sites),
	  num_columns(header->num_columns),
	  voxelsz(header->voxelsz)
{
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

void Snapshot::set_timestep(double timestep)
{
	this->timestep = timestep;
}
double Snapshot::get_timestep()
{
	return timestep;
}

void Snapshot::site_set(unsigned int site_index, uint32_t x, uint32_t y, uint32_t z)
{
	sites[site_index].set(x, y, z);
}

void Snapshot::column_set_plus_offset(uint32_t column_index, uint64_t site_index, double value)
{
	if(column_index >= num_columns) {
		fprintf(stderr, "Error: column index (%u) exceeds number of columns (%u)\n", column_index, num_columns);
		exit(1);
	}
	columns[column_index]->set_plus_offset(site_index, value);
}

double Snapshot::get(uint32_t column_index, uint64_t site_index)
{
	return columns[column_index]->get(site_index);
}

bool Snapshot::get_site_index(uint32_t a, uint32_t b, uint32_t c, uint64_t *site_index)
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

int Snapshot::get_site_index(Site *s, uint64_t *site_index)
{
	return get_site_index(s->x, s->y, s->z, site_index);
}

Site * Snapshot::get_sites()
{
	return sites;
}

/* Very inefficient calculation of site_index corresponding to the given site. Only suitable for small data sets. */
SiteIndex * Snapshot::get_site_indices(Site *list, uint64_t list_size)
{
	SiteIndex *indices = new SiteIndex[list_size];
	for(uint64_t i = 0; i < list_size; i++) {
		indices[i].exists = get_site_index(&list[i], &(indices[i].index));
	}
	return indices;
}

/** Builds hashtable to get indices corresponding to the given site. Necessary for very large data sets. */
SiteIndex * Snapshot::get_site_indices_hashed_lookup(Site *list, uint64_t list_size, bool bool_verbose)
{
	// Build the hash table
	if(bool_verbose == true) { fprintf(stderr, "# Building hashtable...\n"); }
	std::unordered_map<std::string, uint64_t> hashtable;
	for(uint64_t i = 0; i < num_sites; i++) {
		std::ostringstream oss;
		oss << sites[i].x << "," << sites[i].y << "," << sites[i].z;
		std::string hashstr = oss.str();
		hashtable[hashstr] = i;
	}
	if(bool_verbose == true) { fprintf(stderr, "# ...done building hashtable\n"); }

	if(bool_verbose == true) { fprintf(stderr, "# Calculating mapping...\n"); }
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
	if(bool_verbose == true) { fprintf(stderr, "# ... done calculating mapping\n"); }

	return indices;
}

void Snapshot::print(FILE *outfile)
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

void Snapshot::print_stats(FILE *outfile)
{
	fprintf(outfile, "%f", timestep);
	for(uint32_t c = 0; c < num_columns; c++) {
		fprintf(outfile, " ");
		columns[c]->print_stats(outfile);
	}
	fprintf(outfile, "\n");
}

