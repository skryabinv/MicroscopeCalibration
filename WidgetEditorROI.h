#pragma once

#include <QGroupBox>
#include <opencv2/core/types.hpp>
#include <optional>

namespace Ui {
class WidgetEditorROI;
}

class WidgetEditorROI : public QGroupBox {
    Q_OBJECT
signals:
    void roiChanged();
public:
    explicit WidgetEditorROI(QWidget *parent = nullptr);
    ~WidgetEditorROI();
    std::optional<cv::Rect> getROI(const cv::Size& imageSize) const;
private:
    Ui::WidgetEditorROI *ui;
};
