#pragma once

#include <QWidget>

namespace Ui {
class WidgetPixelSizeCalibration;
}

class CameraModel;
class TargetImage;
struct CalibrationParams;

class WidgetPixelSizeCalibration : public QWidget {
    Q_OBJECT
signals:
    void error(const QString& message);
public:
    explicit WidgetPixelSizeCalibration(CameraModel* cameraModel, QWidget *parent = nullptr);
    ~WidgetPixelSizeCalibration();
private:
    CalibrationParams collectCalibrationParams() const;
    void setupTargetImage();
    void setupWidgets();
    void updateWidgets();
    void loadImageFromFile();
    void startCalibration();
    void addCalibrationToModel();
private:    
    Ui::WidgetPixelSizeCalibration *ui;
    CameraModel* mCameraModel{};
    TargetImage* mTargetImage{};

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
