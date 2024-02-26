#include "TargetImage.h"
#include "Calibration.h"
#include "Graphics.h"
#include <opencv2/imgcodecs.hpp>
#include <QRectF>
#include <QDebug>

TargetImage::TargetImage(QObject *parent)
    : QObject{parent}
{}

void TargetImage::loadImage(QString filename) {
    cv::Mat image = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    if(!image.empty()) {
        std::swap(mImage, image);
        mDetectedGridPoints.clear();
        mCameraMatrix.reset();
        mFilename = filename;
        emit changed();
    } else {
        emit error(QString("Не удалось открыть файл %1\n"
                           "Файл поврежден или формат изображения не поддерживается.").arg(filename));
    }
}

void TargetImage::startCalibration(const CalibrationParams &params) {
    auto detectedGrid = detectGridCenters(params.gridSize,
                                   params.edgeStrength,
                                   params.imageROI);
    if(detectedGrid.empty()) {
        emit error(tr("Ошибка поиска калибровочного шаблона"));
        return;
    }
    auto generatedGrid = camcalib::generatePointsGrid(params.gridSize, params.gridStep);
    auto cameraMatrix = camcalib::calibrate(detectedGrid, generatedGrid);
    if(cameraMatrix) {
        std::swap(detectedGrid, mDetectedGridPoints);
        std::swap(cameraMatrix, mCameraMatrix);
        emit changed();
    } else {
        emit error(tr("Ошибка вычисления матрицы камеры"));
    }
}

QRectF TargetImage::getImageRect() const {
    return QRectF(0.0f,  0.0f, mImage.cols, mImage.rows);
}

QString TargetImage::cameraMatrixToString() const {
    if(mCameraMatrix) {
        std::ostringstream ss;
        ss << *mCameraMatrix;
        return QString::fromStdString(ss.str());
    }
    return {};
}

void TargetImage::draw(const Graphics &graphics) const {
    graphics.drawImage(mImage);
    graphics.drawGridPoints(mDetectedGridPoints);
}

std::vector<cv::Point2f> TargetImage::detectGridCenters(const cv::Size &gridSize, double edgeStrength,
                                                 const std::optional<cv::Rect> &imageROI) const {
    auto searchParams = camcalib::GridSearchParams{edgeStrength, gridSize, imageROI};
    return camcalib::findCirclesCentersGrid(mImage, searchParams);
}

std::vector<cv::Vec3f> TargetImage::detectGridCircles(const cv::Size &gridSize, double edgeStrength, const std::optional<cv::Rect> &imageROI) const {
    auto searchParams = camcalib::GridSearchParams{edgeStrength, gridSize, imageROI};
    return camcalib::findCirclesGrid(mImage, searchParams);
}
