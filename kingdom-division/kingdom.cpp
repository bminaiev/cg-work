#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>
#include <algorithm>
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

void make_graph(vector<triangle> const & all_triangles, polygon & points, vector<triangle> & small_triangles, vector<vector<int> > & graph) {
   map<long long, int> betw;
   map<long long, int> triangle_id;
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

void rearrange_triangles(vector<triangle> & small_triangles, vector<vector<int> > const & graph) {
   int n = small_triangles.size();
   vector<bool> was(n);
   vector<triangle> new_small_triangles;
   int cur = 0;
   while (true) {
      was[cur] = true;
      new_small_triangles.push_back(small_triangles[cur]);
      if (!was[graph[cur][0]]) {
         cur = graph[cur][0];
      } else {
         if (!was[graph[cur][1]]) {
            cur = graph[cur][1];
         } else {
            break;
         }
      }
   }
   small_triangles.clear();
   for (int i = 0; i < new_small_triangles.size(); i++) 
      small_triangles.push_back(new_small_triangles[i]);
}

int different_point(triangle const & t1, triangle const & t2) {
   for (int i = 0; i < 3; i++) {
      if (t1[i] != t2[0] && t1[i] != t2[1] && t1[i] != t2[2])
         return t1[i];
   }
   return -1;
}

long long edge(int v, int u) {
   if (u > v) 
      swap(u, v);
   return 1000000000LL * v + u;
}

void make_polygon(int fr, int addV, vector<triangle> const & small_triangles, polygon & points, polygon & ans, double need_sq) {
   double cur_sq = 0;
   int st = fr;
   int interesting_points[2];
   int stop_id = 0;
   vector<long long> edges;
   while (true) {
      triangle t = small_triangles[fr];
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
         stop_id = fr;
         interesting_points[0] = different_point(t, small_triangles[fr + addV]);
         interesting_points[1] = different_point(t, small_triangles[fr - addV]);
         double ostS = need_sq - cur_sq;
         double divide_pr = ostS / (0.0 + add_sq);
         point_d _p11 = points[interesting_points[0]];
         point_d _p22 = points[interesting_points[1]];
         double last_point_x = _p11.x + (_p22.x - _p11.x) * divide_pr;
         double last_point_y = _p11.y + (_p22.y - _p11.y) * divide_pr;
         points.push_back(point_d(last_point_x, last_point_y));
         int last_point = t[0] + t[1] + t[2] - interesting_points[0] - interesting_points[1];
         edges.push_back(edge(interesting_points[0], last_point));
         edges.push_back(edge(interesting_points[0], points.size() - 1));
         edges.push_back(edge(last_point, points.size() - 1));
         break;
      }
      fr += addV;
      cur_sq += add_sq;
   }
   long long interesting_edge = edge(interesting_points[0], interesting_points[1]);
   for (int fr = st; fr != stop_id; fr += addV) {
      for (int i = 0; i < 3; i++) {
         int v1 = small_triangles[fr][i];
         int v2 = small_triangles[fr][(i + 1) % 3];
         long long e = edge(v1, v2);
         if (e == interesting_edge) {
            edges.push_back(edge(interesting_points[0], points.size() - 1));
            edges.push_back(edge(interesting_points[1], points.size() - 1));
         } else {
            edges.push_back(e);
         }
      }
   }
   sort(edges.begin(), edges.end());
   vector<long long> ok_edges;
   for (int i = 0; i < edges.size(); i++) {
      bool ok = true;
      if (i > 0 && edges[i - 1] == edges[i])
         ok = false;
      if (i < edges.size() - 1 && edges[i + 1] == edges[i])
         ok = false;
      if (ok)
         ok_edges.push_back(edges[i]);
   }
   vector<vector<int> > g;
   for (int i = 0; i < points.size(); i++) {
      vector<int> tmp;
      g.push_back(tmp);
   }
   for (int i = 0; i < ok_edges.size(); i++) {
      int v1 = ok_edges[i] % 1000000000LL;
      int v2 = ok_edges[i] / 1000000000LL;
      g[v1].push_back(v2);
      g[v2].push_back(v1);
   }
   int start_vertex = 0;
   for (int i = 0; i < points.size(); i++) {
      if (g[i].size() != 0)
         start_vertex = i;
   }
   vector<bool> was_vertex(points.size());
   while (true) {
      ans.push_back(points[start_vertex]);
      was_vertex[start_vertex] = true;
      if (!was_vertex[g[start_vertex][0]]) {
         start_vertex = g[start_vertex][0];
      } else {
         if (!was_vertex[g[start_vertex][1]]) {
            start_vertex = g[start_vertex][1];
         } else {
            break;
         }
      }
   }
}

void find_answer(polygon & points, vector<triangle> & small_triangles, polygon & ans1, polygon & ans2, int n) {
   double need_sq = 0;
   for (int i = 0; i < n; i++) { 
      point_d p1 = points[i];
      point_d p2 = points[(i + 1) % n];
      need_sq += (p1.x - p2.x) * (p1.y + p2.y) / 4.0;
   }
   make_polygon(0, 1, small_triangles, points, ans1, need_sq);
   make_polygon(small_triangles.size() - 1, -1, small_triangles, points, ans2, need_sq);
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
   make_graph(triangles, p, small_triangles, graph);
   rearrange_triangles(small_triangles, graph);
   polygon p1, p2;
   find_answer(p, small_triangles, p1, p2, n);
   out << p1 << p2;
}