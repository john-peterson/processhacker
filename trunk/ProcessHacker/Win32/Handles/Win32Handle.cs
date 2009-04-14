﻿/*
 * Process Hacker - 
 *   windows handle
 * 
 * Copyright (C) 2008-2009 wj32
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

using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace ProcessHacker
{
    public partial class Win32
    {
        /// <summary>
        /// Represents a generic Windows handle.
        /// </summary>
        public class Win32Handle : IDisposable
        {
            private object _disposeLock = new object();
            private bool _owned = true;
            private bool _disposed = false;
            private int _handle;

            public static implicit operator int(Win32Handle handle)
            {
                return handle.Handle;
            }

            public static implicit operator IntPtr(Win32Handle handle)
            {
                return new IntPtr(handle.Handle);
            }

            /// <summary>
            /// Creates a new, invalid handle. You must set the handle using the Handle property.
            /// </summary>
            protected Win32Handle()
            { }

            /// <summary>
            /// Creates a new handle using the specified value. The handle will be closed when 
            /// this object is disposed or garbage-collected.
            /// </summary>
            /// <param name="handle">The handle value.</param>
            public Win32Handle(int handle)
            {
                _handle = handle;
            }

            /// <summary>
            /// Creates a new handle using the specified value. If owned is set to false, the 
            /// handle will not be closed automatically.
            /// </summary>
            /// <param name="handle">The handle value.</param>
            /// <param name="owned">Specifies whether the handle will be closed automatically.</param>
            public Win32Handle(int handle, bool owned)
            {
                _handle = handle;
                _owned = owned;
            }

            /// <summary>
            /// Gets whether this handle is closed.
            /// </summary>
            public bool Disposed
            {
                get { return _disposed; }
            }

            /// <summary>
            /// Gets whether the handle will be automatically closed.
            /// </summary>
            public bool Owned
            {
                get { return _owned; }
            }

            /// <summary>
            /// Gets the handle value.
            /// </summary>
            public int Handle
            {
                get { return _handle; }
                protected set { _handle = value; }
            }

            /// <summary>
            /// Gets certain information about the handle.
            /// </summary>
            /// <returns>A HANDLE_FLAGS value.</returns>
            public HANDLE_FLAGS GetHandleInformation()
            {
                HANDLE_FLAGS flags;

                if (!Win32.GetHandleInformation(this, out flags))
                    ThrowLastWin32Error();

                return flags;
            }

            /// <summary>
            /// Gets the handle's name.
            /// </summary>
            /// <returns>A string.</returns>
            public string GetHandleName()
            {
                int retLength;

                ZwQueryObject(this, OBJECT_INFORMATION_CLASS.ObjectNameInformation,
                      IntPtr.Zero, 0, out retLength);

                if (retLength > 0)
                {
                    using (MemoryAlloc oniMem = new MemoryAlloc(retLength))
                    {
                        if (ZwQueryObject(this, OBJECT_INFORMATION_CLASS.ObjectNameInformation,
                            oniMem.Memory, oniMem.Size, out retLength) != 0)
                            ThrowLastWin32Error();

                        OBJECT_NAME_INFORMATION oni = oniMem.ReadStruct<OBJECT_NAME_INFORMATION>();

                        return ReadUnicodeString(oni.Name);
                    }
                }
                else
                {
                    ThrowLastWin32Error();
                }

                return null;
            }

            /// <summary>
            /// Sets certain information about the handle.
            /// </summary>
            /// <param name="mask">Specifies which flags to set.</param>
            /// <param name="flags">The values of the flags to set.</param>
            public void SetHandleInformation(HANDLE_FLAGS mask, HANDLE_FLAGS flags)
            {
                if (!Win32.SetHandleInformation(this, mask, flags))
                    ThrowLastWin32Error();
            }

            /// <summary>
            /// Closes the handle. This method must not be called directly; instead, 
            /// override this method in a derived class if your handle must be closed 
            /// with a method other than CloseHandle.
            /// </summary>
            protected virtual void Close()
            {
                CloseHandle(_handle);
            }

            ~Win32Handle()
            {
                this.Dispose(false);
            }

            private void Dispose(bool disposing)
            {
                try
                {
                    if (disposing)
                        Monitor.Enter(_disposeLock);

                    if (!_disposed && _owned)
                    {
                        this.Close();
                        _disposed = true;
                    }
                }
                finally
                {
                    if (disposing)
                        Monitor.Exit(_disposeLock);
                }
            }

            /// <summary>
            /// Closes the handle.
            /// </summary>
            public void Dispose()
            {
                this.Dispose(true);
                GC.SuppressFinalize(this);
            }
        }
    }
}
