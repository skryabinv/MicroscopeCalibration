#pragma once

#include <opencv2/core.hpp>

namespace camcalib {

class CalibrationCostFunction {
public:
    /*
     * params:
     * -- Transform image to world
     * q11, q12, q13, q21, q22, q23
     * -- Distortion center
     * x0, y0
     * -- Distortion parameters
     * k1, k2, p1, p2, s1, s2
     * */
    enum Indices {
        _q11 = 0,
        _q12 = 1,
        _q13 = 2,
        _q21 = 3,
        _q22 = 4,
        _q23 = 5,
        _x0 = 6,
        _y0 = 7,
        _k1 = 8,
        _k2 = 9,
        _p1 = 10,
        _p2 = 11,
        _s1 = 12,
        _s2 = 13
    };
    static constexpr auto ParamsCount = 14;
    CalibrationCostFunction(const cv::Point2d& worldPoint,
                            const cv::Point2d& imagePoint)
        : mWorldPoint{worldPoint}, mImagePoint{imagePoint} {
    }
    template<typename T>
    bool operator()(const T* const params, T* residual) const {
        auto du = mImagePoint.x - params[_x0];
        auto dv = mImagePoint.y - params[_y0];
        auto r = du * du + dv * dv;
        auto uSigma = params[_k1] * du * r +
                      params[_k2] * du * r * r +
                      params[_p1] * (3.0 * du * du + dv * dv) +
                      params[_p2] * 2.0 * du * dv +
                      params[_s1] * r;
        auto vSigma = params[_k1] * dv * r +
                      params[_k2] * dv * r * r +
                      params[_p1] * 2.0 * du * dv +
                      params[_p2] * (du * du + 3.0 * dv * dv) +
                      params[_s2] * r;
        auto u = mImagePoint.x - uSigma;
        auto v = mImagePoint.y - vSigma;
        auto x = params[_q11] * u + params[_q12] * v + params[_q13];
        auto y = params[_q21] * u + params[_q22] * v + params[_q23];
        residual[0] = x - mWorldPoint.x;
        residual[1] = y - mWorldPoint.y;
        return true;
    }

private:   
    cv::Point2d mWorldPoint;
    cv::Point2d mImagePoint;
};

}
