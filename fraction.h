#ifndef FRACTION_H
#define FRACTION_H
class fraction {
public:
	int num;
	int den;
	fraction(int n =0, int d =1): num{n}, den{d}
	{
	}
	fraction& operator/=(int d)
	{
		den *= d;
		return *this;
	}
};

inline fraction operator/(const fraction& a, int b)
{
	fraction f(a);
	f /= b;
	return f;
}

#endif
