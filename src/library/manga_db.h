#ifndef TAIGA_LIBRARY_MANGA_DB_H
#define TAIGA_LIBRARY_MANGA_DB_H
#include "anime_db.h"

namespace manga {

class Database : public anime::Database {

public:
	Database();
	virtual ~Database();

private:
	virtual std::wstring getDatabasePath() const;
	virtual std::wstring getUserLibraryPath() const;
	virtual wchar_t* getMediaTypeString() const;
};

} // namespace manga

extern manga::Database MangaDatabase;

#endif TAIGA_LIBRARY_MANGA_DB_H

