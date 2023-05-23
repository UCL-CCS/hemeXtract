#ifndef INCLUDED_MAPPING_H
#define INCLUDED_MAPPING_H

#include "HemeLBExtractionFileTypes.h" // lattice_map
#include "HemeLBExtractionFile.h" // HemeLBExtractionFile

/* Returns a mapping for each site in A to a corresponding 
 * trilinearly interpolated position in B. 
 */
lattice_map *get_mapping(HemeLBExtractionFile *A, HemeLBExtractionFile *B);

#endif
