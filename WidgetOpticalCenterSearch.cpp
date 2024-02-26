#include "WidgetOpticalCenterSearch.h"
#include "ui_WidgetOpticalCenterSearch.h"
#include "TargetImage.h"
#include "CameraModel.h"
#include "Graphics.h"
#include <QFileDialog>
#include <QPaintEvent>
#include "OpticalCenterCostFunction.h"
#include <ceres/ceres.h>

WidgetOpticalCenterSearch::WidgetOpticalCenterSearch(CameraModel *cameraModel, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::WidgetOpticalCenterSearch),
    mCameraModel{cameraModel} {
    assert(mCameraModel != nullptr);
    ui->setupUi(this);    
    setupWidgets();
    setupTargetImage();
    updateWidgets();
}

WidgetOpticalCenterSearch::~WidgetOpticalCenterSearch() {
    delete ui;
}

bool WidgetOpticalCenterSearch::eventFilter(QObject *watched, QEvent *event) {
    if(watched != ui->labelImage || event->type() != QEvent::Paint) {
        return QWidget::eventFilter(watched, event);;
    }
    auto dstRect = static_cast<QPaintEvent*>(event)->rect();
    QPainter painter(ui->labelImage);
    painter.fillRect(dstRect, QColor(45, 46, 47));
    if(!mTargetImage->empty()) {
        Graphics graphics{painter, dstRect, mTargetImage->getImageRect()};
        mTargetImage->draw(graphics);
        if(auto roi = ui->widgetROI->getROI(mTargetImage->getImage().size())) {
            graphics.drawImageROI(*roi);
        }
        for(const auto& circle: mDetectedCircles) {
            graphics.drawCircle(circle);
        }
    }
    return false;
}

void WidgetOpticalCenterSearch::setupWidgets() {
    ui->labelImage->installEventFilter(this);
    connect(ui->pushButtonLoadImage, &QPushButton::clicked,
            this, &WidgetOpticalCenterSearch::loadImage);
    connect(ui->pushButtonFineCircle, &QPushButton::clicked,
            this, &WidgetOpticalCenterSearch::findTrackedCircle);
    connect(ui->pushButtonCalcOpticCenter, &QPushButton::clicked,
            this, &WidgetOpticalCenterSearch::calculateOpticalCenter);
    connect(ui->pushButtonAddToCameraModel, &QPushButton::clicked,
            this, &WidgetOpticalCenterSearch::addOpticalCenterToModel);
    connect(ui->widgetROI, &WidgetEditorROI::roiChanged,
            ui->labelImage, QOverload<>::of(&QLabel::update));
    connect(ui->pushButtonClear, &QPushButton::clicked, this, [this]{
        mOpticalCenter.reset();
        mDetectedCircles.clear();
        ui->textEditLog->clear();
        update();
    });
}

void WidgetOpticalCenterSearch::setupTargetImage() {
    mTargetImage = new TargetImage{};
    connect(mTargetImage, &TargetImage::error,
            this, &WidgetOpticalCenterSearch::error);
    connect(mTargetImage, &TargetImage::changed,
            this, &WidgetOpticalCenterSearch::onNewImage);
}

void WidgetOpticalCenterSearch::updateWidgets() {
    ui->pushButtonCalcOpticCenter->setEnabled(mDetectedCircles.size() >= 2);
    ui->pushButtonClear->setEnabled(!mDetectedCircles.empty());
    ui->pushButtonAddToCameraModel->setEnabled(mOpticalCenter.has_value());
    ui->pushButtonFineCircle->setDisabled(mTargetImage->empty());
    ui->labelFilename->setText(mTargetImage->getFilename());
    update();
}

void WidgetOpticalCenterSearch::loadImage() {
    static auto dir = QString{};
    auto filename = QFileDialog::getOpenFileName(this, tr("Открыть файл"), dir, tr("Изображения (*.bmp *.jpg *.png)"));
    dir = QFileInfo(filename).dir().path();
    if (!filename.isEmpty()) {
        mTargetImage->loadImage(std::move(filename));
    }    
}

void WidgetOpticalCenterSearch::findTrackedCircle() {
    auto imageROI = ui->widgetROI->getROI(mTargetImage->getImage().size());
    auto circles = mTargetImage->detectGridCircles(cv::Size{1, 1}, 50.0, imageROI);
    if(!circles.empty()) {
        mDetectedCircles.push_back(circles.back());
        std::cout << __FUNCTION__ << mDetectedCircles.back() << std::endl;
    }    
    updateWidgets();
}

void WidgetOpticalCenterSearch::calculateOpticalCenter() {
    if(mDetectedCircles.size() <= 1) {
        emit error(tr("Необходимо добавить хотя бы 2 окружности"));
        return;
    }
    ceres::Problem problem;
    auto refCircle = mDetectedCircles.front();
    std::vector<double> params(2 + 2 * (mDetectedCircles.size() - 1));
    params[0] = refCircle[0];
    params[1] = refCircle[1];
    for(size_t i = 1; i < mDetectedCircles.size(); i++) {
        auto circle = mDetectedCircles[i];
        // Начальная оценка радиуса
        params[2] = circle[2] / refCircle[2];
        params[3] = params[2];
        auto costs = makeOpticalCenterCostFunctions(circle, refCircle, i, 10);
        for(auto cost: costs) {
            auto costAutoDiff = new ceres::AutoDiffCostFunction<OpticalCenterCostFunction, 2, 4>(cost);
            problem.AddResidualBlock(costAutoDiff, nullptr, params.data());
        }
    }
    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.minimizer_progress_to_stdout = true;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    if(summary.IsSolutionUsable()) {
        std::ostringstream os;
        os << summary.BriefReport() << std::endl;
        os << cv::Mat{params} << std::endl;
        ui->textEditLog->setText(QString::fromStdString(os.str()));
        mOpticalCenter.emplace(params[0], params[1]);
        updateWidgets();
    }
}

void WidgetOpticalCenterSearch::addOpticalCenterToModel() {
    if(mOpticalCenter.has_value()) {
        mCameraModel->setOpticalCenter(*mOpticalCenter);        
    }
}

void WidgetOpticalCenterSearch::onNewImage() {    
    updateWidgets();
    ui->labelImage->update();
}

