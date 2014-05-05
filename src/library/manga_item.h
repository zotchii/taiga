#ifndef TAIGA_LIBRARY_MANGA_ITEM_H
#define TAIGA_LIBRARY_MANGA_ITEM_H
#include "library/anime_item.h"

namespace manga {

	class Item : public anime::Item {
	public:
		Item();
		virtual ~Item();
	};
}

#endif  // TAIGA_LIBRARY_MANGA_ITEM_H

