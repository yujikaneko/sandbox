#include <iostream>
#include <opencv2/opencv.hpp>
#include "hog.h"

const int cell_size = 5;
const int block_size = 5;
const int bin_num = 4;

int main(int argc, char *argv[]) {
    if (argc < 2) return -1;
	std::string fname(argv[1]);

	cv::Mat img = cv::imread(fname, 1);
    if (img.empty()) return -1;

    HoG hog;
    cv::Mat dat = hog.Calc(img);

    cv::namedWindow("Image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
    cv::imshow("Image", img);
    cv::waitKey(0);
    return 0;
}
