#include <iostream>
#include <boost/random.hpp>

#include <cg/operations/orientation.h>
#include <cg/io/point.h>

cg::orientation_t double_turn(cg::point_2 const & a, cg::point_2 const & b, cg::point_2 const & c)
{
   cg::vector_2 ab = b - a;
   cg::vector_2 ac = c - a;

   double r = ab.x * ac.y - ac.x * ab.y;
   
   if (r > 0)
      return cg::CG_LEFT;

   if (r < 0)
      return cg::CG_RIGHT;

   return cg::CG_COLLINEAR;
}

int main(int argc, char ** argv)
{
   boost::random::mt19937 gen;
   boost::random::uniform_real_distribution<> distr;

   for (size_t l = 0; l != 1000000; ++l)
   {
      double m = ((1LL << 20) - 1.) / (1LL << 10);
      cg::point_2 a(distr(gen) * m, distr(gen) * m);
      cg::point_2 b(distr(gen) * m, distr(gen) * m);

      cg::point_2 c = a + (b - a) * (distr(gen) * ((1LL << 3) - 1));

      cg::orientation_t
         td = double_turn(a, b, c),
         pr = *cg::orientation_r()(a, b, c);

      if (td && td != pr)
      {
         std::cout << std::setprecision(15) << std::fixed 
                   << a << " " << b << " " << c << std::endl
                   << "double turn: " << td << std::endl
                   << "rational predicate: " << pr << std::endl
                   << std::endl;

         break;
      }
   }
}