#ifndef TAIGA_LIBRARY_MANGA_DB_H
#define TAIGA_LIBRARY_MANGA_DB_H
#include "anime_db.h"

namespace manga {

class Database : public anime::Database {

public:
  Database();
  virtual ~Database();

  virtual int UpdateItem(const anime::Item& item);
//  virtual void UpdateItem(const HistoryItem& history_item);

protected:
  virtual std::wstring GetDatabasePath() const;
  virtual std::wstring GetUserLibraryPath() const;
  virtual wchar_t* GetMediaTypeString() const;
};

} // namespace manga

extern manga::Database MangaDatabase;

#endif TAIGA_LIBRARY_MANGA_DB_H

