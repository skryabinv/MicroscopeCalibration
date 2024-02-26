#include "WidgetEditorROI.h"
#include "ui_WidgetEditorROI.h"

WidgetEditorROI::WidgetEditorROI(QWidget *parent)
    : QGroupBox(parent)
    , ui(new Ui::WidgetEditorROI) {
    ui->setupUi(this);
    for(auto spinBoxROI: {ui->spinBoxLeft, ui->spinBoxRight, ui->spinBoxBottom, ui->spinBoxTop}) {
        connect(spinBoxROI, &QSpinBox::valueChanged,
                this, &WidgetEditorROI::roiChanged);
    }
}

WidgetEditorROI::~WidgetEditorROI() {
    delete ui;
}

std::optional<cv::Rect> WidgetEditorROI::getROI(const cv::Size &imageSize) const {
    auto [leftCrop, rightCrop, bottomCrop, topCrop] = std::tuple{ui->spinBoxLeft->value(),
                                                                 ui->spinBoxRight->value(),
                                                                 ui->spinBoxBottom->value(),
                                                                 ui->spinBoxTop->value()};
    auto roi = cv::Rect(leftCrop, topCrop,
                        imageSize.width - (rightCrop + leftCrop),
                        imageSize.height - (bottomCrop + topCrop));
    if(cv::Rect(0, 0, imageSize.width, imageSize.height) != roi) {
        return  roi;
    }
    return std::nullopt;
}
