#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#define F	((1 << 14))	// fixed point 1
#define INT_MAX	((1 << 31) - 1)
#define INT_MIN ((1 << 31))
// x and y denote fixed_point numbers in 17.14 format
// n is an integer.

int int_to_fp(int);			/* change integer int ofp. */
int fp_to_int_round(int);	/* change fp into integer within round. */
int fp_to_int(int);			/* change fp into tinteger. */
int add_fp(int, int);		/* addition between fps. */
int add_mixed(int, int);	/* addition between fp and integer. */
int sub_fp(int, int);		/* subtraction between fps. */
int sub_mixed(int, int);	/* subtraction between fp and integer. */
int mult_fp(int, int);		/* multiply between fps. */
int mult_mixed(int, int);	/* multiply between fp and integer. */
int div_fp(int, int);		/* divide x by y. */
int div_mixed(int, int);	/* divide x by integer. */

int int_to_fp(int n)
{
	return (n * F);
}

int fp_to_int_round(int x)
{
	return (x >= 0 ? (x + (F/2))/F : (x - (F/2))/F);
}

int fp_to_int(int x)
{
	return (x / F);
}

int add_fp(int x, int y)
{
	return (x + y);
}

int add_mixed(int x, int n)
{
	return (x + (n * F));
}

int sub_fp(int x, int y)
{
	return (x - y);
}

int sub_mixed(int x, int n)
{
	return (x - (n * F));
}

int mult_fp(int x, int y)
{
	return ((((int64_t)x) * y) / F);
}

int mult_mixed(int x, int n)
{
	return (x * n);
}

int div_fp(int x, int y)
{
	return ((((int64_t)x) * F) / y);
}

int div_mixed(int x, int n)
{
	return (x / n);
}


#endif	// FIXED_POINT_H_INCLUDED
