#pragma once

struct Point
{
	short x;
	short y;
};

enum DIR
{
	LEFT,
	RIGHT,
	UP,
	DOWN
};

enum EDGE
{
	HOR,
	VER
};

void Sobel(const unsigned char* in, int height, int width, unsigned short* out);

unsigned char* padImage(unsigned char* image, int width, int height, unsigned char paddingColor);

unsigned char* mooreNeighborTracing(unsigned char* image, int width, int height);

void getAnchors(const unsigned short* in, int width, int height, int grad_thresh,
                int anchor_interval, int anchor_thresh,
                std::vector<Point>& anchors, unsigned char* out);

void trace(const unsigned short* in, int width, int height, int grad_thresh, Point pt_last,
           Point pt_cur, DIR dir_last, bool push_back, std::list<Point>& edge, unsigned char* out);

void traceFromAnchor(const unsigned short* in, int width, int height, int proposal_thresh,
                     const Point& anchor,
                     unsigned char* out,
                     std::vector<std::list<Point>>& edges);

inline int dist(const Point& p1, const Point& p2)
{
	return abs(p1.x - p2.x) + abs(p1.y - p2.y);
}
