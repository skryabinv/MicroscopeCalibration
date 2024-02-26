#pragma once

#include <opencv2/core.hpp>
#include <optional>

namespace camcalib {

std::optional<cv::Vec3f> fitCircleCeres(const std::vector<cv::Point2f>& samples);

}
