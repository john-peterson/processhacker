/*
 *  ReactOS Task Manager
 *
 *  run.c
 *
 *  Copyright (C) 1999 - 2001  Brian Palmer  <brianp@reactos.org>
 *                2005         Klemens Friedl <frik85@reactos.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "taskmgr.h"

VOID TaskManager_OnFileNew()
{
    HICON hIcon = NULL;
    RUNFILEDLG  RunFileDlg = NULL;
    WCHAR szTitle[40];
    WCHAR szText[256];

    /* Load language strings from resource file */
    LoadString(hInst, IDS_CREATENEWTASK, szTitle, _countof(szTitle));
    LoadString(hInst, IDS_CREATENEWTASK_DESC, szText, _countof(szText));

    // Load application icon.
    hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TASKMANAGER));

#pragma warning(disable:4306)
    // Load RunFileDlg export using it's ordinal.
    RunFileDlg = (RUNFILEDLG)(FARPROC)GetProcAddress(GetModuleHandle(TEXT("SHELL32.dll")), (LPCSTR)0x3D);
#pragma warning(default:4306)

    /* Show "Run..." dialog */
    if (RunFileDlg)
    {
        /* NOTE - don't check whether running on win 9x or NT, let's just
                  assume that a unicode build only runs on NT */
        RunFileDlg(hMainWnd, hIcon, NULL, NULL, szText, RFF_CALCDIRECTORY);
    }

    DestroyIcon(hIcon);
}


