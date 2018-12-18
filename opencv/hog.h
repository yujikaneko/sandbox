#pragma once
#include <opencv2/opencv.hpp>

class HoG {
public:
	HoG(int cell_size = 5, int block_size = 5, int bin_num = 4);

	// Histogram of Gradient 算出
	cv::Mat Calc(const cv::Mat img_src);

private:
	cv::Mat HistCell(const cv::Mat grad, const cv::Mat angle);
	int cell_size;
	int block_size;
	int block_area;
	int bin_num;
};
