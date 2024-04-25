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

#include "onlineplayerwindowviewmodel.h"

OnlinePlayerWindowViewModel::OnlinePlayerWindowViewModel(QObject *parent) : QObject(parent),
    m_playerButtonVisible(false),
    m_pauseButtonVisible(false),
    m_opened(true),
    m_isTopMost(false),
    m_windowCursorShape(Qt::ArrowCursor),
    m_panelOpacity(1)
{
    m_supportOutput = m_playerComponent == "Videoplayer/QtPlayer515.qml";
    m_playerComponents.insert(nameVLCPlayer, "Videoplayer/QtVlcPlayer.qml");
    m_playerComponents.insert(nameMpvPlayer, "Videoplayer/MpvPlayer.qml");
    m_playerComponents.insert("Default", "Videoplayer/QtPlayer515.qml");

    m_playerOutputComponents.insert(nameVLCPlayer, "Videoplayer/QtVlcVideoOutput.qml");
    m_playerOutputComponents.insert(nameMpvPlayer, "Videoplayer/MpvPlayerOutput.qml");
    m_playerOutputComponents.insert("Default", "Videoplayer/QtVideo515Output.qml");

    fillSupportedPlayers();

    m_selectedPlayer = m_players.first();
    m_playerComponent = m_playerComponents.value(m_selectedPlayer);
    m_playerOutputComponent = m_playerOutputComponents.value(m_selectedPlayer);
    m_supportOutput = m_playerComponent == "Videoplayer/QtPlayer515.qml";
    m_isHasVlc = m_players.contains(nameVLCPlayer);
    m_isHasMpv = m_players.contains(nameMpvPlayer);
    fillSelectedPlayer();
}

void OnlinePlayerWindowViewModel::setPlayerButtonVisible(const bool &playerButtonVisible) noexcept
{
    if (playerButtonVisible == m_playerButtonVisible) return;

    m_playerButtonVisible = playerButtonVisible;
    emit playerButtonVisibleChanged();
}

void OnlinePlayerWindowViewModel::setPauseButtonVisible(const bool &pauseButtonVisible) noexcept
{
    if (pauseButtonVisible == m_pauseButtonVisible) return;

    m_pauseButtonVisible = pauseButtonVisible;
    emit pauseButtonVisibleChanged();
}

void OnlinePlayerWindowViewModel::setOpened(const bool &opened) noexcept
{
    if (opened == m_opened) return;

    m_opened = opened;
    emit openedChanged();
}

void OnlinePlayerWindowViewModel::setIsTopMost(const bool &isTopMost) noexcept
{
    if (isTopMost == m_isTopMost) return;

    m_isTopMost = isTopMost;
    emit isTopMostChanged();
}

void OnlinePlayerWindowViewModel::setWindowCursorShape(const int &windowCursorShape) noexcept
{
    if (windowCursorShape == m_windowCursorShape) return;

    m_windowCursorShape = windowCursorShape;
    emit windowCursorShapeChanged();
}

void OnlinePlayerWindowViewModel::setPanelOpacity(int panelOpacity) noexcept
{
    if (m_panelOpacity != panelOpacity) {
        m_panelOpacity = panelOpacity;
        emit panelOpacityChanged();
    }
}

void OnlinePlayerWindowViewModel::setStartupPlayer(const QString &startupPlayer) noexcept
{
    if (m_startupPlayer == startupPlayer) return;

    m_startupPlayer = startupPlayer;
    emit startupPlayerChanged();

    if (!m_alreadyRestorePlayer && !startupPlayer.isEmpty()) {
        m_alreadyRestorePlayer = true;
        if (m_players.contains(startupPlayer)) changePlayer(startupPlayer);
    }
}

void OnlinePlayerWindowViewModel::playbackStateChanged(const bool &isPlaying)
{
    setPlayerButtonVisible(!isPlaying);
    setPauseButtonVisible(!m_playerButtonVisible);
}

void OnlinePlayerWindowViewModel::hideControlPanel()
{
    setPanelOpacity(0);
    setWindowCursorShape(Qt::BlankCursor);
}

void OnlinePlayerWindowViewModel::showPanel()
{
    setPanelOpacity(1);
    setWindowCursorShape(Qt::ArrowCursor);
}

void OnlinePlayerWindowViewModel::clearCurrentPlayer()
{
    m_playerComponent = "";
    m_playerOutputComponent = "";
    emit playerComponentChanged();
    emit playerOutputComponentChanged();
}

void OnlinePlayerWindowViewModel::changePlayer(const QString &player)
{
    clearCurrentPlayer();

    m_isSelectedVlc = nameVLCPlayer == player;
    m_isSelectedMpv = nameMpvPlayer == player;
    m_selectedPlayer = player;
    emit isSelectedQtAvChanged();
    emit isSelectedVlcChanged();
    emit selectedPlayerChanged();
    emit isSelectedMpvChanged();

    m_playerComponent = m_playerComponents.value(player);
    m_playerOutputComponent = m_playerOutputComponents.value(player);
    m_supportOutput = m_playerComponent == "Videoplayer/QtPlayer515.qml";

    emit supportOutputChanged();
    emit playerComponentChanged();
    emit playerOutputComponentChanged();
}

void OnlinePlayerWindowViewModel::fillSupportedPlayers()
{
    bool isVlc = false;
    bool isMpv = false;
#ifdef USE_VLC_PLAYER
    isVlc = true;
#endif
#ifdef USE_MPV_PLAYER
    isMpv = true;
#endif

    if (isVlc) m_players.append(nameVLCPlayer);
    if (isMpv) m_players.append(nameMpvPlayer);
#ifndef NO_NEED_STANDART_PLAYER
    m_players.append("Default");
#endif
}

void OnlinePlayerWindowViewModel::fillSelectedPlayer()
{
    m_isSelectedVlc = nameVLCPlayer == m_selectedPlayer;
    m_isSelectedMpv = nameMpvPlayer == m_selectedPlayer;
}
