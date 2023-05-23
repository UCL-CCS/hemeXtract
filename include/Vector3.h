#ifndef INCLUDED_VECTOR3_H
#define INCLUDED_VECTOR3_H

#include <cstdio>
#include <cmath>

/** Reinventing the wheel... */
class Vector3
{
	public:

		Vector3() {}
		
		Vector3(double x_, double y_, double z_)
			: x(x_), y(y_), z(z_) {}

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

		double get_x()
		{
			return this->x;
		}

		double get_y()
		{
			return this->y;
		}

		double get_z()
		{
			return this->z;
		}

		void print(FILE *outfile)
		{
			fprintf(outfile, "(%f %f %f)\n", x, y, z);
		}

	private:
		static constexpr double tol{1e-10}; //JM for C++ 11
		double x{0}, y{0}, z{0};
};

#endif
