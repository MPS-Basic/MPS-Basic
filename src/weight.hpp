#pragma once

#include "common.hpp"

enum class WeightType { SemiImplicit, Explicit, ExplicitGrad };

/**
 * @class Weight
 * @brief Provides a weight function
 *
 * The class supports different types of weight calculations, as specified by the WeightType enumeration.
 *
 * @see WeightType
 */
class Weight {
private:
    WeightType weightType;

public:
    Weight() = default;
    Weight(WeightType weightType);

    /**
     * @brief Weight function for MPS method
     *
     * The weight function is defined as:
     * - \f[w(r) = \frac{r_e}{r} - 1\f] for semi-implicit method
     * - \f[w(r) = \frac{r_e}{r} - \frac{r}{r_e}\f] for the gradient calculations in explicit method
     * - \f[w(r) = \frac{r_e}{r} + \frac{r}{r_e} - 2\f] for the other calculations in explicit method
     *
     * Weight function for semi-implicit method is presented by [Koshizuka and Oka,
     * 1996](https://doi.org/10.13182/nse96-a24205) Weight functions for explicit method is presented by [Yamada, et
     * al., 2011](https://doi.org/10.3327/taesj.J10.033)
     *
     * @param dis distance of each particle \f$r\f$. It must be positive.
     * @param re effective radius  \f$r_e\f$
     * @return weight
     */
    double weightFn(double dis, double re);
};
