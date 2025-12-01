#ifndef BIGFIXED_MATH_WRAPPERS_H
#define BIGFIXED_MATH_WRAPPERS_H

#include "bigfixed_core.h"

/*
 * BigFixed Math Wrappers
 * Wrapper functions to use transcendental functions with BigFixed
 */

void bigfixed_ln(BigFixed* result, const BigFixed* x, int precision);
void bigfixed_sqrt(BigFixed* result, const BigFixed* x, int precision);
void bigfixed_exp(BigFixed* result, const BigFixed* x, int precision);
void bigfixed_tanh(BigFixed* result, const BigFixed* x, int precision);

#endif /* BIGFIXED_MATH_WRAPPERS_H */
