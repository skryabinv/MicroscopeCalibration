#pragma once

#include <opencv2/core.hpp>
#include <optional>

namespace camcalib {

cv::Mat accumulateImageFromFiles(const std::vector<std::string>& files);

std::vector<cv::Point2f> generatePointsGrid(const cv::Size& patternSize, double patternStep);

struct GridSearchParams {
    double edgeStrength{100.0};
    cv::Size gridSize;
    std::optional<cv::Rect> imageROI{std::nullopt};
};

std::vector<cv::Point2f> findCirclesCentersGrid(const cv::Mat& image, const GridSearchParams& params);
std::vector<cv::Vec3f> findCirclesGrid(const cv::Mat& image, const GridSearchParams& params);

void drawGrid(const std::vector<cv::Point2f>& grid, cv::Mat dst, const cv::Scalar& color);

std::optional<cv::Matx33f> calibrate(std::vector<cv::Point2f> centersImage,
                                     std::vector<cv::Point2f> centersWorld);

}
