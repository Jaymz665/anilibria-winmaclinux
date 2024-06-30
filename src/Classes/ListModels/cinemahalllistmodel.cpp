/*
    AniLibria - desktop client for the website anilibria.tv
    Copyright (C) 2021 Roman Vladimirov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QJsonDocument>
#include <QFile>
#include "cinemahalllistmodel.h"
#include "../../globalhelpers.h"

void CinemahallListModel::loadItems() noexcept
{
    QFile file(getCachePath(m_cacheFileName));
    if (!file.open(QFile::ReadOnly | QFile::Text)) return;

    auto json = file.readAll();
    file.close();

    auto document = QJsonDocument::fromJson(json);
    auto jsonArray = document.array();

    m_items->clear();
    foreach (auto item, jsonArray) m_items->append(item.toInt());

    emit countCinemahallChanged();
}

void CinemahallListModel::saveItems() noexcept
{
    QJsonArray cinemahallArray;

    foreach (auto releaseId, *m_items) {
        QJsonValue value(releaseId);
        cinemahallArray.append(value);
    }

    QFile cinemahallFile(getCachePath(m_cacheFileName));
    if (!cinemahallFile.open(QFile::WriteOnly | QFile::Text)) return;

    auto document = QJsonDocument(cinemahallArray);
    cinemahallFile.write(document.toJson());

    cinemahallFile.close();
}

FullReleaseModel *CinemahallListModel::getRelease(int releaseId) const noexcept
{
    auto iterator = std::find_if(
        m_releases->cbegin(),
        m_releases->cend(),
        [releaseId] (FullReleaseModel* release) {
            return release->id() == releaseId;
        }
    );

    if (iterator == m_releases->cend()) return nullptr;

    return *iterator;
}

void CinemahallListModel::clearDraggingState() noexcept
{
    setDragRelease(-1);
    setDropRelease(-1);
}

void CinemahallListModel::refreshItems() noexcept
{
    beginResetModel();
    endResetModel();
}

int CinemahallListModel::getReleaseSeenMarkCount(int releaseId) const noexcept
{
    auto result = 0;
    QHashIterator<QString, bool> iterator(*m_seenMarks);
    while(iterator.hasNext()) {
        iterator.next();

        QString key = iterator.key();
        auto id = QString::number(releaseId);
        if (!key.startsWith(id)) continue;

        result += 1;
    }

    return result;
}

void CinemahallListModel::reorderElements(const int placeIndex, const int oldIndex)
{
    if (placeIndex < 0 || placeIndex > m_items->count()) return;
    if (oldIndex < 0 || oldIndex > m_items->count()) return;

    m_items->removeAt(oldIndex);
    m_items->insert(placeIndex, m_dragRelease);

    saveItems();
    clearDraggingState();
    refreshItems();

    emit needRestoreScrollPosition();
}

CinemahallListModel::CinemahallListModel(QObject *parent)
    : QAbstractListModel{parent}
{
    createIfNotExistsFile(getCachePath(m_cacheFileName), "[]");

    connect(m_itemMenuList.get(), &CommonMenuListModel::itemSelected, this, &CinemahallListModel::itemMenuSelected);

    QStringList options;
    options.append("Переместить в начало");
    options.append("Переместить в конец");
    options.append("Переместить на позицию");
    m_itemMenuList->setup(std::move(options));

    loadItems();
}

void CinemahallListModel::setup(QSharedPointer<QList<FullReleaseModel *> > releases, QHash<QString, bool>* seenMarks)
{
    m_releases = releases;
    m_seenMarks = seenMarks;
}

void CinemahallListModel::setDragRelease(const int dragRelease) noexcept
{
    if (dragRelease == m_dragRelease) return;

    m_dragRelease = dragRelease;
    emit dragReleaseChanged();
}

void CinemahallListModel::setDropRelease(const int dropRelease) noexcept
{
    if (dropRelease == m_dropRelease) return;

    m_dropRelease = dropRelease;
    emit dropReleaseChanged();
}

void CinemahallListModel::setOpenedItemIndex(const int openedItemIndex) noexcept
{
    if (openedItemIndex == m_openedItemIndex) return;

    m_openedItemIndex = openedItemIndex;
    emit openedItemIndexChanged();
    emit validMovedPositionChanged();
}

void CinemahallListModel::setMovedPositionIndex(const int movedPositionIndex) noexcept
{
    if (movedPositionIndex == m_movedPositionIndex) return;

    m_movedPositionIndex = movedPositionIndex;
    emit movedPositionIndexChanged();
    emit validMovedPositionChanged();
}

void CinemahallListModel::setRestoreScrollPosition(const float restoreScrollPosition) noexcept
{
    if (restoreScrollPosition == m_restoreScrollPosition) return;

    m_restoreScrollPosition = restoreScrollPosition;
    emit restoreScrollPositionChanged();
}

QString CinemahallListModel::movedPositionPlaceholder() const noexcept
{
    return "Введите число из диапазона: 1-" + QString::number(m_items->count());
}

int CinemahallListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_items->size();
}

QVariant CinemahallListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    auto releaseId = m_items->at(index.row());
    auto release = getRelease(releaseId);
    if (release == nullptr) return QVariant();

    switch (role) {
        case IdRole: {
            return QVariant(releaseId);
        }
        case TitleRole: {
            return QVariant(release->title());
        }
        case PosterRole: {
            return QVariant(release->poster());
        }
        case SelectedRole: {
            return QVariant(m_selectedItems->contains(releaseId));
        }
        case NumberRole: {
            return QVariant(index.row() + 1);
        }
        case SeenSeries: {
            auto seenReleases = getReleaseSeenMarkCount(releaseId);
            return QVariant(QString::number(seenReleases) + " из " + QString::number(release->countOnlineVideos()));
        }
        default: return QVariant();
    }
}

QHash<int, QByteArray> CinemahallListModel::roleNames() const
{
    return {
        {
            IdRole,
            "id"
        },
        {
            TitleRole,
            "title"
        },
        {
            PosterRole,
            "poster"
        },
        {
            SelectedRole,
            "isSelected"
        },
        {
            NumberRole,
            "releaseNumber"
        },
        {
            SeenSeries,
            "seenSeries"
        }
    };
}

QList<FullReleaseModel *> CinemahallListModel::getCinemahallReleases() const noexcept
{
    QList<FullReleaseModel*> result;
    foreach (auto releaseId, *m_items) {
        auto fullRelease = getRelease(releaseId);
        if (fullRelease == nullptr) continue;

        result.append(fullRelease);
    }

    return result;
}

bool CinemahallListModel::isReleaseInCinemahall(const int releaseId) const noexcept
{
    return m_items->contains(releaseId);
}


void CinemahallListModel::addReleases(const QList<int> &ids)
{
    foreach(auto id, ids) {
        if (m_items->contains(id)) continue;

        m_items->append(id);
    }

    saveItems();
    emit countCinemahallChanged();
    emit hasItemsChanged();
    refreshItems();
}

bool CinemahallListModel::hasCinemahallReleases()
{
    return !m_items->isEmpty();
}

void CinemahallListModel::reorderRelease()
{
    if (m_dragRelease == -1 || m_dropRelease == -1) return;

    reorderElements(m_items->indexOf(m_dropRelease), m_items->indexOf(m_dragRelease));
}

void CinemahallListModel::deleteReleases(const QList<int> &ids)
{
    foreach (auto id, ids) {
        auto index = m_items->indexOf(id);
        if (index == -1) continue;

        m_items->removeAt(index);
    }

    saveItems();
    emit countCinemahallChanged();
    emit hasSelectedItemsChanged();
    emit hasItemsChanged();
    refreshItems();

    emit needRestoreScrollPosition();
}

void CinemahallListModel::deleteSelectedReleases()
{
    if (m_selectedItems->isEmpty()) return;

    QList<int> ids;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ids = m_selectedItems->toList();
#else
    ids = m_selectedItems->values();
#endif

    m_selectedItems->clear();
    deleteReleases(ids);
}

void CinemahallListModel::deleteAllReleases()
{
    m_items->clear();
    m_selectedItems->clear();

    saveItems();
    emit countCinemahallChanged();
    emit hasSelectedItemsChanged();
    emit hasItemsChanged();
    refreshItems();
}

void CinemahallListModel::selectItem(const int releaseId)
{
    if (m_selectedItems->contains(releaseId)) {
        m_selectedItems->remove(releaseId);
    } else {
        m_selectedItems->insert(releaseId);
    }

    const int itemIndex = m_items->indexOf(releaseId);

    emit dataChanged(index(itemIndex), index(itemIndex));
    emit hasSelectedItemsChanged();
}

void CinemahallListModel::deselectItems()
{
    QList<int> ids;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ids = m_selectedItems->toList();
#else
    ids = m_selectedItems->values();
#endif
    m_selectedItems->clear();

    foreach (auto releaseId, ids) {
        const int itemIndex = m_items->indexOf(releaseId);

        emit dataChanged(index(itemIndex), index(itemIndex));
    }

    emit hasSelectedItemsChanged();
}

void CinemahallListModel::deletedSeenReleases()
{
    QList<int> ids;
    auto releases = getCinemahallReleases();
    foreach (auto release, releases) {
        if (release->countOnlineVideos() == getReleaseSeenMarkCount(release->id())){
            ids.append(release->id());
        }
    }
    deleteReleases(ids);
}

void CinemahallListModel::deletedSeenAndFavoritesReleases()
{
    QList<int> ids;
    auto releases = getCinemahallReleases();
    foreach (auto release, releases) {
        if (release->countOnlineVideos() == getReleaseSeenMarkCount(release->id())){
            ids.append(release->id());
        }
    }
    deleteReleases(ids);
    emit needDeleteFavorites(ids);
}

void CinemahallListModel::deletedSeenOnlyFromFavorites()
{
    QList<int> ids;
    auto releases = getCinemahallReleases();
    foreach (auto release, releases) {
        if (release->countOnlineVideos() == getReleaseSeenMarkCount(release->id())){
            ids.append(release->id());
        }
    }
    emit needDeleteFavorites(ids);
}

void CinemahallListModel::moveToTypedNumber()
{
    auto index = m_movedPositionIndex - 1;
    if (index < 0) return;
    if (index == m_openedItemIndex) return;

    reorderElements(index, m_openedItemIndex);
}

void CinemahallListModel::refreshCinemahall()
{
    refreshItems();
}

void CinemahallListModel::itemMenuSelected(const int index)
{
    switch (index) {
        case 0: {// move to start of list
            if (m_openedItemIndex == 0) return;

            reorderElements(0, m_openedItemIndex);
            return;
        }
        case 1: {// move to end of list
            auto lastIndex = m_items->count() - 1;
            if (m_openedItemIndex == lastIndex) return;

            reorderElements(lastIndex, m_openedItemIndex);
            return;
        }
        case 2: {
            emit movedPositionPlaceholderChanged();
            emit visibleSetupMovedNumber();
            return;
        }
    }
}
