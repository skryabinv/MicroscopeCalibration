#pragma once

#include <QObject>
#include <opencv2/core.hpp>

class QTreeWidget;
class QTreeWidgetItem;

class CameraModel : public QObject {
    Q_OBJECT
public:
    explicit CameraModel(QObject *parent = nullptr);
    void clear();
    void addMagnification(std::string name, const cv::Matx33d& cameraMatrix);
    void setOpticalCenter(const cv::Point2d& pos);
    void saveToFile(const QString& filename) const;
    void loadFromFile(const QString& filename);
    void updateUi(QTreeWidget* ui) const;
    bool empty() const;
signals:
    void changed();
private:
    std::optional<cv::Point2d> mOpticalCenter;
    struct Magnification {
        std::string name;
        cv::Size2d pixelSize;        
    };
    static QTreeWidgetItem* makeMagnificationItem(const Magnification& magnification);
    QTreeWidgetItem* makeOpticalCenterItem() const;
    std::vector<Magnification> mMagnifications;
};
