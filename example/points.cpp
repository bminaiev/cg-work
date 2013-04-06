#include <iostream>
#include <boost/random.hpp>
#include <gmpxx.h>

int main(int argc, char ** argv) {
 boost::random::mt19937 gen;
 boost::random::uniform_real_distribution<> distr(0, 100);


 // mpz_t = integer
 mpz_t a, b; 

 mpz_init_set_str(a, "123", 10);
 mpz_init_set_str(b, "345", 10);
 mpz_add(a, a, b);
 printf("%s\n", mpz_get_str(NULL, 10, a));
 mpz_mul(a, a, b);
 printf("%s\n", mpz_get_str(NULL, 10, a));


 // mpq_t = rational


 // mpf_t = float
 mpf_t float_val, var2;
 mpf_init_set_d(float_val, 123.0);
 mpf_init_set_str(var2, "123.00", 10);
 int x = mpf_cmp(float_val, var2);
 double dVal = mpf_get_d(float_val);
 printf("%.20f %d\n", dVal, x);

 // mpz_class
 mpz_class t("123", 10);
 std::cout << "t is eq to " << mpz_get_str(NULL, 10, t.get_mpz_t()) << std::endl;
}