#include "Mapping.h"

#include <cstdio>
#include <cmath>
#include <cstdint>

inline void map(
		double old_voxelsz, 
		double new_voxelsz, 
		uint32_t in_coord, 
		uint32_t *out_root, 
		double *out_interpolate)
{
	double real_pos = in_coord * old_voxelsz;
	*out_root = std::floor(real_pos/new_voxelsz);
	*out_interpolate = (real_pos - *out_root * new_voxelsz) / new_voxelsz;
}

lattice_map * get_mapping(HemeLBExtractionFile *A, HemeLBExtractionFile *B)
{
	// Work out mapping between the two lattices
	uint64_t num_sites_A = A->get_num_sites();
	Site *sitesA = A->get_sites();
	double voxelA = A->get_voxelsz();
	double voxelB = B->get_voxelsz();
	uint32_t mapped_x, mapped_y, mapped_z;
	lattice_map *mapping_A_B = new lattice_map[num_sites_A];
	if(mapping_A_B == NULL) {
		fprintf(stderr, "Could not allocate memory for lattice_map\n");
		return NULL;
	}
	Site *mapped_sites = new Site[num_sites_A * 8];
	if(mapped_sites == NULL) {
		fprintf(stderr, "Could not allocate memory for mapped_sites\n");
		return NULL;
	}

	for(uint64_t i = 0; i < num_sites_A; i++) {
		// Get the interpolation constants for this site in terms of the other lattice
		map(voxelA, voxelB, sitesA[i].x, &mapped_x, &mapping_A_B[i].a);
		map(voxelA, voxelB, sitesA[i].y, &mapped_y, &mapping_A_B[i].b);
		map(voxelA, voxelB, sitesA[i].z, &mapped_z, &mapping_A_B[i].c);

		// Add the 8 sites forming the cube this point lies within
		mapped_sites[i * 8    ].set(mapped_x    , mapped_y    , mapped_z    ); // (000) root
		mapped_sites[i * 8 + 1].set(mapped_x + 1, mapped_y    , mapped_z    ); // (100)
		mapped_sites[i * 8 + 2].set(mapped_x    , mapped_y + 1, mapped_z    ); // (010)
		mapped_sites[i * 8 + 3].set(mapped_x    , mapped_y    , mapped_z + 1); // (001)
		mapped_sites[i * 8 + 4].set(mapped_x + 1, mapped_y    , mapped_z + 1); // (101)
		mapped_sites[i * 8 + 5].set(mapped_x    , mapped_y + 1, mapped_z + 1); // (011)
		mapped_sites[i * 8 + 6].set(mapped_x + 1, mapped_y + 1, mapped_z    ); // (110)
		mapped_sites[i * 8 + 7].set(mapped_x + 1, mapped_y + 1, mapped_z + 1); // (111)
	}

	SiteIndex *site_indices = B->get_site_indices_hashed_lookup(mapped_sites, num_sites_A * 8);

	for(uint64_t i = 0; i < num_sites_A; i++) {
		for(uint32_t j = 0; j < 8; j++) {
			mapping_A_B[i].index[j] = site_indices[i * 8 + j];
		}
	}

	return mapping_A_B;
}
