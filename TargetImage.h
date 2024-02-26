#pragma once

#include <QObject>
#include <opencv2/core.hpp>
#include <vector>
#include <optional>

struct CalibrationParams {
    double edgeStrength;
    double gridStep;
    cv::Size gridSize;
    std::optional<cv::Rect> imageROI;
};

class Graphics;

class TargetImage : public QObject {
    Q_OBJECT
public:
    explicit TargetImage(QObject *parent = nullptr);
    void loadImage(QString filename);
    void startCalibration(const CalibrationParams& prams);
    auto empty() const {
        return mImage.empty();
    }
    const auto& getCameraMatrix() const {
        return mCameraMatrix;
    }
    const auto& getImage() const {
        return mImage;
    }
    const auto& getFilename() const {
        return mFilename;
    }
    QRectF getImageRect() const;
    QString cameraMatrixToString() const;
    void draw(const Graphics& graphics) const;
    std::vector<cv::Point2f> detectGridCenters(const cv::Size& gridSize, double edgeStrength,
                                               const std::optional<cv::Rect>& imageROI) const;
    std::vector<cv::Vec3f> detectGridCircles(const cv::Size& gridSize, double edgeStrength,
                                             const std::optional<cv::Rect>& imageROI) const;
signals:      
    void changed();
    void error(const QString& message);
private:
    QString mFilename;
    cv::Mat mImage;
    std::vector<cv::Point2f> mDetectedGridPoints;
    std::optional<cv::Matx33f> mCameraMatrix{};
};
