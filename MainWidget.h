#pragma once

#include <QTabWidget>

class CameraModel;
class WidgetPixelSizeCalibration;
class WidgetCameraModel;
class WidgetOpticalCenterSearch;

class MainWidget : public QWidget {
    Q_OBJECT
signals:    
public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
private:
    void notifyError(const QString& message);
    void setupWidgets();
    void setupCameraModel();    
private:    
    CameraModel* mCameraModel{};
    WidgetPixelSizeCalibration* mWidgetPixelSizeCalibration{};
    WidgetOpticalCenterSearch* mWidgetOpticalCenter{};
    WidgetCameraModel* mWidgetCameraModel{};
};
