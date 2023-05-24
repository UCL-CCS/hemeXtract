#ifndef INCLUDED_HEMELBEXTRACTIONFILE_H
#define INCLUDED_HEMELBEXTRACTIONFILE_H

#include <cstdint>
#include <cstdio>
#include <memory> // unique_ptr

#include <rpc/types.h> // must precede xdr.h
#include <rpc/xdr.h> // XDR

#include "Vector3.h" // Vector3
#include "Site.h" // Site
#include "HemeLBExtractionFileTypes.h" // lattice_map, SiteIndex
#include "Snapshot.h" // Snapshot

class HemeLBExtractionFile
{
	public:
		// Constructor
		HemeLBExtractionFile(char *fname, 
							 double step_length, 
							 double scaling, 
							 bool verbose, 
							 Vector3 *translate);
		// Destructor
		~HemeLBExtractionFile();

		bool read_time_next();

		int load_next_snapshot();

		void read_and_print_colloids(FILE *outfile);

		bool correctly_initialised() const;

		bool is_colloids_file() const;

		bool no_more_snapshots() const;

		uint64_t get_num_sites() const;

		double get_voxelsz() const;

		bool hasVelocity() const;
		bool hasShearStress() const ;
		bool hasPressure() const;

		double get_scaling() const;

		double get_scalar_quantity(uint32_t column_index, uint64_t site_index) const;

		double get_interpolated_scalar_quantity(
				uint32_t column_index, 
				const lattice_map &map) const;

		void get_vector_quantity(
				uint32_t column_index, 
				uint64_t site_index, 
				Vector3 *returned_val) const;

		void get_interpolated_vector_quantity(
				uint32_t column_index, 
				const lattice_map &map, 
				Vector3 *returned_val) const;

		void get_pressure(uint64_t site_index, double *returned_val) const;

		void get_velocity(uint64_t site_index, Vector3 *returned_val) const;

		void get_shearstress(uint64_t site_index, double *returned_val) const;

		void get_interpolated_pressure(const lattice_map &map, double *returned_val) const;

		void get_interpolated_velocity(const lattice_map &map, Vector3 *returned_val) const;

		void get_interpolated_shearstress(const lattice_map &map, double *returned_val) const;

		const Site * get_sites() const;

		SiteIndex * get_site_indices(Site *list, uint64_t list_size);
		SiteIndex * get_site_indices_hashed_lookup(Site *list, uint64_t list_size);

		double get_time() const;
		double get_time_next() const;

		void print_header(FILE *outfile) const;
		void print_field_header(FILE *outfile) const;
		void print_column_headings(FILE *outfile) const;
		void print_stats_column_headings(FILE *outfile) const;
		void print_all(FILE *outfile) const;
		void print_stats(FILE *outfile) const;

	private:
		/* True if file has been opened and the headers read without problem */
		bool bool_correctly_initialised{false};

		/* True if user wants verbose output. If false, means quiet. */
		bool bool_verbose;

		/* All info contained in the file header */
		std::unique_ptr<HEADER> header{nullptr};
		std::vector<FIELD_HEADER> field_header;

		/** The step length for this file (has to be provided by the user since Heme extraction files don't store this value...) */
		double step_length;

		/** The velocity scaling factor */
		double scaling;

		/** The currently loaded snapshot */
		std::unique_ptr<Snapshot> snapshot{nullptr};

		/** The open extraction file's stream */
		FILE *in;

		/** The XDR reader for this file */
		XDR xdrs;

		/** The time of the next snapshot (next to be loaded) */
		double time_next;

		/** True when no more snapshots can be loaded */
		bool bool_no_more_snapshots;

		/** True if the file contains these field types */
		bool has_velocity{false};
		bool has_shearstress{false};
		bool has_pressure{false};

		/** The column indices in which to find each field type */
		uint32_t column_velocity;
		uint32_t column_pressure;
		uint32_t column_shearstress;

		/** The translation vector (in grid units) to be applied to this lattice (if requested by --translate option) */
		int32_t translate_grid_x{0};
		int32_t translate_grid_y{0};
		int32_t translate_grid_z{0};

		/** Reads a HemeLB extraction file header. Checks that the magic numbers are correct. */
		int read_header();

		/** Reads the field headers and remembers which columns correspond to velocity, pressure and shearstress. */
		int read_field_header();
};

#endif
