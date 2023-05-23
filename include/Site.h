#ifndef INCLUDED_SITE_H
#define INCLUDED_SITE_H

#include <cstdio>
#include <cstdint>

class Site 
{
	public:

		Site() {}

		void set(uint32_t x, uint32_t y, uint32_t z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		void print(FILE *outfile) {
			fprintf(outfile, "%u %u %u", x, y, z);
		}

		void print(FILE *outfile, double voxelsz) {
			fprintf(outfile, "%f %f %f", x * voxelsz, y * voxelsz, z * voxelsz);
		}

		inline bool equals(uint32_t a, uint32_t b, uint32_t c)
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

		bool equals(Site *s)
		{
			return equals(s->x, s->y, s->z);
		}

		uint32_t x{0}, y{0}, z{0};
};

#endif
