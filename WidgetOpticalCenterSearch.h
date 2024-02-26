#pragma once

#include <QWidget>
#include <opencv2/core/types.hpp>

namespace Ui {
class WidgetOpticalCenterSearch;
}

class CameraModel;
class TargetImage;

class WidgetOpticalCenterSearch : public QWidget {
    Q_OBJECT
signals:
    void error(const QString& message);
public:
    explicit WidgetOpticalCenterSearch(CameraModel* cameraModel, QWidget *parent = nullptr);
    ~WidgetOpticalCenterSearch();
    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:    
    void setupWidgets();
    void setupTargetImage();
    void updateWidgets();
    void loadImage();
    void findTrackedCircle();
    void calculateOpticalCenter();
    void addOpticalCenterToModel();
    void onNewImage();    
    Ui::WidgetOpticalCenterSearch *ui;
    CameraModel* mCameraModel{};
    TargetImage* mTargetImage{};
    std::optional<cv::Point2d> mOpticalCenter{};
    std::vector<cv::Vec3f> mDetectedCircles{};
};
