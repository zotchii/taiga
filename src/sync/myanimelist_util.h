/*
** Taiga
** Copyright (C) 2010-2013, Eren Okka
** 
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TAIGA_SYNC_MYANIMELIST_UTIL_H
#define TAIGA_SYNC_MYANIMELIST_UTIL_H

#include <string>

namespace sync {
namespace myanimelist {

std::wstring DecodeText(std::wstring text);

int TranslateSeriesStatusFrom(int value);
int TranslateSeriesStatusFrom(const std::wstring& value);
int TranslateSeriesTypeFrom(int value);
int TranslateSeriesTypeFrom(const std::wstring& value);
std::wstring TranslateMyDateTo(const std::wstring& value);
int TranslateMyStatusFrom(int value);
int TranslateMyStatusTo(int value);
std::wstring TranslateKeyTo(const std::wstring& key);

void ViewAnimePage(int anime_id);
void ViewAnimeSearch(const std::wstring& title);
void ViewHistory();
void ViewMessages();
void ViewPanel();
void ViewProfile();
void ViewSeasonGroup();
void ViewUpcomingAnime();

}  // namespace myanimelist
}  // namespace sync

#endif  // TAIGA_SYNC_MYANIMELIST_UTIL_H