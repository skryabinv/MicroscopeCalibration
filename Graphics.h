#pragma once

#include <QPainter>
#include <opencv2/core.hpp>

class Graphics {
public:
    Graphics(Graphics&&) = delete;
    auto operator=(Graphics&&) = delete;
    Graphics(QPainter& painter,
             const QRectF& dstRect,
             const QRectF& srcRect)
        : mPainter{painter} {
        mPainter.save();
        setFitInViewTransform(dstRect, srcRect);
    }
    ~Graphics() {
        mPainter.restore();
    }

    void drawImage(const cv::Mat& image) const {
        auto qimage = QImage((uchar*)image.data,
                             image.cols,
                             image.rows,
                             QImage::Format_Grayscale8);
        mPainter.drawImage(qimage.rect(), qimage);
    }

    void drawGridPoints(const std::vector<cv::Point2f>& grid) const {
        constexpr auto size = 10.0;
        mPainter.setPen(makeCosmeticPen(Qt::cyan, 1));
        for(const auto& gridPoint: grid) {
            auto cross = std::array{QLineF(gridPoint.x - size, gridPoint.y,
                                           gridPoint.x + size, gridPoint.y),
                                    QLineF(gridPoint.x, gridPoint.y - size,
                                           gridPoint.x, gridPoint.y + size)};
            mPainter.drawLines(cross.data(), (int)cross.size());
        }
    }

    void drawCircle(const cv::Vec3f& circle) {
        mPainter.setPen(makeCosmeticPen(Qt::green, 1));
        auto circleRect = QRectF{circle[0] - circle[2],
                                 circle[1] - circle[2],
                                 2.0 * circle[2],
                                 2.0 * circle[2]};
        mPainter.drawEllipse(circleRect);
    }

    void drawImageROI(const cv::Rect& roi) const {
        mPainter.setPen(makeCosmeticPen(Qt::cyan, 3));
        mPainter.drawRect(QRectF(roi.x, roi.y, roi.width, roi.height));
    }

private:
    static QPen makeCosmeticPen(QColor color, qreal width) {
        auto pen = QPen(color, width);
        pen.setCosmetic(true);
        return pen;
    }
    void setFitInViewTransform(const QRectF& dstRect, const QRectF& srcRect) const {
        auto scale = std::min(dstRect.width() / srcRect.width(), dstRect.height() / srcRect.height());
        mPainter.translate(dstRect.center());
        mPainter.scale(scale, scale);
        mPainter.translate(-srcRect.center());
    }
    QPainter& mPainter;
};

