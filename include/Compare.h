#ifndef INCLUDED_COMPARE_H
#define INCLUDED_COMPARE_H

#include <cstdio>

#include "Vector3.h"
#include "HemeLBExtractionFileTypes.h" // lattice_map
#include "HemeLBExtractionFile.h" // HemeLBExtractionFile

/** Compares two lattices, A and B, with mapping mapA_to_B, by calculating their crosscorrelation, and their L2 norm difference. */
void compare(FILE *outfile, 
			 lattice_map *mapA_to_B, 
			 HemeLBExtractionFile *A, 
			 HemeLBExtractionFile *B, 
			 int minexistent, 
			 bool normalize_correl);

/** Calculates the difference between each site in A, and the corresponding (trilinearly interpolated) point in B */
void diff(FILE *outfile, 
		  lattice_map *mapA_to_B, 
		  HemeLBExtractionFile *A, 
		  HemeLBExtractionFile *B, 
		  int minexistent, 
		  Vector3 *project, 
		  bool relativeErr, 
		  bool verbose);

#endif
