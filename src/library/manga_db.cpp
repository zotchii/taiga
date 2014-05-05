#include "manga_db.h"
#include "taiga/path.h"

manga::Database MangaDatabase;

namespace manga {

Database::Database() {
}

Database::~Database() {
}

std::wstring Database::getDatabasePath() const {
	return taiga::GetPath(taiga::PathType::kPathDatabaseManga);
}

std::wstring Database::getUserLibraryPath() const {
	return taiga::GetPath(taiga::PathType::kPathUserMangaLibrary);
}

wchar_t* Database::getMediaTypeString() const {
	return L"manga";
}

} // namespace manga