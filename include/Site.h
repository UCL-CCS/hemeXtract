#ifndef INCLUDED_SITE_H
#define INCLUDED_SITE_H

#include <cstdio>
#include <cstdint>

class Site 
{
	public:

		Site() {}
		Site(uint32_t x_, uint32_t y_, uint32_t z_)
			: x(x_), y(y_), z(z_) {}

		void set(uint32_t x, uint32_t y, uint32_t z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		void print(FILE *outfile) const {
			fprintf(outfile, "%u %u %u", x, y, z);
		}

		void print(FILE *outfile, double voxelsz) const {
			fprintf(outfile, "%f %f %f", x * voxelsz, y * voxelsz, z * voxelsz);
		}

		inline bool equals(uint32_t a, uint32_t b, uint32_t c) const
		{
			if(a != x) {
				return false;
			}
			if(b != y) {
				return false;
			}
			if(c != z) {
				return false;
			}
			return true;
		}

		bool equals(const Site &s) const
		{
			return equals(s.x, s.y, s.z);
		}

		uint32_t x{0}, y{0}, z{0};
};

#endif
