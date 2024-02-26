#include "WidgetPixelSizeCalibration.h"
#include "ui_WidgetPixelSizeCalibration.h"
#include "CameraModel.h"
#include "TargetImage.h"
#include "Graphics.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPaintEvent>

WidgetPixelSizeCalibration::WidgetPixelSizeCalibration(CameraModel *cameraModel, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::WidgetPixelSizeCalibration),
    mCameraModel{cameraModel} {
    assert(mCameraModel != nullptr);
    ui->setupUi(this);
    setupTargetImage();
    setupWidgets();
    updateWidgets();
}

WidgetPixelSizeCalibration::~WidgetPixelSizeCalibration() {
    delete ui;
}

void WidgetPixelSizeCalibration::setupTargetImage() {
    mTargetImage = new TargetImage{this};
    connect(mTargetImage, &TargetImage::error,
            this, &WidgetPixelSizeCalibration::error);
    connect(mTargetImage, &TargetImage::changed,
            this, &WidgetPixelSizeCalibration::updateWidgets);
}

void WidgetPixelSizeCalibration::setupWidgets() {
    connect(ui->pushButtonOpen, &QPushButton::clicked,
            this, &WidgetPixelSizeCalibration::loadImageFromFile);
    connect(ui->pushButtonCalc, &QPushButton::clicked,
            this, &WidgetPixelSizeCalibration::startCalibration);
    connect(ui->widgetEditorROI, &WidgetEditorROI::roiChanged, this,
            QOverload<>::of(&WidgetPixelSizeCalibration::update));
    connect(ui->pushButtonAddToModel, &QPushButton::clicked,
            this, &WidgetPixelSizeCalibration::addCalibrationToModel);
    connect(ui->lineEditCalibrationName, &QLineEdit::textChanged, this,
            [this](const QString& text){
                auto hasCameraMatrix = mTargetImage->getCameraMatrix().has_value();
                auto hasName = !text.isEmpty();
                ui->pushButtonAddToModel->setEnabled(hasCameraMatrix && hasName);
            });
    ui->labelImage->installEventFilter(this);
    ui->pushButtonAddToModel->setEnabled(false);
}

CalibrationParams WidgetPixelSizeCalibration::collectCalibrationParams() const {
    CalibrationParams result;
    result.edgeStrength = ui->spinBoxEdgeStrength->value();
    result.gridSize = cv::Size{
        ui->spinBoxGridWidth->value(),
        ui->spinBoxGridHeight->value()
    };
    result.gridStep = ui->spinBoxGridDist->value();
    if(!mTargetImage->empty()) {
        auto size = mTargetImage->getImage().size();
        result.imageROI = ui->widgetEditorROI->getROI(size);
    }
    return result;
}

void WidgetPixelSizeCalibration::updateWidgets() {
    ui->labelImage->clear();
    ui->textEditLog->setText(mTargetImage->cameraMatrixToString());
    ui->labelFilename->setText(mTargetImage->getFilename());
    ui->pushButtonCalc->setEnabled(!mTargetImage->empty());
    ui->labelImage->update();
    ui->lineEditCalibrationName->clear();    
    ui->pushButtonAddToModel->setEnabled(false);
}

void WidgetPixelSizeCalibration::loadImageFromFile() {
    static auto dir = QString{};
    auto filename = QFileDialog::getOpenFileName(this, tr("Открыть файл"), dir, tr("Изображения (*.bmp *jpg *png)"));
    dir = QFileInfo(filename).dir().path();
    if (!filename.isEmpty()) {
        mTargetImage->loadImage(std::move(filename));
    }
}

void WidgetPixelSizeCalibration::startCalibration() {
    mTargetImage->startCalibration(collectCalibrationParams());
}

void WidgetPixelSizeCalibration::addCalibrationToModel() {
    auto name = ui->lineEditCalibrationName->text().toStdString();
    mCameraModel->addMagnification(std::move(name), *mTargetImage->getCameraMatrix());    
}

bool WidgetPixelSizeCalibration::eventFilter(QObject *watched, QEvent *event) {
    if(watched != ui->labelImage || event->type() != QEvent::Paint) {
        return QWidget::eventFilter(watched, event);;
    }
    auto dstRect = static_cast<QPaintEvent*>(event)->rect();
    QPainter painter(ui->labelImage);
    painter.fillRect(dstRect, QColor(45, 46, 47));
    if(!mTargetImage->empty()) {
        Graphics graphics{painter, dstRect, mTargetImage->getImageRect()};
        mTargetImage->draw(graphics);
        if(auto roi = collectCalibrationParams().imageROI) {
            graphics.drawImageROI(*roi);
        }
    }
    return false;
}
