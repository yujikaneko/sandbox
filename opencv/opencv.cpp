#include <iostream>
#include <opencv2/opencv.hpp>

const int cell_size = 5;
const int block_size = 5;
const int block_area = block_size * block_size;
const int bin_num = 4;

// 1セルの Histogram を算出
cv::Mat HistCell(cv::Mat grad, cv::Mat angle) {
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
cv::Mat HOG(cv::Mat img_src) {
    std::cout << "HOG Start!" << std::endl;
    std::cout << " Input size:" << img_src.size() << " ch=["
              << img_src.channels() << "]" << std::endl;
    std::cout << " cell_size:" << cell_size << ",block_size:" << block_size
              << ",bin_num:" << bin_num << std::endl;
    std::cout << " feature vector dim:" << block_area * bin_num << std::endl;

    // グレースケールに変換
    cv::Mat img_mono, img_mono_f;
    cv::cvtColor(img_src, img_mono, CV_RGB2GRAY);
    img_mono.convertTo(img_mono_f, CV_32F);
    std::cout << " converted to gray scale" << std::endl;

    // 微分画像作成
    const cv::Mat ker_x =
        (cv::Mat_<float>(3, 3) << 0, 0, 0, -0.5, 0, 0.5, 0, 0, 0);
    const cv::Mat ker_y =
        (cv::Mat_<float>(3, 3) << 0, -0.5, 0, 0, 0, 0, 0, 0.5, 0);
    cv::Mat gradx, grady;
    cv::filter2D(img_mono_f, gradx, -1, ker_x);
    cv::filter2D(img_mono_f, grady, -1, ker_y);
    std::cout << " created deliverable image" << std::endl;

    // 極座標変換
    cv::Mat grad, angl;
    cv::cartToPolar(gradx, grady, grad, angl, true);
    std::cout << " calculated gradient and angle" << std::endl;

    // セルごとのヒストグラム作成
    std::cout << " start histogram calculation" << std::endl;
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
            std::cout << " " << y << "...";
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
    std::cout << " " << Y - cell_size << std::endl
              << " finished histogram calculation" << std::endl;

    // ブロックごとに規定化
    std::cout << " start block calculation" << std::endl;
    cv::Size hog_num(cell_num.width - block_size + 1,
                     cell_num.height - block_size + 1);
    cv::Rect roi(0, 0, block_size, block_size);
    cv::Size hog_siz(block_area * hog_num.width, hog_num.height);
    cv::Mat hog = cv::Mat::zeros(hog_siz, CV_32FC(bin_num));
    cv::Rect hog_roi(0, 0, block_area, 1);
    for (int y = 0; y < hog_num.height; y++) {
        if (y % 100 == 0) {
            std::cout << " " << y << "...";
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
    std::cout << " " << hog_num.height << std::endl
              << " finished block calculation" << std::endl;

    // チャネル数、ビン数×ブロックサイズ^2、ブロック横×ブロック高さの Mat に変形
    cv::Mat ret = hog.reshape(bin_num * block_area, hog_num.height);
    std::cout << " reshape finished" << std::endl;

    std::cout << "HOG finished, returns:" << ret.size() << ", ch=["
              << ret.channels() << "]" << std::endl;
    return ret;
}

int main(void) {
    cv::Mat img = cv::imread("sample.jpg", 1);
    if (img.empty()) return -1;

    cv::Mat hog = HOG(img);

    cv::namedWindow("Image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
    cv::imshow("Image", img);
    cv::waitKey(0);
    return 0;
}
