#pragma once

#include <QWidget>

namespace Ui {
class WidgetCameraModel;
}

class CameraModel;

class WidgetCameraModel : public QWidget {
    Q_OBJECT
public:
    explicit WidgetCameraModel(CameraModel* cameraModel, QWidget *parent = nullptr);
    ~WidgetCameraModel();
private:
    void updateCameraModelUi();
    void setupWidgets();
    void loadCameraModelFromFile();
    void saveCameraModelToFile();
    Ui::WidgetCameraModel *ui;
    CameraModel* mCameraModel{};
};
