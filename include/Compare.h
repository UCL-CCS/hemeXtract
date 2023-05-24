#ifndef INCLUDED_COMPARE_H
#define INCLUDED_COMPARE_H

#include <cstdio>

#include "Vector3.h"
#include "HemeLBExtractionFileTypes.h" // lattice_map
#include "HemeLBExtractionFile.h" // HemeLBExtractionFile

/** Compares two lattices, A and B, with mapping mapA_to_B, by calculating their crosscorrelation, and their L2 norm difference. */
void compare(FILE *outfile, 
			 const lattice_map *mapA_to_B, 
			 const HemeLBExtractionFile *A, 
			 const HemeLBExtractionFile *B, 
			 int minexistent, 
			 bool normalize_correl);

/** Calculates the difference between each site in A, and the corresponding (trilinearly interpolated) point in B */
void diff(FILE *outfile, 
		  const lattice_map *mapA_to_B, 
		  const HemeLBExtractionFile *A, 
		  const HemeLBExtractionFile *B, 
		  int minexistent, 
		  Vector3 *project, 
		  bool relativeErr, 
		  bool verbose);

#endif
