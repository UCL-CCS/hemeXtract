#ifndef INCLUDED_COMPARE_H
#define INCLUDED_COMPARE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rpc/xdr.h>
#include <argp.h>
#include <vector>
#include <math.h>

/** Compares two lattices, A and B, with mapping mapA_to_B, by calculating their crosscorrelation, and their L2 norm difference. */
void compare(FILE *outfile, lattice_map *mapA_to_B, HemeLBExtractionFile *A, HemeLBExtractionFile *B, bool only_consider_existing_sites)
{
	uint64_t num_sites_A = A->get_num_sites();
	double timeA = A->get_time();
	double timeB = B->get_time();

	double presL2, shearstressL2;
	double max_vel_A, max_vel_B;
	double correl_vel, correl_shearstress, correl_pres;

	bool do_velocity = A->hasVelocity() && B->hasVelocity();
	bool do_shearstress = A->hasShearStress() && B->hasShearStress();
	bool do_pressure = A->hasPressure() && B->hasPressure();

	uint64_t num_nonexistent = 0;

	if(do_velocity) {
		Vector3 velA, velB;
		double dot_sum_vel = 0;
		max_vel_A = -INFINITY;
		max_vel_B = -INFINITY;
		for(uint64_t i = 0; i < num_sites_A; i++) {
			if(only_consider_existing_sites == true) {
				bool bad = true;
				for(uint64_t j = 0; j < 8; j++) {
					if(mapA_to_B[i].index[j].exists == true) {
						bad = false;
						break;
					}
				}
				if(bad == true) {
					num_nonexistent++;
					// skip site
					continue;
				}
			}

			// Velocity
			A->get_velocity(i, &velA);
			B->get_interpolated_velocity(&mapA_to_B[i], &velB);

			// Max amplitude
			if(velA.length() > max_vel_A) {
				max_vel_A = velA.length();
			}
			if(velB.length() > max_vel_B) {
				max_vel_B = velB.length();
			}

			// Correlation
			velA.normalise();
			velB.normalise();
			dot_sum_vel += velA.dot(&velB);
		}
		correl_vel = dot_sum_vel / (num_sites_A - num_nonexistent);
	}

	if(do_shearstress) {
		double shearstressA, shearstressB;
		double autocorrelA = 0, autocorrelB = 0, crosscorrelAB = 0;
		shearstressL2 = 0;
		for(uint64_t i = 0; i < num_sites_A; i++) {
			// Cross correlation shearstress
			A->get_shearstress(i, &shearstressA);
			B->get_interpolated_shearstress(&mapA_to_B[i], &shearstressB);
			crosscorrelAB += shearstressA * shearstressB;

			// Autocorrelations
			autocorrelA += shearstressA * shearstressA;
			autocorrelB += shearstressB * shearstressB;

			// L2
			shearstressL2 += (shearstressA - shearstressB) * (shearstressA - shearstressB);
		}
		correl_shearstress = crosscorrelAB / (sqrt(autocorrelA) * sqrt(autocorrelB));
		shearstressL2 = sqrt(shearstressL2);
	}

	if(do_pressure) {
		double presA, presB;
		double autocorrelA = 0, autocorrelB = 0, crosscorrelAB = 0;
		presL2 = 0;
		for(uint64_t i = 0; i < num_sites_A; i++) {
			// Pressure
			A->get_pressure(i, &presA);
			B->get_interpolated_pressure(&mapA_to_B[i], &presB);

			autocorrelA += presA * presA;
			autocorrelB += presB * presB;
			crosscorrelAB += presA * presB;
			presL2 += (presA - presB) * (presA - presB);
		}
		correl_pres = crosscorrelAB / (sqrt(autocorrelA) * sqrt(autocorrelB));
		presL2 = sqrt(presL2);
	}

	fprintf(outfile, "%f %f %f %f %f %f %f %f %f\n", timeA, timeB, correl_vel, max_vel_A, max_vel_B, correl_shearstress, shearstressL2, correl_pres, presL2);
}

/** Calculates the difference between each site in A, and the corresponding (trilinearly interpolated) point in B */
void diff(FILE *outfile, lattice_map *mapA_to_B, HemeLBExtractionFile *A, HemeLBExtractionFile *B, bool only_consider_existing_sites)
{
	uint64_t num_sites_A = A->get_num_sites();
	double timeA = A->get_time();
	double timeB = B->get_time();
	bool do_velocity = A->hasVelocity() && B->hasVelocity();
	double voxelA = A->get_voxelsz();
	uint64_t num_nonexistent = 0;

	if(do_velocity) {
		fprintf(outfile, "# timeA=%f timeB=%f\n", timeA, timeB);
		Vector3 velA, velB;
		for(uint64_t i = 0; i < num_sites_A; i++) {
			if(only_consider_existing_sites == true) {
				bool bad = true;
				for(uint64_t j = 0; j < 8; j++) {
					if(mapA_to_B[i].index[j].exists == true) {
						bad = false;
						break;
					}
				}
				if(bad == true) {
					num_nonexistent++;
					// skip site
					continue;
				}
			}

			// Get velocity from A and corresponding interpolated velocity from B
			A->get_velocity(i, &velA);
			B->get_interpolated_velocity(&mapA_to_B[i], &velB);

			// Calculate magnitude of difference between them
			double abs_diff = velA.abs_diff(&velB);
			A->get_sites()[i].print(outfile, voxelA);
			fprintf(outfile, " %f\n", abs_diff);
		}
	}
}

#endif
