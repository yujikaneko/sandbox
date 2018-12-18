#include <iostream>
#include <opencv2/opencv.hpp>
#include "hog.h"

#ifdef DEBUG
#define LOG(str) std::cout << str << std::endl;
#define LOGN(str) std::cout << str;
#else
#define LOG(str)
#define LOGN(str)
#endif

const int cell_size = 5;
const int block_size = 5;
const int block_area = block_size * block_size;
const int bin_num = 4;

HoG::HoG(int cell, int block, int bin) : cell_size(cell), block_size(block), block_area(block * block), bin_num(bin) {
}

// 1セルの Histogram を算出
cv::Mat HoG::HistCell(const cv::Mat grad, const cv::Mat angle) {
    cv::Size siz(bin_num, 1);
    cv::Mat hist = cv::Mat::zeros(1, 1, CV_32FC(bin_num));
    for (int y = 0; y < cell_size; y++) {
        for (int x = 0; x < cell_size; x++) {
            float theta = angle.at<float>(y, x);
            unsigned int th_id =
                static_cast<unsigned int>(theta / 360.0 * bin_num * 4.0);
            th_id = ((th_id + 1) / 2) % bin_num;
            hist.data[th_id] += grad.at<float>(y, x);
        }
    }
    return hist;
}

// Histogram of Gradient 算出
cv::Mat HoG::Calc(const cv::Mat img_src) {
	LOG("HOG Start!");
    LOG(" Input size:" << img_src.size() << " ch=[" << img_src.channels() << "]");
    LOG(" cell_size:" << cell_size << ",block_size:" << block_size << ",bin_num:" << bin_num);
    LOG(" feature vector dim:" << block_area * bin_num);

    // グレースケールに変換
    cv::Mat img_mono, img_mono_f;
    cv::cvtColor(img_src, img_mono, CV_RGB2GRAY);
    img_mono.convertTo(img_mono_f, CV_32F);
    LOG(" converted to gray scale");

    // 微分画像作成
    const cv::Mat ker_x =
        (cv::Mat_<float>(3, 3) << 0, 0, 0, -0.5, 0, 0.5, 0, 0, 0);
    const cv::Mat ker_y =
        (cv::Mat_<float>(3, 3) << 0, -0.5, 0, 0, 0, 0, 0, 0.5, 0);
    cv::Mat gradx, grady;
    cv::filter2D(img_mono_f, gradx, -1, ker_x);
    cv::filter2D(img_mono_f, grady, -1, ker_y);
    LOG(" created deliverable image");

    // 極座標変換
    cv::Mat grad, angl;
    cv::cartToPolar(gradx, grady, grad, angl, true);
    LOG(" calculated gradient and angle");

    // セルごとのヒストグラム作成
    LOG(" start histogram calculation");
    int X = grad.cols;
    int Y = grad.rows;
    cv::Size cell_num(X / cell_size, Y / cell_size);
    cv::Rect cell_roi(0, 0, cell_size, cell_size);
    cv::Mat hist =
        cv::Mat::zeros(cell_num.height, cell_num.width, CV_32FC(bin_num));
    cv::Rect hist_roi(0, 0, 1, 1);
    for (int y = 0; y < Y - cell_size; y += cell_size) {
        cell_roi.y = y;
        if (y % 100 == 0) {
            LOGN(" " << y << "...");
        }
        for (int x = 0; x < X - cell_size; x += cell_size) {
            cell_roi.x = x;
            cv::Mat grad_roi = grad(cell_roi);
            cv::Mat angl_roi = angl(cell_roi);
            cv::Mat h = HistCell(grad_roi, angl_roi);
            h.copyTo(hist(hist_roi));
            hist_roi.x++;
        }
        hist_roi.x = 0;
        hist_roi.y++;
    }
    LOG(" " << Y - cell_size);
    LOG(" finished histogram calculation");

    // ブロックごとに規定化
    LOG(" start block calculation");
    cv::Size hog_num(cell_num.width - block_size + 1,
                     cell_num.height - block_size + 1);
    cv::Rect roi(0, 0, block_size, block_size);
    cv::Size hog_siz(block_area * hog_num.width, hog_num.height);
    cv::Mat hog = cv::Mat::zeros(hog_siz, CV_32FC(bin_num));
    cv::Rect hog_roi(0, 0, block_area, 1);
    for (int y = 0; y < hog_num.height; y++) {
        if (y % 100 == 0) {
            LOGN(" " << y << "...");
        }
        for (int x = 0; x < hog_num.width; x++) {
            cv::Mat r = hist(roi).clone();
            cv::Mat rmat = r.reshape(1, 1);
            double norm = 1.0 / (1.0 + cv::norm(rmat));
            cv::Mat rn = r.mul(norm);
            cv::Mat rnv = rn.reshape(bin_num, 1);
            rnv.copyTo(hog(hog_roi));
            roi.x++;
            hog_roi.x += block_area;
        }
        roi.x = 0;
        roi.y++;
        hog_roi.x = 0;
        hog_roi.y++;
    }
    LOG(" " << hog_num.height);
    LOG(" finished block calculation");

    // チャネル数、ビン数×ブロックサイズ^2、ブロック横×ブロック高さの Mat に変形
    cv::Mat ret = hog.reshape(bin_num * block_area, hog_num.height);
    LOG(" reshape finished");

    LOG("HOG finished, returns:" << ret.size() << ", ch=[" << ret.channels() << "]");
    return ret;
}
