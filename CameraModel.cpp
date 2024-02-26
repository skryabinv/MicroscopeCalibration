#include "CameraModel.h"
#include <QTreeWidget>

CameraModel::CameraModel(QObject *parent)
    : QObject{parent}
{}

void CameraModel::clear() {
    mMagnifications.clear();
    emit changed();
}

void CameraModel::addMagnification(std::string name, const cv::Matx33d &cameraMatrix) {
    auto mx = 1.0 / cameraMatrix(0, 0);
    auto my = 1.0 / cameraMatrix(1, 1);
    mMagnifications.emplace_back(Magnification{std::move(name), cv::Size2d{mx, my}});
    emit changed();
}

void CameraModel::setOpticalCenter(const cv::Point2d &pos) {
    mOpticalCenter = pos;
    emit changed();
}

void CameraModel::saveToFile(const QString &filename) const {
    cv::FileStorage storage(filename.toUtf8().toStdString(), cv::FileStorage::WRITE);
    if(mOpticalCenter) {
        storage << "optical_center" << *mOpticalCenter;
    }
    storage << "magnifications" << "[";
    for(const auto& magn: mMagnifications) {
        storage << "{"
                << "name" << magn.name
                << "pixel_size" << magn.pixelSize
                << "}";
    }
    storage << "]";
    storage.release();
}

void CameraModel::loadFromFile(const QString &filename) {
    mMagnifications.clear();
    cv::FileStorage storage(filename.toUtf8().toStdString(), cv::FileStorage::READ);
    if(auto node = storage["optical_center"]; !node.empty()) {
        mOpticalCenter.emplace();
        node >> *mOpticalCenter;
    }
    for(const auto& node: storage["magnifications"]) {
        Magnification magn;
        node["name"] >> magn.name;
        node["pixel_size"] >> magn.pixelSize;
        mMagnifications.emplace_back(std::move(magn));
    }
    emit changed();
}

void CameraModel::updateUi(QTreeWidget *ui) const {
    ui->clear();
    if(mOpticalCenter) {
        ui->addTopLevelItem(makeOpticalCenterItem());
    }
    if(!empty()) {
        auto items = QList<QTreeWidgetItem*>{};
        std::transform(mMagnifications.cbegin(),
                       mMagnifications.cend(),
                       std::back_inserter(items),
                       makeMagnificationItem);
        ui->addTopLevelItems(items);
    }
    ui->expandAll();
}

bool CameraModel::empty() const {
    return mMagnifications.empty();
}

static auto makeItemForFloatingNumber(QTreeWidgetItem* root,
                                      const QString& title,
                                      double value, int precision) {
    auto item = new QTreeWidgetItem(root);
    item->setText(0, title);
    item->setText(1,  QString::number(value, 'f', precision));
}

QTreeWidgetItem *CameraModel::makeMagnificationItem(const Magnification &magnification) {
    auto item = new QTreeWidgetItem();
    item->setText(0, QString::fromUtf8(magnification.name));
    makeItemForFloatingNumber(item, tr("Ширина пиксела"), magnification.pixelSize.width, 7);
    makeItemForFloatingNumber(item, tr("Высота пиксела"), magnification.pixelSize.height, 7);
    return item;
}

QTreeWidgetItem *CameraModel::makeOpticalCenterItem() const {
    auto item = new QTreeWidgetItem();
    item->setText(0, tr("Оптический центр"));
    makeItemForFloatingNumber(item, tr("X"), mOpticalCenter->x, 3);
    makeItemForFloatingNumber(item, tr("Y"), mOpticalCenter->y, 3);
    return item;
}

