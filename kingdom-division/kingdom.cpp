#include <iostream>
#include <cstdio>
#include <vector>
#include <map>

using namespace std;

struct point
{
	int x, y;

   point(int x, int y) : x(x), y(y) {}

   bool operator !=  (point an) {return an.x != x || an.y != y;}
};

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

const int N = 5000;

int n;
vector<point> points;
vector<triangle> all_triangles;
vector<triangle> small_triangles;
vector<vector<int> > g;
map<long long, int> betw;
map<long long, int> triangle_id;
bool cut_vertices[N];
bool can_cut[N];
long long cur_sq, need_sq;
vector<bool> used_trinalges;
double last_point_x;
double last_point_y;
vector<int> chain1;
vector<int> chain2;
int interesting_points[2];

int prev(int v) {
	v = (v - 1 + n) % n;
	while (cut_vertices[v]) 
		v = (v - 1 + n) % n;
	return v;
}

int next(int v) {
	v = (v + 1) % n;
	while (cut_vertices[v])
		v = (v + 1) % n;
	return v;
}

int orientation(point a, point b, point c) {
	long long l = (b.x - a.x) * 1LL * (c.y - a.y);
   long long r = (b.y - a.y) * 1LL * (c.x - a.x);
   long long res = l - r;
   return (res > 0 ? 1 : (res < 0 ? -1 : 0));
}

bool inside_triangle(triangle t, int pt) {
	return 
		orientation(points[t[0]], points[t[1]], points[pt]) >= 0 &&
		orientation(points[t[1]], points[t[2]], points[pt]) >= 0 &&
		orientation(points[t[2]], points[t[0]], points[pt]) >= 0;
}

void update_can_cut(int v) {
	int pr = prev(v);
	int ne = next(v);
	if (orientation(points[pr], points[v], points[ne]) <= 0) {
		can_cut[v] = false;
		return;
	}
	bool ok = true;
	triangle tr(pr, v, ne);
	for (int i = 0; i < n; i++) {
		if (i == pr || i == ne || i == v || cut_vertices[i])
			continue;
		if (inside_triangle(tr, i)) {
			ok = false;
			break;
		}
	}
	can_cut[v] = ok;
}

int get_betw(int p1, int p2) {
	if (p1 > p2) 
		swap(p1, p2);
	long long hash= p1 * 1000000000LL + p2;
	int id = betw[hash];
	if (id == 0) {
		int x = (points[p1].x + points[p2].x) / 2;
		int y = (points[p1].y + points[p2].y) / 2;
		points.push_back(point(x, y));
		betw[hash] = points.size() - 1;
		return points.size() - 1;
	} else {
		return id;
	}
}

void save_tringle_side_connection(int p1, int p2, int trId) {
	if (p1 > p2)
		swap(p1, p2);
	long long hash = 1000000000LL * p1 + p2;
	triangle_id[hash] += 1 + trId;
}

int get_another_triangle_on_same_side(int p1, int p2, int trId) {
	if (p1 > p2)
		swap(p1, p2);
	long long hash = 1000000000LL * p1 + p2;
	int xx = triangle_id[hash];
	if (xx == trId + 1)
		return -1;
	return xx - trId - 2;
}

bool is_vertex_on_triangle(int v, int t) {
	return small_triangles[t][0] == v || small_triangles[t][1] == v || small_triangles[t][2] == v;
}

vector<int> get_points_same_for_triangles(int tr1, int tr2) {
	vector<int> res;
	for (int i = 0; i < 3; i++)
		if (is_vertex_on_triangle(small_triangles[tr1][i], tr2)) 
			res.push_back(small_triangles[tr1][i]);
	return res;
}

bool is_point_interesting(int v) {
	return interesting_points[0] == v || interesting_points[1] == v;
}

void dfs(int v, int p) {
	used_trinalges[v] = true;
	triangle t = small_triangles[v];
	point p1 = points[t[0]];
	point p2 = points[t[1]];
	point p3 = points[t[2]];
	long long add_sq = 0;
	add_sq += (p1.x - p2.x) * 1LL * (p1.y + p2.y) / 2;
	add_sq += (p2.x - p3.x) * 1LL * (p3.y + p2.y) / 2;
	add_sq += (p3.x - p1.x) * 1LL * (p1.y + p3.y) / 2;
	if (add_sq < 0)
		add_sq = -add_sq;
	if (add_sq + cur_sq >= need_sq) {
		used_trinalges[v] = false;
		for (int i = 0; i < 3; i++) {
			int p11 = t[i];
			int p22 = t[(i + 1) % 3];
			int anoth = get_another_triangle_on_same_side(p11, p22, v);
			if (anoth != g[v][0] && anoth != g[v][1]) {
				interesting_points[0] = p11;
				interesting_points[1] = p22;
				long long ostS = need_sq - cur_sq;
				double divide_pr = ostS / (0.0 + add_sq);
				if (small_triangles[p][0] == p22 || small_triangles[p][1] == p22 || small_triangles[p][2] == p22) 
					swap(p11, p22);
				point _p11 = points[p11];
				point _p22 = points[p22];
				last_point_x = _p11.x + (_p22.x - _p11.x) * divide_pr;
				last_point_y = _p11.y + (_p22.y - _p11.y) * divide_pr;
				vector<int> sam = get_points_same_for_triangles(v, p);
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
		dfs(to, v);
		if (p != -1) {
			int diff = t[0] + t[1] + t[2] - chain1[chain1.size() - 1] - chain2[chain2.size() - 1];
			if (is_vertex_on_triangle(chain1[chain1.size() - 1], p)) {
				if (chain2.size() > 1 && chain2[chain2.size() - 2] == diff) {
					chain2.pop_back(); 
				} else {
					if (chain2.size() > 0 && is_point_interesting(chain2[chain2.size() - 1]) && is_point_interesting(diff))
						chain2.pop_back();
					chain2.push_back(diff); 
				}
			} else {
				if (chain1.size() > 1 && chain1[chain1.size() - 2] == diff) {
					chain1.pop_back();
				} else {
					if (chain1.size() > 0 && is_point_interesting(chain1[chain1.size() - 1]) && is_point_interesting(diff))
						chain1.pop_back();
					chain1.push_back(diff);
				}
			}
		}
		return;
	}
}

void print_vertex(double x, double y) {
	printf("%.18f %.18f", x , y);
}

void print_vertex_double(int v) {
	printf("%.18f %.18f\n", ((double) points[v].x) / 12.0, ((double)points[v].y) / 12.0);
}

void print_vertex_double(double x, double y) {
	printf("%.18f %.18f\n", x / 12.0, y / 12.0);
}

void print_answer(int k) {
	triangle t = small_triangles[k];
	int diff = t[0] + t[1] + t[2] - chain1[chain1.size() - 1] - chain2[chain2.size() - 1];
	if (chain1.size() > 1 && chain1[chain1.size() - 2] == diff)
		chain1.pop_back();
	if (chain2.size() > 1 && chain2[chain2.size() - 2] == diff)
		chain2.pop_back();
	printf("%d\n", chain1.size() + chain2.size() + 2);	
	print_vertex_double(diff);
	for (int i = 0; i < chain1.size(); i++) {
		print_vertex_double(chain1[chain1.size() - 1 - i]);
	}
	print_vertex_double(last_point_x, last_point_y);
	for (int i = 0; i < chain2.size(); i++) {
		print_vertex_double(chain2[i]);
	}
}

int main() {
	freopen("kingdom.in", "r", stdin);
	freopen("kingdom.out", "w", stdout);
	scanf("%d", &n);
	for (int i = 0; i < n; i++) {
		int x, y;
		scanf("%d%d", &x, &y);
		points.push_back(point(x * 12, y * 12));
	}
	for (int i = 0; i < n; i++) 
		cut_vertices[i] = false;
	for (int i = 0; i < n; i++) {
		update_can_cut(i);
	}
	for (int it = 0; it < n - 2; it++) {
		int rem = -1;
		for (int i= 0; i < n; i++) {
			if (!cut_vertices[i] && can_cut[i]) {
				rem = i;
			}
		}
		int pr = prev(rem);
		int ne = next(rem);
		cut_vertices[rem] = true;
		all_triangles.push_back(triangle(pr, rem, ne));
		update_can_cut(pr);
		update_can_cut(ne);
	}
	for (int i = 0; i < all_triangles.size(); i++) {
		triangle t = all_triangles[i];
		int m1 = get_betw(t[0], t[1]);
		int m2 = get_betw(t[1], t[2]);
		int m3 = get_betw(t[2], t[0]);
		int xx = (points[t[0]].x + points[t[1]].x + points[t[2]].x) / 3;
		int yy = (points[t[0]].y + points[t[1]].y + points[t[2]].y) / 3;
		int med = points.size();
		points.push_back(point(xx, yy));
		small_triangles.push_back(triangle(t[0], m1, med));
		small_triangles.push_back(triangle(t[1], m1, med));
		small_triangles.push_back(triangle(t[1], m2, med));
		small_triangles.push_back(triangle(t[2], m2, med));
		small_triangles.push_back(triangle(t[2], m3, med));
		small_triangles.push_back(triangle(t[0], m3, med));
	}
	for (int i = 0; i < small_triangles.size(); i++) {
		vector<int> tmp;
		g.push_back(tmp);
		triangle t = small_triangles[i];
		save_tringle_side_connection(t[0], t[1], i);
		save_tringle_side_connection(t[1], t[2], i);
		save_tringle_side_connection(t[2], t[0], i);
	}
	for (int i = 0; i < small_triangles.size(); i++) {
		triangle t = small_triangles[i];
		int getAn = get_another_triangle_on_same_side(t[0], t[1], i);
		if (getAn == -1) {
			int get_an2 = get_another_triangle_on_same_side(t[1], t[2], i);
			g[i].push_back(get_an2);
		} else {
			g[i].push_back(getAn);
		}
		int getAn3 = get_another_triangle_on_same_side(t[2], t[0], i);
		g[i].push_back(getAn3);
	}
	need_sq = 0;
	for (int i = 0; i < n; i++) { 
		point p1 = points[i];
		point p2 = points[(i + 1) % n];
		need_sq += (p1.x - p2.x) * 1LL * (p1.y + p2.y) / 4;
	}
	for (int i = 0; i < small_triangles.size(); i++) {
		used_trinalges.push_back(false);
	}
	cur_sq = 0;
	dfs(0, -1);
	print_answer(0);
	chain1.clear();
	chain2.clear();
	cur_sq = 0;
	dfs(g[0][1], -1);
	print_answer(g[0][1]);
}