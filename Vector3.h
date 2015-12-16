#ifndef INCLUDED_VECTOR3_H
#define INCLUDED_VECTOR3_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rpc/xdr.h>
#include <argp.h>
#include <vector>
#include <math.h>
#include <string.h>
#include <tr1/unordered_map>
#include <string>
#include <sstream>

/** Reinventing the wheel... */
class Vector3
{
	public:
		Vector3()
		{
			x = 0;
			y = 0;
			z = 0;
		}
		Vector3(double x, double y, double z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		void set(double x, double y, double z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		void scale(double s)
		{
			this->x *= s;
			this->y *= s;
			this->z *= s;
		}

		double dot(Vector3 *v)
		{
			return this->x*v->x + this->y*v->y + this->z*v->z;
		}

		double length()
		{
			return sqrt(x*x + y*y + z*z);
		}

		void normalise()
		{
			double l = length();

			if(l < tol) {
				x = 0;
				y = 0;
				z = 0;
				return;
			}

			x /= l;
			y /= l;
			z /= l;
		}

		double abs_diff(Vector3 *v)
		{
			double dx = x - v->x;
			double dy = y - v->y;
			double dz = z - v->z;
			return sqrt(dx * dx + dy * dy + dz * dz);
		}

		void add(Vector3 *v, double scale)
		{
			this->x += v->x * scale;
			this->y += v->y * scale;
			this->z += v->z * scale;
		}

		void print(FILE *outfile)
		{
			fprintf(outfile, "(%f %f %f)\n", x, y, z);
		}

	private:
		static const double tol = 1e-10;
		double x, y, z;
};

#endif
