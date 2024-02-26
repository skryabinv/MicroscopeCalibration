#include "WidgetCameraModel.h"
#include "ui_WidgetCameraModel.h"
#include "CameraModel.h"
#include <QFileDialog>
#include <QMessageBox>

WidgetCameraModel::WidgetCameraModel(CameraModel *cameraModel, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::WidgetCameraModel),
    mCameraModel{cameraModel} {
    assert(mCameraModel != nullptr);
    ui->setupUi(this);
    setupWidgets();
}

WidgetCameraModel::~WidgetCameraModel() {
    delete ui;
}

void WidgetCameraModel::updateCameraModelUi() {
    mCameraModel->updateUi(ui->treeWidgetCameraModel);
}

void WidgetCameraModel::setupWidgets() {
    connect(ui->pushButtonClearModel, &QPushButton::clicked,
            mCameraModel, &CameraModel::clear);
    connect(ui->pushButtonSaveModel, &QPushButton::clicked,
            this, &WidgetCameraModel::saveCameraModelToFile);
    connect(ui->pushButtonLoadModel, &QPushButton::clicked,
            this, &WidgetCameraModel::loadCameraModelFromFile);
    connect(mCameraModel, &CameraModel::changed,
            this, &WidgetCameraModel::updateCameraModelUi);
}

void WidgetCameraModel::loadCameraModelFromFile() {
    static auto dir = QString{};
    auto filename = QFileDialog::getOpenFileName(this, tr("Открыть файл камеры"), dir, tr("Файл камеры (*.json)"));
    dir = QFileInfo(filename).dir().path();
    mCameraModel->loadFromFile(filename);
}

void WidgetCameraModel::saveCameraModelToFile() {
    static auto dir = QString{};
    auto filename = QFileDialog::getSaveFileName(this, tr("Сохранить файл камеры"), dir, tr("Файл камеры (*.json)"));
    dir = QFileInfo(filename).dir().path();
    if(!filename.isEmpty()) {
        mCameraModel->saveToFile(filename);        
    }
}
