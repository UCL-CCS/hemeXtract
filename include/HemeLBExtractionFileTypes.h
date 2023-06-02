#ifndef INCLUDED_HEMELBEXTRACTIONFILETYPES_H
#define INCLUDED_HEMELBEXTRACTIONFILETYPES_H

#include <cstdint>

typedef struct {
	uint32_t version;
	double voxelsz;
	double originx;
	double originy;
	double originz;
	uint64_t num_sites;
	uint32_t field_count;
	uint32_t field_header_length;
	uint32_t num_columns;

	bool is_colloids_file;
	uint32_t num_particles;
} HEADER;

typedef struct {
	char *name;
	uint32_t num_floats;
	double offset;
} FIELD_HEADER;

class SiteIndex
{
	public:
		SiteIndex() {}
		uint64_t index{0};
		bool exists{false};
};

typedef struct
{
    SiteIndex index[8];
    double a, b, c;
} lattice_map;

#endif
