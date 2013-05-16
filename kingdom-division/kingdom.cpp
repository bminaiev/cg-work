#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>
#include <map>

using namespace std;

struct point_d
{
   double x, y;

   point_d(double x, double y) : x(x), y(y) {}
};

struct polygon 
{
   vector<point_d> points;

   point_d const& operator [] (size_t idx) const
   {
      return points[idx];
   }

   size_t size() {
      return points.size();
   }

   void push_back(point_d const & p) {
      points.push_back(p);
   }
};

istream & operator >> (istream & in, polygon & p)
{
   size_t n = 0;
   in >> n;

   for (size_t i = 0; i < n; i++)
   {
      double x, y;
      in >> x >> y;
      p.push_back(point_d(x * 12, y * 12));
   }

   return in;
}

ostream & operator << (ostream & out, polygon const & p)
{
   out.precision(18);
   out << p.points.size() << endl;

   for (size_t i = 0; i < p.points.size(); i++)
   {
      out << fixed << p[i].x / 12. << " " << p[i].y / 12 << endl;
   }

   return out;
}

struct triangle
{
   triangle() {}
   triangle(int a, int b, int c) 
   {
      pts[0] = a; pts[1] = b; pts[2] = c;
   }

   int &         operator [] (size_t id)       { return pts[id]; }
   int const &   operator [] (size_t id) const { return pts[id]; }

private:
   int pts[3];
};

int prev(int v, vector<bool> const & cut_vertices) {
   int n = cut_vertices.size();
   v = (v - 1 + n) % n;
   while (cut_vertices[v]) 
      v = (v - 1 + n) % n;
   return v;
}

int next(int v, vector<bool> const & cut_vertices) {
   int n = cut_vertices.size();
   v = (v + 1) % n;
   while (cut_vertices[v])
      v = (v + 1) % n;
   return v;
}

int orientation(point_d a, point_d b, point_d c) {
   double l = (b.x - a.x) * (c.y - a.y);
   double r = (b.y - a.y) * (c.x - a.x);
   double res = l - r;
   return (res > 0 ? 1 : (res < 0 ? -1 : 0));
}

bool inside_triangle(triangle t, int pt, polygon const & points) {
   return 
   orientation(points[t[0]], points[t[1]], points[pt]) >= 0 &&
   orientation(points[t[1]], points[t[2]], points[pt]) >= 0 &&
   orientation(points[t[2]], points[t[0]], points[pt]) >= 0;
}

bool update_can_cut(int v, polygon & p, vector<bool> & cut_vertices) {
   int pr = prev(v, cut_vertices);
   int ne = next(v, cut_vertices);
   if (orientation(p[pr], p[v], p[ne]) <= 0) {
      return false;
   }
   bool ok = true;
   int n = p.size();
   triangle tr(pr, v, ne);
   for (int i = 0; i < n; i++) {
      if (i == pr || i == ne || i == v || cut_vertices[i])
         continue;
      if (inside_triangle(tr, i, p)) {
         ok = false;
         break;
      }
   }
   return ok;
}



int get_betw(int p1, int p2, map<long long, int> & betw, polygon & points) {
   if (p1 > p2) 
      swap(p1, p2);
   long long hash= p1 * 1000000000LL + p2;
   int id = betw[hash];
   if (id == 0) {
      double x = (points[p1].x + points[p2].x) / 2.0;
      double y = (points[p1].y + points[p2].y) / 2.0;
      points.push_back(point_d(x, y));
      betw[hash] = points.size() - 1;
      return points.size() - 1;
   } else {
      return id;
   }
}

void save_tringle_side_connection(int p1, int p2, int trId, map<long long, int> & triangle_id) {
   if (p1 > p2)
      swap(p1, p2);
   long long hash = 1000000000LL * p1 + p2;
   triangle_id[hash] += 1 + trId;
}

int get_another_triangle_on_same_side(int p1, int p2, int trId, map<long long, int> & triangle_id) {
   if (p1 > p2)
      swap(p1, p2);
   long long hash = 1000000000LL * p1 + p2;
   int xx = triangle_id[hash];
   if (xx == trId + 1)
      return -1;
   return xx - trId - 2;
}



void make_answer(int k, point_d last, vector<int> & chain1, vector<int> & chain2, polygon & ans, vector<triangle> const & small_triangles, polygon const & p) {
   triangle t = small_triangles[k];
   int diff = t[0] + t[1] + t[2] - chain1[chain1.size() - 1] - chain2[chain2.size() - 1];
   if (chain1.size() > 1 && chain1[chain1.size() - 2] == diff)
      chain1.pop_back();
   if (chain2.size() > 1 && chain2[chain2.size() - 2] == diff)
      chain2.pop_back();
   ans.push_back(p[diff]);
   for (int i = 0; i < chain1.size(); i++) {
      ans.push_back(p[chain1[chain1.size() - 1 - i]]);
   }
   ans.push_back(last);
   for (int i = 0; i < chain2.size(); i++) {
      ans.push_back(p[chain2[i]]);
   }
}

bool is_vertex_on_triangle(int v, int t, vector<triangle> const & small_triangles) 
{
   return small_triangles[t][0] == v || small_triangles[t][1] == v || small_triangles[t][2] == v;
}

vector<int> get_points_same_for_triangles(int tr1, int tr2, vector<triangle> const & small_triangles) 
{
   vector<int> res;
   for (int i = 0; i < 3; i++) {
      if (is_vertex_on_triangle(small_triangles[tr1][i], tr2, small_triangles)) 
         res.push_back(small_triangles[tr1][i]);
   }
   return res;
}

bool is_point_interesting(int v, int* interesting_points) 
{
   return interesting_points[0] == v || interesting_points[1] == v;
}

void dfs(int v, int p, vector<triangle> const & small_triangles, vector<vector<int> > const & g, polygon const & points, vector<bool> & used_trinalges, double & cur_sq, double need_sq, int* interesting_points, point_d & last, vector<int> & chain1, vector<int> & chain2, map<long long, int> & triangle_id) {
   used_trinalges[v] = true;
   triangle t = small_triangles[v];
   point_d p1 = points[t[0]];
   point_d p2 = points[t[1]];
   point_d p3 = points[t[2]];
   double add_sq = 0;
   add_sq += (p1.x - p2.x)  * (p1.y + p2.y) / 2.0;
   add_sq += (p2.x - p3.x)  * (p3.y + p2.y) / 2.0;
   add_sq += (p3.x - p1.x)  * (p1.y + p3.y) / 2.0;
   if (add_sq < 0)
      add_sq = -add_sq;
   if (add_sq + cur_sq >= need_sq) {
      used_trinalges[v] = false;
      for (int i = 0; i < 3; i++) {
         int p11 = t[i];
         int p22 = t[(i + 1) % 3];
         int anoth = get_another_triangle_on_same_side(p11, p22, v, triangle_id);
         if (anoth != g[v][0] && anoth != g[v][1]) {
            interesting_points[0] = p11;
            interesting_points[1] = p22;
            double ostS = need_sq - cur_sq;
            double divide_pr = ostS / (0.0 + add_sq);
            if (small_triangles[p][0] == p22 || small_triangles[p][1] == p22 || small_triangles[p][2] == p22) 
               swap(p11, p22);
            point_d _p11 = points[p11];
            point_d _p22 = points[p22];
            double last_point_x = _p11.x + (_p22.x - _p11.x) * divide_pr;
            double last_point_y = _p11.y + (_p22.y - _p11.y) * divide_pr;
            last.x = last_point_x;
            last.y = last_point_y;
            vector<int> sam = get_points_same_for_triangles(v, p, small_triangles);
            chain1.push_back(sam[0]);
            chain2.push_back(sam[1]);     
            break;
         }
      }
      return;
   }
   for (int i = 0; i < g[v].size(); i++) {
      int to = g[v][i];
      if (used_trinalges[to])
         continue;
      cur_sq += add_sq;
      dfs(to, v, small_triangles, g, points, used_trinalges, cur_sq, need_sq, interesting_points, last, chain1, chain2, triangle_id);
      if (p != -1) {
         int diff = t[0] + t[1] + t[2] - chain1[chain1.size() - 1] - chain2[chain2.size() - 1];
         if (is_vertex_on_triangle(chain1[chain1.size() - 1], p, small_triangles)) {
            if (chain2.size() > 1 && chain2[chain2.size() - 2] == diff) {
               chain2.pop_back(); 
            } else {
               if (chain2.size() > 0 && is_point_interesting(chain2[chain2.size() - 1], interesting_points) && is_point_interesting(diff, interesting_points))
                  chain2.pop_back();
               chain2.push_back(diff); 
            }
         } else {
            if (chain1.size() > 1 && chain1[chain1.size() - 2] == diff) {
               chain1.pop_back();
            } else {
               if (chain1.size() > 0 && is_point_interesting(chain1[chain1.size() - 1], interesting_points) && is_point_interesting(diff, interesting_points))
                  chain1.pop_back();
               chain1.push_back(diff);
            }
         }
      }
      return;
   }
}


void triangulate(polygon p, vector<triangle> & triangles) {
   int n = p.size();
   bool can_cut[n];
   vector<bool> cut_vertices(n, false);
   for (int i = 0; i < n; i++) 
      cut_vertices[i] = false;
   for (int i = 0; i < n; i++)
      can_cut[i] = update_can_cut(i, p, cut_vertices);
   for (int it = 0; it < n - 2; it++) {
      int rem = -1;
      for (int i= 0; i < n; i++) {
         if (!cut_vertices[i] && can_cut[i]) {
            rem = i;
         }
      }
      int pr = prev(rem, cut_vertices);
      int ne = next(rem, cut_vertices);
      cut_vertices[rem] = true;
      triangles.push_back(triangle(pr, rem, ne));
      can_cut[pr] = update_can_cut(pr, p, cut_vertices);
      can_cut[ne] = update_can_cut(ne, p, cut_vertices);
   }
}

void make_graph(vector<triangle> const & all_triangles, polygon & points, vector<triangle> & small_triangles, vector<vector<int> > & graph, map<long long, int> & triangle_id) {
   map<long long, int> betw;
   for (int i = 0; i < all_triangles.size(); i++) {
      triangle t = all_triangles[i];
      int m1 = get_betw(t[0], t[1], betw, points);
      int m2 = get_betw(t[1], t[2], betw, points);
      int m3 = get_betw(t[2], t[0], betw, points);
      double xx = (points[t[0]].x + points[t[1]].x + points[t[2]].x) / 3.0;
      double yy = (points[t[0]].y + points[t[1]].y + points[t[2]].y) / 3.0;
      int med = points.size();
      points.push_back(point_d(xx, yy));
      small_triangles.push_back(triangle(t[0], m1, med));
      small_triangles.push_back(triangle(t[1], m1, med));
      small_triangles.push_back(triangle(t[1], m2, med));
      small_triangles.push_back(triangle(t[2], m2, med));
      small_triangles.push_back(triangle(t[2], m3, med));
      small_triangles.push_back(triangle(t[0], m3, med));
   }
   for (int i = 0; i < small_triangles.size(); i++) {
      vector<int> tmp;
      graph.push_back(tmp);
      triangle t = small_triangles[i];
      save_tringle_side_connection(t[0], t[1], i, triangle_id);
      save_tringle_side_connection(t[1], t[2], i, triangle_id);
      save_tringle_side_connection(t[2], t[0], i, triangle_id);
   }
   for (int i = 0; i < small_triangles.size(); i++) {
      triangle t = small_triangles[i];
      int getAn = get_another_triangle_on_same_side(t[0], t[1], i, triangle_id);
      if (getAn == -1) {
         int get_an2 = get_another_triangle_on_same_side(t[1], t[2], i, triangle_id);
         graph[i].push_back(get_an2);
      } else {
         graph[i].push_back(getAn);
      }
      int getAn3 = get_another_triangle_on_same_side(t[2], t[0], i, triangle_id);
      graph[i].push_back(getAn3);
   }
}

void find_answer(polygon & points, vector<triangle> & small_triangles, vector<vector<int> > & graph, polygon & ans1, polygon & ans2, int n, map<long long, int> triangle_id) {
   double need_sq = 0;
   for (int i = 0; i < n; i++) { 
      point_d p1 = points[i];
      point_d p2 = points[(i + 1) % n];
      need_sq += (p1.x - p2.x) * (p1.y + p2.y) / 4.0;
   }
   vector<bool> used_trinalges(small_triangles.size());
   vector<int> chain1;
   vector<int> chain2;
   point_d last(0.0, 0.0);
   double cur_sq = 0;
   int interesting_points[2];
   dfs(0, -1, small_triangles, graph, points, used_trinalges, cur_sq, need_sq, interesting_points, last, chain1, chain2, triangle_id);
   make_answer(0, last, chain1, chain2, ans1, small_triangles, points);
   chain1.clear();
   chain2.clear();
   cur_sq = 0;
   dfs(graph[0][1], -1, small_triangles, graph, points, used_trinalges, cur_sq, need_sq, interesting_points, last, chain1, chain2, triangle_id);
   make_answer(graph[0][1], last, chain1, chain2, ans2, small_triangles, points);
}


int main() {
   ifstream in("kingdom.in");
   ofstream out("kingdom.out");
   polygon p;
   in >> p;
   int n = p.size();
   vector<triangle> triangles;
   triangulate(p, triangles);
   vector<vector<int> > graph;
   vector<triangle> small_triangles;
   map<long long, int> triangle_id;
   make_graph(triangles, p, small_triangles, graph, triangle_id);
   polygon p1, p2;
   find_answer(p, small_triangles, graph, p1, p2, n, triangle_id);
   out << p1 << p2;
}