#include "Calibration.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <algorithm>
#include <execution>

namespace camcalib {

template<typename T>
static std::vector<T> sortGrid(std::vector<T> points, const cv::Size& patternSize) {
    if(points.size() != patternSize.area()) {
        std::cerr << __FUNCTION__": Grid size mismatch: "
                  << patternSize
                  << " "
                  << points.size()
                  << std::endl;
        return {};
    }
    std::sort(points.begin(), points.end(), [](const auto& p1, const auto& p2){
        if constexpr (std::is_same_v<cv::Point2f, T>) {
            return p1.y < p2.y;
        } else { // Vec3f - Circle
            return p1[1] < p2[1];
        }
    });
    for(auto first = points.begin(); first != points.end();) {
        auto last = first + patternSize.width;
        std::sort(first, last, [](const auto& p1, const auto& p2){
            if constexpr (std::is_same_v<cv::Point2f, T>) {
                return p1.x < p2.x;
            } else { // Vec3f - Circle
                return p1[0] < p2[0];
            }
        });
        first = last;
    }
    return points;
}

static inline auto fitCircle(const std::vector<cv::Point2f>& points, const cv::Point2f& offset = {}) {  
    try {
        auto ellipse = cv::fitEllipse(points);
        auto [x, y] = ellipse.center + cv::Point2f(offset);
        auto r = (ellipse.size.width + ellipse.size.height) / 4.0f;
        return cv::Vec3f{x, y, r};
    } catch(std::runtime_error& err) {
        std::cerr << __FUNCTION__": " << err.what() << std::endl;
        return cv::Vec3f{};
    }
}

static inline auto makeCircleSearcher(cv::Mat edges) {
    return [=](const cv::Rect& roi) {
        std::vector<cv::Point2f> edgePoints;
        cv::findNonZero(edges(roi), edgePoints);
        return fitCircle(edgePoints, roi.tl());
    };
}

static inline auto makeCircleCenterSearcher(cv::Mat edges) {
    auto circleSearcher = makeCircleSearcher(std::move(edges));
    return [circleSearcher](const cv::Rect& roi) {
        auto circle = circleSearcher(roi);
        return cv::Point2f(circle[0], circle[1]);
    };
}

static inline auto getComponentRect(const cv::Mat& stats, int componentIndex) {
    return cv::Rect(
        stats.at<int32_t>(componentIndex, cv::CC_STAT_LEFT),
        stats.at<int32_t>(componentIndex, cv::CC_STAT_TOP),
        stats.at<int32_t>(componentIndex, cv::CC_STAT_WIDTH),
        stats.at<int32_t>(componentIndex, cv::CC_STAT_HEIGHT)
        );
}

static inline auto isValidComponent(const cv::Rect& componentRect, const std::optional<cv::Rect>& roi) {
    if(auto roundness = std::abs(1.0 - componentRect.size().aspectRatio()); roundness > 0.1) {
        return false;
    }
    if(roi.has_value() && (*roi & componentRect) != componentRect) {
        return false;
    }
    return true;
}

// P = \ p11 p12 p13 \
//     \ p21 p22 p23 \
// uv_image = P * xy_world
static std::optional<cv::Matx23f> makeProjectionMatrix(const std::vector<cv::Point2f> &centersImage,
                                                       const std::vector<cv::Point2f> &centersWorld) {
    if(centersImage.size() != centersWorld.size()) {
        return std::nullopt;
    }
    const auto rows = 2 * static_cast<int>(centersWorld.size());
    auto rhs = cv::Mat(centersImage).reshape(1, rows);
    auto lhs = cv::Mat(rows, 6, CV_32F);
    for(auto r = 0; r < centersWorld.size(); r++) {
        auto [x, y] = centersWorld[r];
        cv::Mat(cv::Matx16f(x, y, 1.0f, 0.0f, 0.0f ,0.0f), false)
            .copyTo(lhs.row(2 * r));
        cv::Mat(cv::Matx16f(0.0f, 0.0f, 0.0f, x, y, 1.0f), false)
            .copyTo(lhs.row(2 * r + 1));
    }
    if(cv::Matx61f proj; cv::solve(lhs, rhs, proj, cv::DECOMP_NORMAL)) {
        return cv::Matx23f(proj(0), proj(1), proj(2),
                           proj(3), proj(4), proj(5));
    } else {        
        std::cerr << __FUNCTION__": can't find least squares projection matrix" << std::endl;
    }
    return std::nullopt;
}

static inline auto toMatx33f(const cv::Matx23f& mat) {
    return cv::Matx33f(mat(0, 0), mat(0, 1), mat(0, 2),
                       mat(1, 0), mat(1, 1), mat(1, 2),
                       0.0f, 0.0f, 1.0f);
}

static auto factorizeCameraMatrix(const cv::Matx33d& matrix) {
    /*
     *     \ mx alpha 0.0 \
     * F = \ 0.0 my   0.0 \
     *     \ 0.0 0.0  1.0 \
     *     \ r11 r12 t1 \
     * R = \ r21 r22 t2 \
     *     \ 0.0 0.0 1.0\
     * matrix = F * R
     * */
    auto my = std::hypot(matrix(1, 0), matrix(1, 1));
    auto t2 = matrix(1, 2) / my;
    auto r21 = matrix(1, 0) / my;
    auto r22 = matrix(1, 1) / my;
    auto alpha = matrix(0, 0) * r21 + matrix(0, 1) * r22;
    auto mx = std::hypot(matrix(0, 0) - alpha * r21, matrix(0, 1) - alpha * r22);
    auto t1 = (matrix(0, 2) - alpha * t2) / mx;
    auto r11 = (matrix(0, 0) - alpha * r21) / mx;
    auto r12 = (matrix(0, 1) - alpha * r22) / mx;
    auto r = cv::Matx33d(r11, r12, t1,
                         r21, r22, t2,
                         0.0f, 0.0f, 1.0f);
    auto f = cv::Matx33d(mx, alpha, 0.0f,
                         0.0f, my, 0.0f,
                         0.0f, 0.0f, 1.0f);
    return std::pair{f, r};
}

static cv::Matx23f makeNormalizationMatrix(const std::vector<cv::Point2f> &points) {
    assert(!points.empty());
    auto mean = cv::mean(points);
    auto center = cv::Point2d(mean(0), mean(1));
    auto distanceSum = std::accumulate(points.begin(),
                                       points.end(),
                                       0.0,
                                       [center](double acc, const cv::Point2d& p){
                                           return acc + cv::norm<double>(p - center);
                                       });
    auto scale = static_cast<float>(std::sqrt(2.0) * points.size() / distanceSum);
    return cv::Matx23f(
        scale, 0.0f, -scale * static_cast<float>(center.x),
        0.0f, scale, -scale * static_cast<float>(center.y));
}

static std::vector<cv::Rect> findCandidateRectangles(const cv::Mat& edges, const GridSearchParams& params) {
    cv::Mat labels, stats, centroids;
    auto components = cv::connectedComponentsWithStats(edges, labels, stats, centroids);
    std::vector<cv::Rect> result;
    for(int i = 1; i < components; i++) {
        if(auto rect = getComponentRect(stats, i); isValidComponent(rect, params.imageROI)) {
            result.push_back(rect);
        }
    }
    if(params.gridSize.area() > result.size()) {
        std::cerr << __FUNCTION__" count of components too small" << std::endl;
        return {};
    }
    std::sort(result.begin(), result.end(), [](const auto& r1, const auto& r2){
        return r1.area() > r2.area();
    });
    result.erase(result.begin() + params.gridSize.area(), result.end());
    return result;
}

template<typename Callable>
static inline auto findGrid(const cv::Mat& image, const GridSearchParams& params, Callable fitFunction) {
    assert(!params.gridSize.empty());
    assert(image.type() == CV_8U);
    assert(params.edgeStrength >= 0.0);
    cv::Mat edges;
    cv::Canny(image, edges, 0, params.edgeStrength);
    auto rectangles = findCandidateRectangles(edges, params);
    auto fit = fitFunction(std::move(edges));
    std::vector<decltype(fit({}))> centers(rectangles.size());
    std::transform(std::execution::par,
                   rectangles.begin(),
                   rectangles.end(),
                   centers.begin(),
                   fit);
    return sortGrid(std::move(centers), params.gridSize);
}

std::vector<cv::Point2f> findCirclesCentersGrid(const cv::Mat& image, const GridSearchParams& params) {
    return findGrid(image, params, makeCircleCenterSearcher);
}

std::vector<cv::Vec3f> findCirclesGrid(const cv::Mat &image, const GridSearchParams &params) {
    return findGrid(image, params, makeCircleSearcher);
}

std::vector<cv::Point2f> generatePointsGrid(const cv::Size &patternSize,
                                            double patternStep) {
    if(patternSize.empty()) {
        return {};
    }
    auto grid = std::vector<cv::Point2f>(patternSize.area());
    std::generate(std::execution::seq,
                  grid.begin(), grid.end(),
                  [i{0}, patternSize, patternStep]() mutable {
                      auto row = i / patternSize.width;
                      auto col = i % patternSize.width;
                      ++i;
                      return cv::Point2d(col * static_cast<float>(patternStep),
                                         row * static_cast<float>(patternStep));
                  });
    return grid;
}

std::optional<cv::Matx33f> calibrate(std::vector<cv::Point2f> centersImage,
                                     std::vector<cv::Point2f> centersWorld) {
    if(centersImage.size() != centersWorld.size()) {
        return std::nullopt;
    }
    auto normMatrixWorld = makeNormalizationMatrix(centersWorld);
    cv::transform(centersWorld,
                  centersWorld,
                  normMatrixWorld);
    auto normMatrixImage = makeNormalizationMatrix(centersImage);
    cv::transform(centersImage,
                  centersImage,
                  normMatrixImage);
    auto projectionMatrix = makeProjectionMatrix(centersImage, centersWorld);
    if(projectionMatrix) {
        auto proj = toMatx33f(normMatrixImage).inv() * toMatx33f(*projectionMatrix) * toMatx33f(normMatrixWorld);
        auto [f, r] = factorizeCameraMatrix(proj);
        std::cout << f(0, 0) << " " << f(1, 1) << std::endl;
        return f;
    }
    return std::nullopt;
}

void drawGrid(const std::vector<cv::Point2f> &grid, cv::Mat dst, const cv::Scalar &color) {
    for(size_t i = 1; i < grid.size(); i++) {
        cv::arrowedLine(dst, grid[i - 1], grid[i], color);
    }
}


}
