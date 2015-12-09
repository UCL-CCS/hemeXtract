#ifndef INCLUDED_HEMELBEXTRACTIONFILETYPES_H
#define INCLUDED_HEMELBEXTRACTIONFILETYPES_H

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
} HEADER;

typedef struct {
	char *name;
	uint32_t num_floats;
	double offset;
} FIELD_HEADER;

class SiteIndex
{
	public:
		SiteIndex()
		{
			index = 0;
			exists = false;
		}
		uint64_t index;
		bool exists;
};

typedef struct
{
        SiteIndex index[8];
        double a, b, c;
} lattice_map;

#endif
