#pragma once
#include "common.hpp"
/**
 * @brief Wight function for MPS method presented by [Koshizuka and Oka, 1996](https://doi.org/10.13182/nse96-a24205)
 * @param dis distance of each particle \f$r\f$. It must be positive.
 * @param re effective radius  \f$r_e\f$
 * @return weight \f[w(r) = \frac{r_e}{r} - 1\f]
 */
double weight(double dis, double re) {
	assert(dis > 0.0);

	if (dis >= re) {
		return 0.0;

	} else {
		return (re / dis) - 1.0;
	}
}
