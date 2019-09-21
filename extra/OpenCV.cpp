#include <opencv2/opencv.hpp>
#include "OpenCV.h"
#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <fstream>


/**
 * \brief Computes gradient, with magnitude and orientation
 * \param in input picture
 * \param width
 * \param height
 * \param out output picture, lowest digit in each pixel shows orientation, higher digits show gradient magnitude
 */
void Sobel(const unsigned char* in, const int height, const int width, unsigned short* out)
{
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			if (i == 0 || i == height - 1 || j == 0 || j == width - 1)
			{
				*(out + i * width + j) = 0;
				continue;
			}

			int Gx = *(in + (i - 1) * width + j + 1)
				+ 2 * *(in + i * width + j + 1)
				+ *(in + (i + 1) * width + j + 1)
				- *(in + (i - 1) * width + j - 1)
				- 2 * *(in + i * width + j - 1)
				- *(in + (i + 1) * width + j - 1);
			int Gy = *(in + (i - 1) * width + j - 1)
				+ 2 * *(in + (i - 1) * width + j)
				+ *(in + (i - 1) * width + j + 1)
				- *(in + (i + 1) * width + j - 1)
				- 2 * *(in + (i + 1) * width + j)
				- *(in + (i + 1) * width + j + 1);
			/*if (abs(Gx) + abs(Gy) >= 128)
				*(out + i * width + j) = 254;
			else
				*(out + i * width + j) = (abs(Gx) + abs(Gy)) << 1;*/
			*(out + i * width + j) = (abs(Gx) + abs(Gy)) << 1;
			*(out + i * width + j) += abs(Gx) > abs(Gy) ? VER : HOR;
			//if (abs(Gx) + abs(Gy) >= 72)
			//	*(out + i * width + j) = 255;
		}
	}
}

unsigned char* padImage(unsigned char* image, int width, int height, unsigned char paddingColor)
{
	unsigned char* paddedImage = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * (height + 2) * (width + 2))
		);
	for (int x = 0; x < width + 2; x++)
	{
		for (int y = 0; y < height + 2; y++)
		{
			if (x == 0 || y == 0 || x == width + 1 || y == height + 1)
			{
				paddedImage[x + y * (width + 2)] = paddingColor;
			}
			else
			{
				paddedImage[x + y * (width + 2)] = image[x - 1 + (y - 1) * width];
			}
		}
	}
	return paddedImage;
}

unsigned char* mooreNeighborTracing(unsigned char* image, int width, int height)
{
	bool inside = false;
	int pos = 0;
	// std::ofstream out("out.txt");

	unsigned char* paddedImage = padImage(image, width, height, 255);

	unsigned char* borderImage = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * (height + 2) * (width + 2))
		);

	for (int y = 0; y < (height + 2); y++)
	{
		for (int x = 0; x < (width + 2); x++)
		{
			borderImage[x + y * (width + 2)] = 255;
		}
	}

	for (int y = 0; y < (height + 2); y++)
	{
		for (int x = 0; x < (width + 2); x++)
		{
			pos = x + y * (width + 2);

			// Scan for BLACK pixel
			if (borderImage[pos] == 0 && !inside) // Entering an already discovered border
			{
				inside = true;
			}
			else if (paddedImage[pos] == 0 && inside) // Already discovered border point
			{
			}
			else if (paddedImage[pos] == 255 && inside) // Leaving a border
			{
				inside = false;
			}
			else if (paddedImage[pos] == 0 && !inside) // Undiscovered border point
			{
				borderImage[pos] = 0; // Mark the start pixel
				int checkLocationNr = 1; // The neighbor number of the location we want to check for a new border point
				int checkPosition; // The corresponding absolute array address of checkLocationNr
				int newCheckLocationNr;
				// Variable that holds the neighborhood position we want to check if we find a new border at checkLocationNr
				int startPos = pos; // Set start position
				int counter = 0; // Counter is used for the jacobi stop criterion
				int counter2 = 0; // Counter2 is used to determine if the point we have discovered is one single point

				// Defines the neighborhood offset position from current position and the neighborhood
				// position we want to check next if we find a new border at checkLocationNr
				int neighborhood[8][2] = {
					{-1, 7},
					{-3 - width, 7},
					{-width - 2, 1},
					{-1 - width, 1},
					{1, 3},
					{3 + width, 3},
					{width + 2, 5},
					{1 + width, 5}
				};
				// Trace around the neighborhood
				while (true)
				{
					checkPosition = pos + neighborhood[checkLocationNr - 1][0];
					newCheckLocationNr = neighborhood[checkLocationNr - 1][1];

					if (paddedImage[checkPosition] == 0) // Next border point found
					{
						if (checkPosition == startPos)
						{
							counter++;

							// Stopping criterion (jacob)
							if (newCheckLocationNr == 1 || counter >= 3)
							{
								// Close loop
								inside = true;
								// Since we are starting the search at were we first started we must set inside to true
								break;
							}
						}

						checkLocationNr = newCheckLocationNr; // Update which neighborhood position we should check next
						pos = checkPosition;
						counter2 = 0; // Reset the counter that keeps track of how many neighbors we have visited
						borderImage[checkPosition] = 0; // Set the border pixel
						/*out << checkPosition / (width + 2) << ' ' << checkPosition % (width + 2) << std::endl;*/
					}
					else
					{
						// Rotate clockwise in the neighborhood
						checkLocationNr = 1 + (checkLocationNr % 8);
						if (counter2 > 8)
						{
							// If counter2 is above 8 we have traced around the neighborhood and
							// therefor the border is a single black pixel and we can exit
							counter2 = 0;
							break;
						}
						counter2++;
					}
				}
			}
		}
	}

	// Remove white padding and return it
	unsigned char* clippedBorderImage = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * (height) * (width)));
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			clippedBorderImage[x + y * width] = borderImage[x + 1 + (y + 1) * (width + 2)];
		}
	}
	free(paddedImage);
	free(borderImage);
	return clippedBorderImage;
}

void getAnchors(const unsigned short* in, const int width, const int height, const int grad_thresh,
	const int anchor_interval, const int anchor_thresh,
	std::vector<Point>& anchors, unsigned char* out)
{
	anchors.clear();
	anchors.reserve(height * width);

	for (int i = 1; i < height - 1; i += anchor_interval)
	{
		for (int j = 1; j < width - 1; j += anchor_interval)
		{
			const auto grad = *(in + i * width + j) >> 1;
			if (grad < grad_thresh)
				continue;

			if (*(in + i * width + j) % 2 == VER)
			{
				// vertical
				if (grad - (*(in + (i - 1) * width + j) >> 1) >= anchor_thresh &&
					grad - (*(in + (i + 1) * width + j) >> 1) >= anchor_thresh)
				{
					anchors.emplace_back(Point{ static_cast<short>(i), static_cast<short>(j) });
					*(out + i * width + j) = 255;
				}
			}
			else
			{
				// horizontal
				if (grad - (*(in + i * width + j - 1) >> 1) >= anchor_thresh &&
					grad - (*(in + i * width + j + 1) >> 1) >= anchor_thresh)
				{
					anchors.emplace_back(Point{ static_cast<short>(i), static_cast<short>(j) });
					*(out + i * width + j) = 255;
				}
			}
		}
	}
}


void trace(const unsigned short* in, const int width, const int height, const int grad_thresh, Point pt_last,
	Point pt_cur, DIR dir_last, bool push_back, std::list<Point>& edge, unsigned char* out)
{
	DIR dir_cur;
	while (true)
	{
		if (*(out + pt_cur.x * width + pt_cur.y))
			break;

		if (*(in + pt_cur.x * width + pt_cur.y) >> 1 < grad_thresh)
		{
			*(out + pt_cur.x * width + pt_cur.y) = 1;
			break;
		}

		*(out + pt_cur.x * width + pt_cur.y) = 255;
		if (push_back)
			edge.push_back(pt_cur);
		else
			edge.push_front(pt_cur);

		if (*(in + pt_cur.x * width + pt_cur.y) % 2 == HOR)
		{
			if (dir_last == UP || dir_last == DOWN)
			{
				if (pt_cur.y < pt_last.y)
					dir_cur = LEFT;
				else
					dir_cur = RIGHT;
			}
			else
				dir_cur = dir_last;

			pt_last = pt_cur;
			dir_last = dir_cur;

			if (dir_cur == LEFT)
			{
				auto leftTop = *(in + (pt_cur.x + 1) * width + pt_cur.y - 1);
				auto left = *(in + pt_cur.x * width + pt_cur.y - 1);
				auto leftBottom = *(in + (pt_cur.x - 1) * width + pt_cur.y - 1);

				if (leftTop >= left && leftTop >= leftBottom)
					pt_cur = { pt_cur.x + 1, pt_cur.y - 1 };
				else if (leftBottom >= left && leftBottom >= leftTop)
					pt_cur = { pt_cur.x - 1, pt_cur.y - 1 };
				else
					pt_cur.y -= 1;

				if (pt_cur.y == 0 || pt_cur.x == 0 || pt_cur.x == height - 1)
					break;
			}

			// go right
			else
			{
				auto rightTop = *(in + (pt_cur.x + 1) * width + pt_cur.y + 1);
				auto right = *(in + pt_cur.x * width + pt_cur.y + 1);
				auto rightBottom = *(in + (pt_cur.x - 1) * width + pt_cur.y + 1);

				if (rightTop >= right && rightTop >= rightBottom)
					pt_cur = { pt_cur.x + 1, pt_cur.y + 1 };
				else if (rightBottom >= right && rightBottom >= rightTop)
					pt_cur = { pt_cur.x - 1, pt_cur.y + 1 };
				else
					pt_cur.y += 1;

				if (pt_cur.y == width - 1 || pt_cur.x == 0 || pt_cur.x == height - 1)
					break;
			}
		}

		else
		{
			if (dir_last == LEFT || dir_last == RIGHT)
			{
				if (pt_cur.x < pt_last.x)
					dir_cur = DOWN;
				else
					dir_cur = UP;
			}
			else
				dir_cur = dir_last;

			pt_last = pt_cur;
			dir_last = dir_cur;

			if (dir_cur == UP)
			{
				auto leftTop = *(in + (pt_cur.x + 1) * width + pt_cur.y - 1);
				auto top = *(in + (pt_cur.x + 1) * width + pt_cur.y);
				auto rightTop = *(in + (pt_cur.x + 1) * width + pt_cur.y + 1);

				if (leftTop >= top && leftTop >= rightTop)
					pt_cur = { pt_cur.x + 1, pt_cur.y - 1 };
				else if (rightTop >= top && rightTop >= leftTop)
					pt_cur = { pt_cur.x + 1, pt_cur.y + 1 };
				else
					pt_cur.x += 1;

				if (pt_cur.x == height - 1 || pt_cur.y == 0 || pt_cur.y == width - 1)
					break;
			}

			else
			{
				auto leftBottom = *(in + (pt_cur.x - 1) * width + pt_cur.y - 1);
				auto bottom = *(in + (pt_cur.x - 1) * width + pt_cur.y);
				auto rightBottom = *(in + (pt_cur.x - 1) * width + pt_cur.y + 1);

				if (leftBottom >= bottom && leftBottom >= rightBottom)
					pt_cur = { pt_cur.x - 1, pt_cur.y - 1 };
				else if (rightBottom >= bottom && rightBottom >= leftBottom)
					pt_cur = { pt_cur.x - 1, pt_cur.y + 1 };
				else
					pt_cur.x -= 1;

				if (pt_cur.x == 0 || pt_cur.y == 0 || pt_cur.y == width - 1)
					break;
			}
		}
	}
}

void traceFromAnchor(const unsigned short* in, const int width, const int height, const int proposal_thresh,
	const Point& anchor,
	unsigned char* out,
	std::vector<std::list<Point>>& edges)
{
	if (*(out + anchor.x * width + anchor.y))
		return;

	std::list<Point> edge;
	Point pt_last;
	DIR dir_last;

	if (*(in + anchor.x * width + anchor.y) % 2 == HOR)
	{
		pt_last = { anchor.x, anchor.y + 1 };
		dir_last = LEFT;
		trace(in, width, height, proposal_thresh, pt_last, anchor, dir_last, false, edge, out);

		*(out + anchor.x * width + anchor.y) = 0;

		pt_last = { anchor.x, anchor.y - 1 };
		dir_last = RIGHT;
		trace(in, width, height, proposal_thresh, pt_last, anchor, dir_last, true, edge, out);
	}
	else
	{
		pt_last = { anchor.x - 1, anchor.y };
		dir_last = UP;
		trace(in, width, height, proposal_thresh, pt_last, anchor, dir_last, false, edge, out);

		*(out + anchor.x * width + anchor.y) = 0;

		pt_last = { anchor.x + 1, anchor.y };
		dir_last = DOWN;
		trace(in, width, height, proposal_thresh, pt_last, anchor, dir_last, true, edge, out);
	}
	if (edge.size() > 20)
		edges.push_back(edge);
}

void writeData(cv::Mat& picture, std::ofstream& outfile, bool flag)
{
	GaussianBlur(picture, picture, cv::Size(3, 3), 0, 0);
	const int height = picture.rows, width = picture.cols;

	unsigned char* in = picture.data;
	auto* out = new unsigned short[height * width];
	auto* out2 = new unsigned char[height * width];
	auto* out3 = new unsigned char[height * width];
	memset(out, 0, sizeof(unsigned short) * height * width);
	memset(out2, 0, sizeof(unsigned char) * height * width);
	memset(out3, 0, sizeof(unsigned char) * height * width);

	Sobel(in, height, width, out);

	std::vector<Point> anchors;
	const int thresh = flag ? 20 : 30;
	const int anchor_interval = 2;
	const int anchor_thresh = flag ? 8 : 15;
	getAnchors(out, width, height, thresh, anchor_interval, anchor_thresh, anchors, out2);
	std::vector<std::list<Point>> edges;
	for (const auto& anchor : anchors)
		traceFromAnchor(out, width, height, thresh, anchor, out3, edges);
	sort(edges.begin(), edges.end(), [](const std::list<Point>& a, const std::list<Point>& b) -> bool
		{
			return a.size() > b.size();
		});
	std::list<Point> all;
	for (auto& list : edges)
	{
		/*if (flag)
		{
			bool cycle = dist(list.front(), list.back()) < 5;
			int min = 999999;
			auto min_it = all.end();
			auto min_it2 = list.end();
			int cnt1 = 0, cnt2 = 0;
			for (auto it = all.begin(); it != all.end(); ++it, ++cnt1)
			{
				for (auto it2 = list.begin(); it2 != list.end(); ++it2, ++cnt2)
				{
					int d = dist(*it2, *it);
					if (d < min)
					{
						min = d;
						min_it = it;
						min_it2 = it2;
						if (d == 1)
							goto label;
					}
				}
			}
		label:
			if (!cycle && (cnt1 < all.size() / 2 && cnt2 < list.size() / 2 || cnt2 > list.size() / 2 && cnt1 > all.size() /
				2))
			{
				list.reverse();
				all.splice(min_it, list);
			}
			else
			{
				all.splice(min_it, list, min_it2, list.end());
				all.splice(min_it, list, list.begin(), list.end());
			}
		}
		else
		{*/
			all.splice(all.end(), list);
		//}
	}

	int cnt = 6;

	short n = 0xffff;
	outfile.write((const char*)& n, sizeof(n));
	n = (all.size() + 4) / 5 * 2;
	outfile.write((const char*)& n, sizeof(n));
	int a = 0;
	for (auto& pt : all)
	{
		if ((a++) % 5)
			continue;
		pt.x = 512 - pt.x;
		pt.y = 512 - pt.y;
		pt.x <<= 3;
		pt.x |= 0x1000;
		pt.y <<= 3;
		pt.y |= 0x9000;
		outfile.write((const char*)& pt.x, sizeof(pt.x));
		outfile.write((const char*)& pt.y, sizeof(pt.y));
		cnt += 4;
	}

	n = 0xf00f;
	outfile.write((const char*)& n, sizeof(n));

	char pad = 0;
	for (int i = cnt % 512; i != 512; ++i)
	{
		outfile.write(&pad, sizeof(pad));
	}

	for (const auto& list : edges)
	{
		a = 0;
		for (const auto& pt : list)
		{

			if ((a++) % 5)
				continue;
			*(out3 + pt.x * width + pt.y) = 255;
		}
	}

	imshow("test4", cv::Mat(height, width, CV_8UC1, out3));
	cv::waitKey(30);

	delete[] out3;
	delete[] out2;
	delete[] out;
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cerr << "Invalid argument" << std::endl;
		return -1;
	}

	std::ofstream outfile(argv[2], std::ios_base::binary);
    auto targetFps=atoi(argv[3]);
	short cnt = 0;
	try
	{
		cv::VideoCapture cap(argv[1]);
		bool first = true;
		int fps = int(round(cap.get(cv::CAP_PROP_FPS)));
		double timeFps = 1.0*targetFps/fps;

		double now = 1.01-timeFps;
		bool skip;
		while (cap.isOpened())
		{
			skip = true;
			now += timeFps;
			if(now>=1)
			{
				now=now-1;
				skip = false;
			}
			cv::Mat frame, pic;
			cap.read(frame);
			if (frame.empty())
			{
				break;
			}
			if (skip)
				continue;
			cvtColor(frame, pic, cv::COLOR_BGR2GRAY);
			int height = round(512 * pic.rows / pic.cols);
			cv::resize(pic, pic, cv::Size(512, height));
			if (first)
			{
				first = false;

				short n = 0xffff;
				outfile.write((const char*)& n, sizeof(n));
				n = 0;
				outfile.write((const char*)& n, sizeof(n));
				n = targetFps;
				outfile.write((const char*)& n, sizeof(n));
				n = 0xf00f;
				outfile.write((const char*)& n, sizeof(n));
				n = 0;
				for (int i = 0; i < 252; ++i)
				{
					outfile.write((const char*)& n, sizeof(n));
				}
				
				cv::Mat begin_frame(cv::Size(512, height), CV_8UC1);
				cv::putText(begin_frame, argv[1], cv::Point(0, height / 2), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255));
				for (int i = 0; i < 20; ++i)
				{
					writeData(begin_frame, outfile, true);
					++cnt;
				}
			}
			writeData(pic, outfile, false);
			++cnt;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	outfile.seekp(2, std::ios_base::beg);
	outfile.write((const char*)& cnt, sizeof(cnt));

	//Point cur = {-1, 0};
	//for (const auto& pt : all)
	//{
	//	if (cur.x != -1 && cur.x < 500 && abs(pt.x - cur.x) + abs(pt.y - cur.y) > 2)
	//	{
	//		int x = cur.x, y = cur.y;
	//		while (pt.x != x)
	//		{
	//			if (pt.x > x)
	//				++x;
	//			else
	//				--x;
	//			*(out3 + x * width + y) = 255;
	//		}
	//		while (pt.y != y)
	//		{
	//			if (pt.y > y)
	//				++y;
	//			else
	//				--y;
	//			*(out3 + x * width + y) = 255;
	//		}
	//	}
	//	*(out3 + pt.x * width + pt.y) = 255;
	//	cur = pt;
	//}
	//mooreNeighborTracing(out3, width, height);
	//std::ifstream input("out.txt", std::ios_base::in);
	//int i, j;
	//while (input >> i >> j)
	//{
	//	if (i < 1 || j < 1 || i > 512 || j > 512)
	//		continue;
	//	out2[(i - 1) * width + j - 1] = 255;
	//}
	/*cv::Mat a(512, 512, CV_8UC1, out3);
	int c = 0;
	for (const auto& list : edges)
	{
		putText(a, std::to_string(++c), cv::Point(list.front().y, list.front().x), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
		putText(a, std::to_string(++c), cv::Point(list.back().y, list.back().x), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
		for (const auto& pt : list)
		{
			*(out3 + pt.x * width + pt.y) = 255;
		}
	}*/

	//imshow("test", picture);
	////imshow("test2", cv::Mat(512, 512, CV_8UC1, out));
	//imshow("test3", cv::Mat(512, 512, CV_8UC1, out2));
	////imshow("test4", a);
	//imshow("test4", cv::Mat(512, 512, CV_8UC1, out3));
	//cv::waitKey(0);
	return 0;
}
