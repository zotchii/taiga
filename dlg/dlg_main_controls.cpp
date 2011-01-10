/*
** Taiga, a lightweight client for MyAnimeList
** Copyright (C) 2010-2011, Eren Okka
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

#include "../std.h"
#include "../animelist.h"
#include "../common.h"
#include "dlg_main.h"
#include "../event.h"
#include "../gfx.h"
#include "../myanimelist.h"
#include "../resource.h"
#include "../settings.h"
#include "../string.h"
#include "../taiga.h"
#include "../theme.h"
#include "../win32/win_gdi.h"

// =============================================================================

/* TreeView control */

void CMainWindow::CMainTree::RefreshItems() {
  // Clear items
  DeleteAllItems();
  for (int i = 0; i < 7; i++) {
    htItem[i] = NULL;
  }
  
  // My tralala
  htItem[0] = InsertItem(L"My Panel", NULL, NULL);
  htItem[1] = InsertItem(L"My Profile", NULL, NULL);
  htItem[2] = InsertItem(L"My History", NULL, NULL);

  // Seperator
  htItem[3] = InsertItem(NULL, -1, NULL);
  
  // My Anime List
  htItem[4] = InsertItem(L"My Anime List", NULL, NULL);
  InsertItem(MAL.TranslateMyStatus(MAL_WATCHING, true).c_str(), NULL, htItem[4]);
  InsertItem(MAL.TranslateMyStatus(MAL_COMPLETED, true).c_str(), NULL, htItem[4]);
  InsertItem(MAL.TranslateMyStatus(MAL_ONHOLD, true).c_str(), NULL, htItem[4]);
  InsertItem(MAL.TranslateMyStatus(MAL_DROPPED, true).c_str(), NULL, htItem[4]);
  InsertItem(MAL.TranslateMyStatus(MAL_PLANTOWATCH, true).c_str(), NULL, htItem[4]);
  Expand(htItem[4]);

  // Seperator
  htItem[5] = InsertItem(NULL, -1, NULL);

  // Foobar
  htItem[6] = InsertItem(L"Foo", NULL, NULL);
  InsertItem(L"Foofoo", NULL, htItem[6]);
  InsertItem(L"Foobar", NULL, htItem[6]);
  InsertItem(L"Foobaz", NULL, htItem[6]);
  Expand(htItem[6]);
}

LRESULT CMainWindow::OnTreeNotify(LPARAM lParam) {
  LPNMHDR pnmh = reinterpret_cast<LPNMHDR>(lParam);

  switch (pnmh->code) {
    // Custom draw
    case NM_CUSTOMDRAW: {
      LPNMLVCUSTOMDRAW pCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(lParam);
      switch (pCD->nmcd.dwDrawStage) {
        case CDDS_PREPAINT:
          return CDRF_NOTIFYITEMDRAW;
        case CDDS_ITEMPREPAINT:
          return CDRF_NOTIFYPOSTPAINT;
        case CDDS_ITEMPOSTPAINT: {
          // Draw seperator
          if (pCD->nmcd.lItemlParam == -1) {
            CRect rcItem = pCD->nmcd.rc;
            CDC hdc = pCD->nmcd.hdc;
            hdc.FillRect(rcItem, RGB(255, 255, 255));
            rcItem.Inflate(-8, 0);
            rcItem.top += (rcItem.bottom - rcItem.top) / 2;
            GradientRect(hdc.Get(), &rcItem, RGB(245, 245 ,245), RGB(255, 255, 255), true);
            rcItem.bottom = rcItem.top + 2;
            hdc.FillRect(rcItem, RGB(255, 255, 255));
            rcItem.bottom -= 1;
            hdc.FillRect(rcItem, RGB(230, 230, 230));
            hdc.DetachDC();
          }
          return CDRF_DODEFAULT;
        }
      }
    }
  }

  return 0;
}

// =============================================================================

/* ListView control */

int CMainWindow::CMainList::GetSortType(int column) {
  switch (column) {
    // Score
    case 2:
      return LISTSORTTYPE_NUMBER;
    // Season
    case 4:
      return LISTSORTTYPE_SEASON;
    // Other columns
    default:
      return LISTSORTTYPE_DEFAULT;
  }
}

LRESULT CMainWindow::CMainList::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_MBUTTONDOWN: {
      int item_index = HitTest();
      if (item_index > -1) {
        SetSelectedItem(item_index);
        switch (Settings.Program.List.MiddleClick) {
          case 1: ExecuteAction(L"EditAll");    break;
          case 2: ExecuteAction(L"OpenFolder"); break;
          case 3: ExecuteAction(L"PlayNext");   break;
          case 4: ExecuteAction(L"Info");       break;
        }
      }
      break;
    }
  }
  
  return WindowProcDefault(hwnd, uMsg, wParam, lParam);
}

LRESULT CMainWindow::OnListNotify(LPARAM lParam) {
  LPNMHDR pnmh = reinterpret_cast<LPNMHDR>(lParam);
  switch (pnmh->code) {
    // Item drag
    case LVN_BEGINDRAG: {
      POINT pt = {0};
      LPNMLISTVIEW lplv = reinterpret_cast<LPNMLISTVIEW>(lParam);
      m_List.m_DragImage = m_List.CreateDragImage(lplv->iItem, &pt);
      if (m_List.m_DragImage.GetHandle()) {
        pt = lplv->ptAction;
        m_List.m_DragImage.BeginDrag(0, 0, 0);
        m_List.m_DragImage.DragEnter(g_hMain, pt.x, pt.y);
        m_List.m_bDragging = true;
        SetCapture();
      }
      break;
    }
    
    // Column click
    case LVN_COLUMNCLICK: {
      LPNMLISTVIEW lplv = (LPNMLISTVIEW)lParam;
      int iOrder = m_List.GetSortOrder() * -1;
      if (iOrder == 0) iOrder = 1;
      m_List.Sort(lplv->iSubItem, iOrder, m_List.GetSortType(lplv->iSubItem), ListViewCompareProc);
      break;
    }

    // Item select
    case LVN_ITEMCHANGED: {
      LPNMLISTVIEW lplv = reinterpret_cast<LPNMLISTVIEW>(lParam);
      CAnime* pItem = reinterpret_cast<CAnime*>(lplv->lParam);
      if (pItem) {
        AnimeList.Index = pItem->Index;
      } else {
        AnimeList.Index = 0;
      }
      break;
    }

      // Double click
    case NM_DBLCLK: {
      if (m_List.GetSelectedCount() > 0) {
        switch (Settings.Program.List.DoubleClick) {
          case 1: ExecuteAction(L"EditAll");    break;
          case 2: ExecuteAction(L"OpenFolder"); break;
          case 3: ExecuteAction(L"PlayNext");   break;
          case 4: ExecuteAction(L"Info");       break;
        }
      }
      break;
    }

    // Right click
    case NM_RCLICK: {
      if (pnmh->hwndFrom == m_List.GetWindowHandle()) {
        if (m_List.GetSelectedCount() > 0) {
          UpdateAllMenus(AnimeList.Index);
          int index = m_List.HitTest(true);
          ExecuteAction(UI.Menus.Show(g_hMain, 0, 0, index == 2 ? L"EditScore" : L"RightClick"));
          RefreshMenubar(AnimeList.Index);
        }
      } else if (pnmh->hwndFrom == m_List.GetHeader()) {
        HDHITTESTINFO hdhti;
        ::GetCursorPos(&hdhti.pt);
        ::ScreenToClient(m_List.GetHeader(), &hdhti.pt);
        if (::SendMessage(m_List.GetHeader(), HDM_HITTEST, 0, reinterpret_cast<LPARAM>(&hdhti))) {
          if (hdhti.iItem == 3) {
            ExecuteAction(UI.Menus.Show(m_hWindow, 0, 0, L"FilterType"));
            RefreshMenubar(AnimeList.Index);
            return TRUE;
          }
        }
      }
      break;
    }

    // Text callback
    case LVN_GETDISPINFO: {
      NMLVDISPINFO* plvdi = reinterpret_cast<NMLVDISPINFO*>(lParam);
      CAnime* pItem = reinterpret_cast<CAnime*>(plvdi->item.lParam);
      if (!pItem) break;
      switch (plvdi->item.iSubItem) {
        case 0: // Anime title
          plvdi->item.pszText = const_cast<LPWSTR>(pItem->Series_Title.data());
          break;
      }
      break;
    }

    // Key press
    case LVN_KEYDOWN: {
      LPNMLVKEYDOWN pnkd = reinterpret_cast<LPNMLVKEYDOWN>(lParam);
      switch (pnkd->wVKey) {
        // Delete item
        case VK_DELETE: {
          if (m_List.GetSelectedCount() > 0) {
            ExecuteAction(L"EditDelete()");
          }
          break;
        }
        // Check new episodes
        case VK_F5: {
          ExecuteAction(L"CheckNewEpisodes");
          break;
        }
      }
      break;
    }

    // Custom draw
    case NM_CUSTOMDRAW: {
      return OnListCustomDraw(lParam);
    }
  }

  return 0;
}

LRESULT CMainWindow::OnListCustomDraw(LPARAM lParam) {
  LPNMLVCUSTOMDRAW pCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(lParam);

  switch (pCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      return CDRF_NOTIFYITEMDRAW;
    case CDDS_ITEMPREPAINT:
      return CDRF_NOTIFYSUBITEMDRAW;
    case CDDS_PREERASE:
    case CDDS_ITEMPREERASE:
      return CDRF_NOTIFYPOSTERASE;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM: {
      CAnime* pAnimeItem = reinterpret_cast<CAnime*>(pCD->nmcd.lItemlParam);
      // Alternate background color
      if ((pCD->nmcd.dwItemSpec % 2) && !m_List.IsGroupViewEnabled()) {
        pCD->clrTextBk = RGB(248, 248, 248);
      }
      // Change text color
      if (!pAnimeItem) return CDRF_NOTIFYPOSTPAINT;
      if (pAnimeItem->Series_Status == MAL_NOTYETAIRED) {
        pCD->clrText = GetSysColor(COLOR_GRAYTEXT);
      } else if (pAnimeItem->NewEps) {
        if (Settings.Program.List.Highlight) {
          pCD->clrText = GetSysColor(pCD->iSubItem == 0 ? COLOR_HIGHLIGHT : COLOR_WINDOWTEXT);
        }
      }
      // Indicate currently playing
      if (pAnimeItem->Playing) {
        pCD->clrTextBk = RGB(230, 255, 230);
        static HFONT hFontDefault = ChangeDCFont(pCD->nmcd.hdc, NULL, -1, true, -1, -1);
        static HFONT hFontBold = reinterpret_cast<HFONT>(GetCurrentObject(pCD->nmcd.hdc, OBJ_FONT));
        SelectObject(pCD->nmcd.hdc, pCD->iSubItem == 0 ? hFontBold : hFontDefault);
        return CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
      }
      return CDRF_NOTIFYPOSTPAINT;
    }
    
    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM: {
      CAnime* pAnimeItem = reinterpret_cast<CAnime*>(pCD->nmcd.lItemlParam);
      if (!pAnimeItem) return CDRF_DODEFAULT;
      int eps_watched  = pAnimeItem->My_WatchedEpisodes;
      int eps_total    = pAnimeItem->Series_Episodes;
      int eps_estimate = pAnimeItem->GetTotalEpisodes();
      int eps_buffer   = pAnimeItem->GetLastWatchedEpisode();
      if (eps_buffer == eps_watched) eps_buffer = 0;
      
      // Draw progress bar
      if (pCD->iSubItem == 1) {
        CRect rcItem;
        if (GetWinVersion() < WINVERSION_VISTA) {
          m_List.GetSubItemRect(pCD->nmcd.dwItemSpec, pCD->iSubItem, &rcItem);
        } else {
          rcItem = pCD->nmcd.rc;
        }
        if (rcItem.IsEmpty()) return CDRF_DODEFAULT;
        CDC hdc = pCD->nmcd.hdc;
        CRect rcText = rcItem;
        
        // Draw border
        rcItem.Inflate(-2, -2);
        UI.ListProgress.Border.Draw(hdc.Get(), &rcItem);
        // Draw background
        rcItem.Inflate(-1, -1);
        UI.ListProgress.Background.Draw(hdc.Get(), &rcItem);
        
        // Draw gradient
        if (eps_watched > 0 || eps_buffer > 0) {
          float ratio_watched, ratio_buffer;
          if (eps_total == 0) {
            // Estimate episode count
            if (eps_estimate) {
              ratio_watched = static_cast<float>(eps_watched) / static_cast<float>(eps_estimate);
            } else {
              ratio_watched = eps_buffer > 0 ? 0.75f : 0.8f;
              ratio_buffer = eps_buffer > 0 ? 0.8f : 0.0f;
            }
          } else {
            ratio_watched = static_cast<float>(eps_watched) / static_cast<float>(eps_total);
            ratio_buffer = static_cast<float>(eps_buffer) / static_cast<float>(eps_total);
          }

          CRect rcBuffer = rcItem;
          rcItem.right = static_cast<int>((rcItem.right - rcItem.left) * ratio_watched) + rcItem.left;
          
          // Draw buffer
          if (eps_buffer > 0) {
            rcBuffer.right = static_cast<int>((rcBuffer.right - rcBuffer.left) * ratio_buffer) + rcBuffer.left;
            UI.ListProgress.Buffer.Draw(hdc.Get(), &rcBuffer);
            rcBuffer.left = rcItem.right;
            rcBuffer.right = rcItem.right + 1;
            UI.ListProgress.Seperator.Draw(hdc.Get(), &rcBuffer);
          }
          
          // Completed
          if (ratio_watched == 1.0f) {
            UI.ListProgress.Completed.Draw(hdc.Get(), &rcItem);
          // Watching
          } else if (pAnimeItem->GetStatus() == MAL_WATCHING) {
            UI.ListProgress.Watching.Draw(hdc.Get(), &rcItem);
          // Dropped
          } else if (pAnimeItem->GetStatus() == MAL_DROPPED) {
            UI.ListProgress.Dropped.Draw(hdc.Get(), &rcItem);
          // Completed / On hold / Plan to watch
          } else {
            UI.ListProgress.Completed.Draw(hdc.Get(), &rcItem);
          }
        }

        // Draw text
        if (pCD->nmcd.uItemState & CDIS_SELECTED || pCD->nmcd.uItemState & CDIS_HOT) {
          wstring text = MAL.TranslateNumber(eps_buffer ? eps_buffer : eps_watched) + L"/" + 
            MAL.TranslateNumber(eps_total) + L" episodes";
          hdc.EditFont(NULL, 7);
          hdc.SetBkMode(TRANSPARENT);
          hdc.SetTextColor(RGB(0, 0, 0)); // TODO: Color should be set in theme data
          hdc.DrawText(text.c_str(), text.length(), rcText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
          DeleteObject(hdc.DetachFont());
        }

        // Return
        hdc.DetachDC();
        return CDRF_DODEFAULT;
      }
    }

    default: {
      return CDRF_DODEFAULT;
    }
  }
}

// =============================================================================

/* Button control */

LRESULT CMainWindow::OnButtonCustomDraw(LPARAM lParam) {
  LPNMCUSTOMDRAW pCD = reinterpret_cast<LPNMCUSTOMDRAW>(lParam);

  switch (pCD->dwDrawStage) {
    case CDDS_PREPAINT: {
      return CDRF_NOTIFYPOSTPAINT;
    }
    case CDDS_POSTPAINT: {
      CDC dc = pCD->hdc;
      dc.FillRect(pCD->rc, ::GetSysColor(COLOR_WINDOW));
      UI.ImgList16.Draw(Icon16_Cross, dc.Get(), 0, 0);
      dc.DetachDC();
      return CDRF_SKIPDEFAULT;
    }
  }

  return 0;
}

/* Edit control */

LRESULT CMainWindow::CEditSearch::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_COMMAND: {
      if (HIWORD(wParam) == BN_CLICKED) {
        // Clear search text
        if (LOWORD(wParam) == IDC_BUTTON_CANCELSEARCH) {
          MainWindow.m_EditSearch.SetText(L"");
          return TRUE;
        }
      }
      break;
    }
  }
  
  return WindowProcDefault(hwnd, uMsg, wParam, lParam);
}

// =============================================================================

/* Tab control */

LRESULT CMainWindow::OnTabNotify(LPARAM lParam) {
  switch (reinterpret_cast<LPNMHDR>(lParam)->code) {
    // Tab select
    case TCN_SELCHANGE: {
      int index = static_cast<int>(m_Tab.GetItemParam(m_Tab.GetCurrentlySelected()));
      RefreshList(index);
      RefreshTabs(index, false);
      break;
    }
  }

  return 0;
}

// =============================================================================

/* Toolbar */

BOOL CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
  // Toolbar
  switch (LOWORD(wParam)) {
    // Login
    case 100:
      ExecuteAction(L"LoginLogout");
      return TRUE;
    // Synchronize
    case 101:
      ExecuteAction(L"Synchronize");
      return TRUE;
    // MyAnimeList
    case 102:
      ExecuteAction(L"ViewPanel");
      return TRUE;
    // Torrents
    case 106:
      ExecuteAction(L"Torrents");
      return TRUE;
    // Filter
    case 108:
      ExecuteAction(L"Filter");
      return TRUE;
    // Settings
    case 109:
      ExecuteAction(L"Settings");
      return TRUE;
    // About
    case 111:
      #ifdef _DEBUG
      DebugTest();
      #else
      ExecuteAction(L"About");
      #endif
      return TRUE;
  }

  // Menu
  if (wParam != 0 && lParam == 0) {
    wstring* str = reinterpret_cast<wstring*>(wParam);
    ExecuteAction(*str);
    RefreshMenubar();
    return TRUE;
  }
  
  // Search text
  if (HIWORD(wParam) == EN_CHANGE) {
    if (LOWORD(wParam) == IDC_EDIT_SEARCH) {
      wstring text;
      m_EditSearch.GetText(text);
      m_CancelSearch.Show(text.empty() ? SW_HIDE : SW_SHOWNORMAL);
      switch (GetSearchMode()) {
        case SEARCH_MODE_LIST:
          AnimeList.Filter.Text = text;
          RefreshList(text.empty() ? -1 : 0);
          return TRUE;
      }
    }
  }

  return FALSE;
}

LRESULT CMainWindow::OnToolbarNotify(LPARAM lParam) {
  switch (reinterpret_cast<LPNMHDR>(lParam)->code) {
    // Dropdown button click
    case TBN_DROPDOWN: {
      RECT rect; LPNMTOOLBAR nmt = reinterpret_cast<LPNMTOOLBAR>(lParam);
      ::SendMessage(nmt->hdr.hwndFrom, TB_GETRECT, static_cast<WPARAM>(nmt->iItem), reinterpret_cast<LPARAM>(&rect));          
      MapWindowPoints(nmt->hdr.hwndFrom, HWND_DESKTOP, reinterpret_cast<LPPOINT>(&rect), 2);
      wstring action;
      switch (LOWORD(nmt->iItem)) {
        // Folders
        case 104:
          action = UI.Menus.Show(m_hWindow, rect.left, rect.bottom, L"Folders");
          break;
        // Tools
        case 105:
          action = UI.Menus.Show(m_hWindow, rect.left, rect.bottom, L"Tools");
          break;
        // Search
        case 200:
          action = UI.Menus.Show(m_hWindow, rect.left, rect.bottom, L"SearchBar");
          break;
      }
      if (!action.empty()) {
        ExecuteAction(action);
        RefreshMenubar(AnimeList.Index);
      }
      break;
    }

    // Show tooltips
    case TBN_GETINFOTIP: {
      NMTBGETINFOTIP* git = reinterpret_cast<NMTBGETINFOTIP*>(lParam);
      git->cchTextMax = INFOTIPSIZE;
      git->pszText = (LPWSTR)(m_Toolbar.GetButtonTooltip(git->lParam));
      break;
    }
  }

  return 0L;
}