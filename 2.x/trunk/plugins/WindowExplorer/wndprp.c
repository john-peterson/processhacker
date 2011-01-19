/*
 * Process Hacker Window Explorer - 
 *   window properties
 * 
 * Copyright (C) 2010 wj32
 * 
 * This file is part of Process Hacker.
 * 
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "wndexp.h"
#include "resource.h"

typedef struct _WINDOW_PROPERTIES_CONTEXT
{
    HWND WindowHandle;
} WINDOW_PROPERTIES_CONTEXT, *PWINDOW_PROPERTIES_CONTEXT;

VOID PhShowWindowProperties(
    __in HWND ParentWindowHandle,
    __in HWND WindowHandle
    )
{
    //HWND hwnd;
    PWINDOW_PROPERTIES_CONTEXT context;

    context = PhAllocate(sizeof(WINDOW_PROPERTIES_CONTEXT));
    memset(context, 0, sizeof(WINDOW_PROPERTIES_CONTEXT));
    context->WindowHandle = WindowHandle;

    NOTHING;
}
