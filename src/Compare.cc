#include "Compare.h"

#include <cstdint>

/** Compares two lattices, A and B, with mapping mapA_to_B, by calculating their crosscorrelation, and their L2 norm difference. */
void compare(FILE *outfile, lattice_map *mapA_to_B, HemeLBExtractionFile *A, HemeLBExtractionFile *B, int minexistent, bool normalize_correl)
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

	uint64_t num_skipped = 0;

	if(do_velocity) {
		Vector3 velA, velB;
		double dot_sum_vel = 0, sum_mags = 0;
		max_vel_A = -INFINITY;
		max_vel_B = -INFINITY;
		for(uint64_t i = 0; i < num_sites_A; i++) {
			if(minexistent > 0) {
				int existent = 0;
				for(uint64_t j = 0; j < 8; j++) {
					if(mapA_to_B[i].index[j].exists == true) {
						existent += 1;
					}
				}
				if(existent < minexistent) {
					num_skipped++;
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

			if(normalize_correl == true) {
				// Normalize velocity vectors
				velA.normalise();
				velB.normalise();
			}

			// Correlation
			sum_mags += velA.length() * velB.length();
			dot_sum_vel += velA.dot(&velB);

		}
		// Calc correlation
		if(sum_mags != 0) {
			correl_vel = dot_sum_vel / sum_mags;
		} else {
			correl_vel = 1.0; // Choose the correlation to be 1 if all vectors are zero
		}
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
void diff(FILE *outfile, lattice_map *mapA_to_B, HemeLBExtractionFile *A, HemeLBExtractionFile *B, int minexistent, Vector3 *project, bool relativeErr, bool verbose)
{
	uint64_t num_sites_A = A->get_num_sites();
	double timeA = A->get_time();
	double timeB = B->get_time();
	bool do_velocity = A->hasVelocity() && B->hasVelocity();
	bool do_shearstress = A->hasShearStress() && B->hasShearStress();
	double voxelA = A->get_voxelsz();
	uint64_t num_skipped = 0;

	if(do_velocity) {

		if(verbose == true) {
			fprintf(stderr, "Velocity difference calc\n");
			fprintf(outfile, "# timeA=%f timeB=%f\n", timeA, timeB);
		}
		Vector3 velA, velB;
		for(uint64_t i = 0; i < num_sites_A; i++) {
			if(minexistent > 0) {
				int existent = 0;
				for(uint64_t j = 0; j < 8; j++) {
					if(mapA_to_B[i].index[j].exists == true) {
						existent += 1;
					}
				}
				if(existent < minexistent) {
					num_skipped++;
					// skip site
					continue;
				}
			}

			// Get velocity from A and corresponding interpolated velocity from B
			A->get_velocity(i, &velA);
			B->get_interpolated_velocity(&mapA_to_B[i], &velB);

//			fprintf(stderr, "velA ");
//			velA.print(stderr);
//			fprintf(stderr, "velB ");
//			velB.print(stderr);

			// Print out site coords
			A->get_sites()[i].print(outfile, voxelA);

			if(relativeErr == false) {
				if(project == NULL) {
					// Calculate magnitude of difference between vectors
					double abs_diff = velA.abs_diff(&velB);
					fprintf(outfile, " %f\n", abs_diff);
				} else {
					double proj_diff = velB.dot(project) - velA.dot(project);
					fprintf(outfile, " %f\n", proj_diff);
				}
			} else { // Calculate error relative to inputfile A
				if(project == NULL) {
					double abs_diff = velA.abs_diff(&velB);
					printf("AAA velA=%f velB=%f abs_diff=%f rel=%f\n", velA.length(), velB.length(), abs_diff, abs_diff/velA.length());
					fprintf(outfile, " %f\n", abs_diff/velA.length());
				} else {
					double proj_diff = velB.dot(project) - velA.dot(project);
					fprintf(outfile, " %f\n", proj_diff/velA.dot(project));
				}
			}
		}
	}

	if(do_shearstress) {
		if(verbose == true) {
			fprintf(stderr, "WSS difference calc\n");
			fprintf(outfile, "# timeA=%f timeB=%f\n", timeA, timeB);
		}
		double shearA, shearB;
		for(uint64_t i = 0; i < num_sites_A; i++) {
			if(minexistent > 0) {
				int existent = 0;
				for(uint64_t j = 0; j < 8; j++) {
					if(mapA_to_B[i].index[j].exists == true) {
						existent += 1;
					}
				}
				if(existent < minexistent) {
					num_skipped++;
					// skip site
					continue;
				}
			}

			// Get shearstress from A and corresponding interpolated shearstress from B
			A->get_shearstress(i, &shearA);
			B->get_interpolated_shearstress(&mapA_to_B[i], &shearB);

			// Print out site coords
			A->get_sites()[i].print(outfile, voxelA);

			double sheardiff = shearA - shearB;

			// If requested, give difference relative to local WSS from input file A
			if(relativeErr == true) {
				sheardiff /= shearA;
			}

			fprintf(outfile, " %f\n", sheardiff);
		}
	}
}
