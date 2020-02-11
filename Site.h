#ifndef INCLUDED_SITE_H
#define INCLUDED_SITE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rpc/xdr.h>
#include <argp.h>
#include <vector>
#include <math.h>
#include <string.h>
//#include <tr1/unordered_map>
#include <unordered_map>
#include <string>
#include <sstream>

class Site {
	public:
		Site()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
		}

		void set(uint32_t x, uint32_t y, uint32_t z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		uint32_t x, y, z;

		void print(FILE *outfile) {
			fprintf(outfile, "%u %u %u", x, y, z);
		}

		void print(FILE *outfile, double voxelsz) {
			fprintf(outfile, "%f %f %f", x * voxelsz, y * voxelsz, z * voxelsz);
		}

		bool equals(uint32_t a, uint32_t b, uint32_t c)
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
};

#endif
