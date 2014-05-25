#include "library/manga_db.h"
#include "taiga/path.h"

manga::Database MangaDatabase;

namespace manga {

Database::Database() {}

Database::~Database() {}

int Database::UpdateItem(const anime::Item& item) {
  int id = item.GetId();
  items[id] = item;
  return id;
}

//void Database::UpdateItem(const HistoryItem& history_item) {}

std::wstring Database::GetDatabasePath() const {
  return taiga::GetPath(taiga::PathType::kPathDatabaseManga);
}

std::wstring Database::GetUserLibraryPath() const {
  return taiga::GetPath(taiga::PathType::kPathUserMangaLibrary);
}

wchar_t* Database::GetMediaTypeString() const {
  return L"manga";
}

} // namespace manga