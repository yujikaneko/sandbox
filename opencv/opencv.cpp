//============================================================================
// Name        : opencv.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : https://qiita.com/mikaji/items/3e3f85e93d894b4645f7
//============================================================================
#include <iostream>
#include <opencv2/opencv.hpp>

//cv::Mat affine(cv::Mat src, int x, int y) {
//    cv::Mat mat =
//        (cv::Mat_<float>(2, 3) << 1.0, 0.0, (float)x, 0.0, 1.0, (float)y);
//    cv::Mat dst;
//    cv::warpAffine(src, dst, mat, src.size());
//    return dst;
//}

const int cell_size = 5;
const int bin_num = 4;

cv::Mat HistCell(cv::Mat grad, cv::Mat angle, int x, int y) {
    int X = x + cell_size;
    int Y = y + cell_size;
    cv::Size siz(bin_num, 1);
    cv::Mat dst = cv::Mat::zeros(siz, CV_32F);
    for (; y < Y; y++) {
        for (; x < X; x++) {
            float theta = angle.at<float>(y, x);
            unsigned int th_id =
                static_cast<unsigned int>(theta / 360.0 * bin_num * 4.0);
            th_id = ((th_id + 1) / 2) % bin_num;
            dst.at<float>(0, th_id) += grad.at<float>(y, x);
        }
    }
    return dst;
}

cv::Mat HOG(cv::Mat grad, cv::Mat angle) {
    int X = grad.cols;
    int Y = grad.rows;
    cv::Size cell_num(bin_num * static_cast<int>(X / cell_size),
                      static_cast<int>(Y / cell_size));
    cv::Mat dst = cv::Mat::zeros(cell_num, CV_32F);

    cv::Rect roi(1, 1, cell_size, cell_size);
    cv::Rect dst_roi(0, 0, bin_num, 1);
    for (roi.y = 1; roi.y + cell_size < Y - 1; roi.y += cell_size) {
        for (roi.x = 1; roi.x + cell_size < X - 1; roi.x += cell_size) {
            cv::Mat h = HistCell(grad, angle, roi.x, roi.y);
            h.copyTo(dst(dst_roi));
            dst_roi.x += bin_num;
        }
        dst_roi.x = 0;
        dst_roi.y++;
    }
    return dst;
}

int main(void) {
    cv::Mat src_img = cv::imread("sample.jpg", 1);
    if (src_img.empty()) return -1;

    cv::Mat m_img, mono_img;
    cv::cvtColor(src_img, m_img, CV_RGB2GRAY);
    m_img.convertTo(mono_img, CV_32F);

    const cv::Mat ker_x =
        (cv::Mat_<float>(3, 3) << 0, 0, 0, -0.5, 0, 0.5, 0, 0, 0);
    const cv::Mat ker_y =
        (cv::Mat_<float>(3, 3) << 0, -0.5, 0, 0, 0, 0, 0, 0.5, 0);
    cv::Mat gradx, grady;
    cv::filter2D(mono_img, gradx, -1, ker_x);
    cv::filter2D(mono_img, grady, -1, ker_y);

    //    cv::Mat img_xp1 = affine(mono_img, 1, 0);
    //    cv::Mat img_xm1 = affine(mono_img, -1, 0);
    //    cv::Mat img_yp1 = affine(mono_img, 0, 1);
    //    cv::Mat img_ym1 = affine(mono_img, 0, -1);

    //    cv::Mat gradx = img_xp1 - img_xm1;
    //    cv::Mat grady = img_yp1 - img_ym1;
    cv::Mat mag, angl;
    cv::cartToPolar(gradx, grady, mag, angl, true);
    cv::Mat hog = HOG(mag, angl);
    // std::cout << "hog=" << hog << std::endl;

    cv::namedWindow("Image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
    cv::imshow("Image", angl);
    cv::waitKey(0);
    return 0;
}
