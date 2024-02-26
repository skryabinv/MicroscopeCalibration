#pragma once

#include <opencv2/core.hpp>

class OpticalCenterCostFunction {
public:
    OpticalCenterCostFunction(const cv::Point2d& featurePos,
                              size_t magnificationIndex,
                              const cv::Point2d& refPos)
        : mMagnificationIndex{magnificationIndex},
        mFeaturePos{featurePos},
        mRefPos{refPos} {
    }
    template<typename T>
    bool operator()(const T* const params, T* residual) const {
        auto x0 = params[0];
        auto y0 = params[1];
        auto sx = params[2];
        auto sy = params[3];

        residual[0] = sx * mRefPos.x + (1.0 - sx) * x0 - mFeaturePos.x;
        residual[1] = sy * mRefPos.y + (1.0 - sy) * y0 - mFeaturePos.y;
        return true;
    }
private:
    // Индекс увеличения
    size_t mMagnificationIndex;
    // Позиция точки с текущим увеличением
    cv::Point2d mFeaturePos;
    // Позиция точки с базовым увеличением (от которого счистаем коэффициент)
    cv::Point2d mRefPos;
};

inline auto getCircleSampler(const cv::Vec3f& circle, size_t samples) {
    auto delta = CV_2PI / samples;
    return [=](size_t index) {
        return cv::Point2d{
            circle[0] + std::cos(index * delta) * circle[2],
            circle[1] + std::sin(index * delta) * circle[2],
        };
    };
}

inline auto makeOpticalCenterCostFunctions(const cv::Vec3f& circle,
                                           const cv::Vec3f& refCircle,
                                           size_t magnificationIndex,
                                           size_t samples = 10) {
    std::vector<OpticalCenterCostFunction*> result;
    auto sampler = getCircleSampler(circle, samples);
    auto refSampler = getCircleSampler(refCircle, samples);
    for(size_t i = 0; i < samples; i++) {
        result.push_back(new OpticalCenterCostFunction(sampler(i),
                                                       magnificationIndex,
                                                       refSampler(i)));
    }
    return result;
}
