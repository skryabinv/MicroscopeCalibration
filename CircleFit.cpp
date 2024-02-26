#include "CircleFit.h"
#include <opencv2/imgproc.hpp>
#include <ceres/ceres.h>

namespace {

class CircleFitCostFunction {
public:
    CircleFitCostFunction(const cv::Point2f& sample) : mSample{sample} {}
    template<typename T>
    bool operator()(const T* const params, T* residual) const {
        // params: [0] - x0, [1] - y0, [2] - radius
        auto r2 = params[2] * params[2];
        auto dx = (mSample.x - params[0]);
        auto dy = (mSample.y - params[1]);
        residual[0] = r2 -  dx * dx - dy * dy;
        return true;
    }
private:
    cv::Point2d mSample;
};

auto findInitialGuess(const std::vector<cv::Point2f> &samples) {
    auto ellipse = cv::fitEllipse(samples);
    auto [x, y] = ellipse.center;
    auto r = (ellipse.size.width + ellipse.size.height) / 4.0f;
    return cv::Vec3d{x, y, r};
}

}

namespace camcalib {

std::optional<cv::Vec3f> fitCircleCeres(const std::vector<cv::Point2f> &samples) {
    auto initial = findInitialGuess(samples);
    auto circle = initial;
    ceres::Problem problem;
    for(const auto& pos: samples) {
        auto cost = new ceres::AutoDiffCostFunction<CircleFitCostFunction, 1, 3>(new CircleFitCostFunction(pos));
        problem.AddResidualBlock(cost, nullptr, circle.val);
    }
    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.minimizer_progress_to_stdout = false;
    options.max_linear_solver_iterations = 10;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    if(summary.IsSolutionUsable()) {
        return circle;
    }
    return std::nullopt;
}


}
