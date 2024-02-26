#include "MainWidget.h"
#include "CameraModel.h"
#include "WidgetPixelSizeCalibration.h"
#include "WidgetOpticalCenterSearch.h"
#include "WidgetCameraModel.h"
#include <QMessageBox>
#include <QTabWidget>
#include <QBoxLayout>

void MainWidget::setupWidgets() {
    mWidgetPixelSizeCalibration = new WidgetPixelSizeCalibration(mCameraModel);
    connect(mWidgetPixelSizeCalibration, &WidgetPixelSizeCalibration::error,
            this, &MainWidget::notifyError);    
    mWidgetCameraModel = new WidgetCameraModel(mCameraModel);
    mWidgetOpticalCenter = new WidgetOpticalCenterSearch(mCameraModel);
    auto tab = new QTabWidget();
    tab->addTab(mWidgetPixelSizeCalibration, tr("Размер пиксела"));
    tab->addTab(mWidgetOpticalCenter, tr("Оптический центр"));
    tab->setCurrentIndex(0);
    auto layout = new QHBoxLayout();
    layout->addWidget(tab);
    layout->addWidget(mWidgetCameraModel);
    layout->setStretch(0, 1);
    setLayout(layout);
}

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent) {
    setWindowTitle(tr("Калибровка микроскопа"));
    setupCameraModel();    
    setupWidgets();    
}

MainWidget::~MainWidget() {

}

void MainWidget::setupCameraModel() {
    mCameraModel = new CameraModel(this);   
}

void MainWidget::notifyError(const QString &message) {
    QMessageBox::critical(this, tr("Ошибка"), message);
}
