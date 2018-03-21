#include <iostream>
#include <fstream>
#include <ctime>
#include <mpirxx.h>

mpz_class Factorial1(unsigned int n)
{
	mpz_class x = 1;
	for(mpz_class i = 2;i <= n;++i)
	{
		x *= i; // 1 * 2 * 3 * ... * n
	}
	return x;
}

mpz_class Factorial2(unsigned int n)
{
	mpz_class k = (n / 2) + 1; // select appropriate k
	mpz_class m = n;
	mpz_class s = 0;
	mpz_class result = k;

	m -= k;
	k *= k;
	for(mpz_class i = 1;i <= m;++i)
	{
		s += ((i << 1) - 1); // s = 1 , 4 , 9 , ...
		result *= (k - s);
	}
	return result; // n! = k * (k^2 - 1) * (k^2 - 4) * (k^2 - 9) * ...
}

mpz_class Factorial3(unsigned int n)
{
	mpz_class x = 1;
	if (n == 0) return 1;
	if (n < 3) return n;

	// this algorithm depends on following formulae
	// n! = 2^floor(n/2) * (floor(n/2))! * 1 * 3 * 5 * ...

	for(mpz_class y = 1;y <= n;y += 2)
	{
		x *= y; // 1 * 3 * 5 * ...
	}
	return (x * Factorial3(n >> 1)) << (n >> 1);
}

mpz_class mul_odd(unsigned int high,unsigned int low)
{
	if ((high - low) <= 4)
	{
		if ((high - low) == 0)
		{
			return high;
		}
		else if ((high - low) == 2)
		{
			return mpz_class(high) * low;
		}
		return mpz_class(high) * low * (low + 2);
	}
	uint64_t m = high;
	m = (m + low) >> 1;
	if ((m & 1) == 0) --m;
	return mul_odd(high, m + 2 /* (m + 2) might not fit in unsigned int */) * mul_odd(m, low);
}

mpz_class Factorial4(unsigned int n)
{
	if (n < 2) return 1;
	return (Factorial4(n >> 1) * mul_odd((n & 1) ? n : (n - 1),1)) << (n >> 1);
}

mpz_class mul_odd(uint64_t upper,uint64_t lower)
{
	if ((upper - lower) <= 4)
	{
		if ((upper - lower) == 0)
		{
			return upper;
		}
		else if ((upper - lower) == 2)
		{
			return upper * lower;
		}
		return mpz_class(upper * lower) * (lower + 2);
	}
	uint64_t m = (upper + lower) >> 1;
	if ((m & 1) == 0) --m;
	return mul_odd(upper,m + 2) * mul_odd(m,lower);
}

mpz_class Factorial5(unsigned int n)
{
	unsigned int l = 0;
	unsigned int total_count_of_even = 0;
	unsigned int end = 1,start = 1;
	for(unsigned int k = 1;k <= n;k <<= 1) ++l; // number of bits of n
	mpz_class x = 1,y = 1;
	if (n < 2) return 1;
	if (n < 3) return n;
	for(unsigned int k = l;k >= 2;k--)
	{
		end = n >> (k - 2); // now end will be [n / 2^(k - 2)]
		total_count_of_even += (end >> 1); // (end >> 1) will be counts of even number less than or equal to [n / 2^(k - 2)]
		end = (end & 1) ? end : end - 1; // end must be odd number
		y *= mul_odd(end, start);
		// y now holds 1 * 3 * 5 * ... * [n / 2^(k - 2)]
		x *= y; // x will be Го_[ q = 1 to [log_2(n) - 1] ] ( Го_[ p = 1 to [n / 2^(l - q - 1)] ] (2*p - 1) )
		start = end + 2; // update lower bound
	}
	return x << total_count_of_even;
}

mpz_class Factorial6(unsigned int n)
{
	// this is just wrapper of mpz_fac_ui
	mpz_t m;
	mpz_init(m);
	mpz_fac_ui(m,n);
	return mpz_class(m);
}

typedef mpz_class (*FACTORIAL_FUNCP)(unsigned int);

unsigned int run_test(FACTORIAL_FUNCP func,unsigned int func_arg,int test_count)
{
	clock_t e,s;
	unsigned int t = 0;
	for(int i = 1;i <= test_count;++i)
	{
		s = clock();
		func(func_arg);
		e = clock();
		t += (e - s);
	}
	return t / test_count;
}

int main(void)
{
	int test_count = 10;
	std::ofstream ofs;

	ofs.open("factorial_test_0-50000.csv");
	ofs << "n,Algorithm1,Algorithm2,Algorithm3,Algorithm4,Algorithm5,mpz_fac_ui" << std::endl;

	std::cout << "Test for algorithm1 to algorithm5" << std::endl;
	for(unsigned int n = 0;n <= 50000;n += 1000)
	{
		std::cout << "Testing " << n << "!\r";
		ofs << n;
		ofs << ',' << run_test(Factorial1,n,test_count);
		ofs << ',' << run_test(Factorial2,n,test_count);
		ofs << ',' << run_test(Factorial3,n,test_count);
		ofs << ',' << run_test(Factorial4,n,test_count);
		ofs << ',' << run_test(Factorial5,n,test_count);
		ofs << ',' << run_test(Factorial6,n,test_count);
		ofs << std::endl;
	}
	ofs.close();

	ofs.open("factorial_test_50000-500000.csv");
	ofs << "n,Algorithm4,Algorithm5,mpz_fac_ui" << std::endl;

	std::cout << std::endl << "Test for algorithm4 and algorithm5" << std::endl;
	for(unsigned int n = 50000;n <= 500000;n += 10000)
	{
		std::cout << "Testing " << n << "!\r";
		ofs << n;
		ofs << ',' << run_test(Factorial4,n,test_count);
		ofs << ',' << run_test(Factorial5,n,test_count);
		ofs << ',' << run_test(Factorial6,n,test_count);
		ofs << std::endl;
	}
	ofs.close();

	ofs.open("factorial_test_500000-1000000.csv");
	ofs << "n,Algorithm5,mpz_fac_ui" << std::endl;

	std::cout << std::endl <<  "Test for algorithm5" << std::endl;
	for(unsigned int n = 500000;n <= 1000000;n += 10000)
	{
		std::cout << "Testing " << n << "!\r";
		ofs << n;
		ofs << ',' << run_test(Factorial5,n,test_count);
		ofs << ',' << run_test(Factorial6,n,test_count);
		ofs << std::endl;
	}
	ofs.close();
	return 0;
}
