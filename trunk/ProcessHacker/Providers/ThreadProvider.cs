﻿/*
 * Process Hacker
 * 
 * Copyright (C) 2008 wj32
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;

namespace ProcessHacker
{
    public struct ThreadItem
    {
        public int TID;
        public ProcessThread Thread;

        public string CPUTime;
        public string Priority;
        public int StartAddressI;
        public string StartAddress;
        public string WaitReason;
    }

    public class ThreadProvider : Provider<int, ThreadItem>
    {
        private SymbolProvider _symbols = new SymbolProvider();
        private int _pid;

        public ThreadProvider(int PID)
            : base()
        {
            _pid = PID;

            this.ProviderUpdate += new ProviderUpdateOnce(UpdateOnce);

            // start loading symbols
            Thread t = new Thread(new ThreadStart(delegate
            {
                try
                {
                    foreach (ProcessModule module in Process.GetProcessById(_pid).Modules)
                    {
                        try
                        {
                            _symbols.LoadSymbolsFromLibrary(module.FileName, module.BaseAddress.ToInt32());
                        }
                        catch
                        { }
                    }
                }
                catch
                { }
            }));

            t.Priority = ThreadPriority.Lowest;
            t.Start();
        }

        private void UpdateOnce()
        {
            Process process = Process.GetProcessById(_pid);
            ProcessThreadCollection threads = process.Threads;
            Dictionary<int, ProcessThread> tids = new Dictionary<int, ProcessThread>();
            Dictionary<int, ThreadItem> newdictionary = new Dictionary<int, ThreadItem>(this.Dictionary);

            foreach (ProcessThread t in threads)
                tids.Add(t.Id, t);

            // look for dead threads
            foreach (int tid in Dictionary.Keys)
            {
                if (!tids.ContainsKey(tid))
                {
                    this.CallDictionaryRemoved(this.Dictionary[tid]);
                    newdictionary.Remove(tid);
                }
            }

            // look for new threads
            foreach (int tid in tids.Keys)
            {
                ProcessThread t = tids[tid];

                if (!Dictionary.ContainsKey(tid))
                {
                    ThreadItem item = new ThreadItem();

                    item.TID = t.Id;
                    item.Thread = t;

                    try { item.CPUTime = Misc.GetNiceTimeSpan(t.TotalProcessorTime); }
                    catch { }

                    try { item.Priority = t.PriorityLevel.ToString(); }
                    catch { }

                    try
                    {
                        using (Win32.ThreadHandle handle =
                            new Win32.ThreadHandle(tid, Win32.THREAD_RIGHTS.THREAD_QUERY_INFORMATION))
                        {
                            int retLen;

                            if (Win32.ZwQueryInformationThread(handle.Handle,
                                Win32.THREAD_INFORMATION_CLASS.ThreadQuerySetWin32StartAddress,
                                out item.StartAddressI, 4, out retLen) != 0)
                                throw new Exception();
                        }

                        item.StartAddress = _symbols.GetNameFromAddress(item.StartAddressI);
                    }
                    catch { }

                    try { item.WaitReason = t.WaitReason.ToString(); }
                    catch { }

                    newdictionary.Add(t.Id, item);
                    this.CallDictionaryAdded(item);
                }
                // look for modified threads
                else
                {
                    ThreadItem item = Dictionary[t.Id];
                    ThreadItem newitem = item;

                    try { newitem.CPUTime = Misc.GetNiceTimeSpan(t.TotalProcessorTime); }
                    catch { }

                    try { newitem.Priority = t.PriorityLevel.ToString(); }
                    catch { }

                    try
                    {
                        SymbolProvider.FoundLevel level;

                        string symName = _symbols.GetNameFromAddress(newitem.StartAddressI, out level);

                        if (level != SymbolProvider.FoundLevel.Address)
                            newitem.StartAddress = symName;
                    }
                    catch { }

                    try { newitem.WaitReason = t.WaitReason.ToString(); }
                    catch { }

                    if (
                        newitem.CPUTime != item.CPUTime ||
                        newitem.Priority != item.Priority || 
                        newitem.StartAddress != item.StartAddress ||
                        newitem.WaitReason != item.WaitReason
                        )
                    {
                        newdictionary[t.Id] = newitem;
                        this.CallDictionaryModified(item, newitem);
                    }
                }
            }

            Dictionary = newdictionary;
        }

        public SymbolProvider Symbols
        {
            get { return _symbols; }
        }

        public int PID
        {
            get { return _pid; }
        }
    }
}
