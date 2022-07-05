/*
    AniLibria - desktop client for the website anilibria.tv
    Copyright (C) 2020 Roman Vladimirov

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

#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include "fullreleasemodel.h"
#include "globalconstants.h"

void FullReleaseModel::setTitle(const QString &title) noexcept
{
    m_Title = title;
}

void FullReleaseModel::setStatus(const QString &status) noexcept
{
    m_Status = status;
}

void FullReleaseModel::setYear(const QString &year) noexcept
{
    m_Year = year;
}

void FullReleaseModel::setPoster(const QString &poster) noexcept
{
    if (poster.contains("https://")) {
        m_Poster = poster;
    } else {
        m_Poster = AnilibriaImagesPath + poster;
    }
}

void FullReleaseModel::setDescription(const QString &description) noexcept
{
    m_Description = description;
}

void FullReleaseModel::setReleaseType(const QString &releaseType) noexcept
{
    m_Type = releaseType;
}

void FullReleaseModel::setGenres(const QString &genres) noexcept
{
    m_Genres = genres;
}

void FullReleaseModel::setVoicers(const QString &voicers) noexcept
{
    m_Voices = voicers;
}

void FullReleaseModel::setSeason(const QString &season) noexcept
{
    m_Season = season;
}

void FullReleaseModel::setSeries(const QString &series) noexcept
{
    m_Series = series;
}

void FullReleaseModel::setCode(const QString &code) noexcept
{
    m_Code = code;
}

void FullReleaseModel::setId(const int id) noexcept
{
    m_Id = id;
}

void FullReleaseModel::setCountOnlineVideos(const int countOnlineVideos) noexcept
{
    m_CountVideos = countOnlineVideos;
}

void FullReleaseModel::setCountTorrents(const int countTorrents) noexcept
{
    m_CountTorrents = countTorrents;
}

void FullReleaseModel::setAnnounce(const QString &announce) noexcept
{
    m_Announce = announce;
}

void FullReleaseModel::setOriginalName(const QString &originalName) noexcept
{
    m_OriginalName = originalName;
}

void FullReleaseModel::setRating(const int rating) noexcept
{
    m_Rating = rating;
}

void FullReleaseModel::setTorrents(const QString &torrents) noexcept
{
    m_Torrents = torrents;
}

void FullReleaseModel::setVideos(const QString &videos) noexcept
{
    m_Videos = videos;
}

void FullReleaseModel::setTimestamp(const int timestamp) noexcept
{
    m_Timestamp = timestamp;
}

void FullReleaseModel::setType(const QString &type) noexcept
{
    m_Type = type;
}

void FullReleaseModel::setIsDeleted(const bool isDeleted) noexcept
{
    m_isDeleted = isDeleted;
}

void FullReleaseModel::writeToJson(QJsonObject &json) const noexcept
{
    json["id"] = m_Id;
    json["title"] = m_Title;
    json["code"] = m_Code;
    json["originalName"] = m_OriginalName;
    json["rating"] = m_Rating;
    json["series"] = m_Series;
    json["status"] = m_Status;
    json["type"] = m_Type;
    json["timestamp"] = m_Timestamp;
    json["year"] = m_Year;
    json["season"] = m_Season;
    json["countTorrents"] = m_CountTorrents;
    json["countVideos"] = m_CountVideos;
    json["description"] = m_Description;
    json["announce"] = m_Announce;
    json["genres"] = m_Genres;
    json["poster"] = m_Poster;
    json["voices"] = m_Voices;
    json["torrents"] = m_Torrents;
    json["videos"] = m_Videos;
    json["isDeleted"] = m_isDeleted;
}

void FullReleaseModel::readFromJson(QJsonValueRef &json)
{
    setId(json[QString("id")].toInt());
    setTitle(json[QString("title")].toString());
    setCode(json[QString("code")].toString());
    setOriginalName(json[QString("originalName")].toString());
    setRating(json[QString("rating")].toInt());
    setSeries(json[QString("series")].toString());
    setStatus(json[QString("status")].toString());
    setType(json[QString("type")].toString());
    setTimestamp(json[QString("timestamp")].toInt());
    setYear(json[QString("year")].toString());
    setSeason(json[QString("season")].toString());
    setCountTorrents(json[QString("countTorrents")].toInt());
    setCountOnlineVideos(json[QString("countVideos")].toInt());
    setDescription(json[QString("description")].toString());
    setAnnounce(json[QString("announce")].toString());
    setGenres(json[QString("genres")].toString());
    setPoster(json[QString("poster")].toString());
    setVoicers(json[QString("voices")].toString());
    setTorrents(json[QString("torrents")].toString());
    setVideos(json[QString("videos")].toString());
    setIsDeleted(json[QString("isDeleted")].toBool());
}
bool FullReleaseModel::operator== (const FullReleaseModel &comparedModel) noexcept
{
    return m_Id == comparedModel.id();
}
