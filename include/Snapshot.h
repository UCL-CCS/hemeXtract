#ifndef INCLUDED_SNAPSHOT_H
#define INCLUDED_SNAPSHOT_H

#include <cstdio>
#include <cstdint>

#include "Site.h" // Site
#include "HemeLBExtractionFileTypes.h" // HEADER, FIELD_HEADER, SiteIndex

class Column; // Forward declaration (could be private potentially)

class Snapshot {
	public:
		Snapshot(HEADER *header, FIELD_HEADER *field_header);

		void set_timestep(double timestep);
		double get_timestep();

		void site_set(unsigned int site_index, uint32_t x, uint32_t y, uint32_t z);

		void column_set_plus_offset(uint32_t column_index, uint64_t site_index, double value);

		double get(uint32_t column_index, uint64_t site_index);

		bool get_site_index(uint32_t a, uint32_t b, uint32_t c, uint64_t *site_index);

		int get_site_index(Site *s, uint64_t *site_index);

		Site *get_sites();

		/* Very inefficient calculation of site_index corresponding to the 
		 * given site. Only suitable for small data sets. */
		SiteIndex *get_site_indices(Site *list, uint64_t list_size);

		/* Builds hashtable to get indices corresponding to the given 
		 * site. Necessary for very large data sets. */
		SiteIndex *get_site_indices_hashed_lookup(Site *list, 
				uint64_t list_size, bool bool_verbose);

		void print(FILE *outfile);
		void print_stats(FILE *outfile);

	private:
		double timestep;
		uint64_t num_sites;
		uint32_t num_columns;
		double voxelsz;
		Column **columns;
		Site *sites;
};

#endif
