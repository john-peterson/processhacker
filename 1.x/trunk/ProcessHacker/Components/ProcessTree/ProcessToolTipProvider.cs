﻿/*
 * Process Hacker - 
 *   IToolTipProvider implementation for the process tree
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
using System.Collections.Generic;
using System.Diagnostics;
using Aga.Controls.Tree;
using Microsoft.Win32;
using ProcessHacker.Common;
using ProcessHacker.Native;
using ProcessHacker.Native.Api;

namespace ProcessHacker
{
    public class ProcessToolTipProvider : IToolTipProvider
    {
        private readonly ProcessTree _tree;

        public ProcessToolTipProvider(ProcessTree owner)
        {
            _tree = owner;
        }

        public string GetToolTip(TreeNodeAdv node, Aga.Controls.Tree.NodeControls.NodeControl nodeControl)
        {
            var pNode = _tree.FindNode(node);

            // Use the process node's tooltip mechanism to allow caching.
            if (pNode != null)
                return pNode.GetTooltipText(this);
            
            return string.Empty;
        }

        public string GetToolTip(ProcessNode pNode)
        {
            try
            {
                string cmdText = (!string.IsNullOrEmpty(pNode.ProcessItem.CmdLine) ?
                        (Utils.CreateEllipsis(pNode.ProcessItem.CmdLine.Replace("\0", string.Empty), 100) + "\n") : string.Empty);

                string fileText = string.Empty;

                try
                {
                    if (pNode.ProcessItem.VersionInfo != null)
                    {
                        var info = pNode.ProcessItem.VersionInfo;

                        fileText = "File:\n" + PhUtils.FormatFileInfo(info.FileName, info.FileDescription, info.CompanyName, info.FileVersion, 4);
                    }
                }
                catch
                {
                    if (!string.IsNullOrEmpty(pNode.ProcessItem.FileName))
                        fileText = "File:\n    " + pNode.ProcessItem.FileName;
                }

                string runDllText = string.Empty;

                if (!string.IsNullOrEmpty(pNode.ProcessItem.FileName) &&
                    pNode.ProcessItem.FileName.EndsWith("\\rundll32.exe",
                    StringComparison.InvariantCultureIgnoreCase) &&
                    !string.IsNullOrEmpty(pNode.ProcessItem.CmdLine))
                {
                    try
                    {
                        // TODO: fix crappy method
                        string targetFile = pNode.ProcessItem.CmdLine.Split(new char[] { ' ' }, 2)[1].Split(',')[0];

                        // if it doesn't specify an absolute path, assume it's in system32.
                        if (!targetFile.Contains(":", StringComparison.OrdinalIgnoreCase))
                            targetFile = Environment.SystemDirectory + "\\" + targetFile;

                        FileVersionInfo info = FileVersionInfo.GetVersionInfo(targetFile);

                        runDllText = "\nRunDLL target:\n    " + info.FileName + "\n    " +
                            info.FileDescription + " " + info.FileVersion + "\n    " +
                            info.CompanyName;
                    }
                    catch (Exception ex)
                    {
                        Logging.Log(ex);
                    }
                }

                string dllhostText = string.Empty;

                if (!string.IsNullOrEmpty(pNode.ProcessItem.FileName) &&
                    pNode.ProcessItem.FileName.EndsWith("\\dllhost.exe", StringComparison.InvariantCultureIgnoreCase) &&
                    !string.IsNullOrEmpty(pNode.ProcessItem.CmdLine))
                {
                    try
                    {
                        string clsid = pNode.ProcessItem.CmdLine.ToLowerInvariant().Split(new[]
                        {
                            "/processid:"
                        }, StringSplitOptions.None)[1].Split(' ')[0];

                        using (RegistryKey key = Registry.ClassesRoot.OpenSubKey("CLSID\\" + clsid))
                        using (RegistryKey inprocServer32 = key.OpenSubKey("InprocServer32"))
                        {
                            string name = key.GetValue(string.Empty) as string;
                            string fileName = inprocServer32.GetValue(string.Empty) as string;

                            FileVersionInfo info = FileVersionInfo.GetVersionInfo(Environment.ExpandEnvironmentVariables(fileName));

                            dllhostText = "\nCOM Target:\n    " + name + " (" + clsid.ToUpper() + ")\n    " +
                                          info.FileName + "\n    " +
                                          info.FileDescription + " " + info.FileVersion + "\n    " + info.CompanyName;
                        }

                    }
                    catch (Exception ex)
                    {
                        Logging.Log(ex);
                    }
                }

                string servicesText = string.Empty;

                try
                {
                    IDictionary<int, List<string>> processServices;
                    IDictionary<string, ServiceItem> services;

                    if (!_tree.DumpMode)
                    {
                        processServices = Program.HackerWindow.ProcessServices;
                        services = Program.ServiceProvider.Dictionary;
                    }
                    else
                    {
                        processServices = _tree.DumpProcessServices;
                        services = _tree.DumpServices;
                    }

                    if (processServices.ContainsKey(pNode.Pid))
                    {
                        foreach (string service in processServices[pNode.Pid])
                        {
                            if (services.ContainsKey(service))
                            {
                                if (string.IsNullOrEmpty(services[service].Status.DisplayName))
                                    servicesText += "    " + service + " (" + services[service].Status.DisplayName + ")\n";
                                else
                                    servicesText += "    " + service + "\n";
                            }
                            else
                            {
                                servicesText += "    " + service + "\n";
                            }
                        }

                        servicesText = "\nServices:\n" + servicesText.TrimEnd('\n');
                    }
                }
                catch (Exception ex)
                {
                    Logging.Log(ex);
                }

                string otherNotes = string.Empty;

                try
                {
                    if (pNode.ProcessItem.IsPacked && pNode.ProcessItem.ImportModules > 0)
                        otherNotes += "\n    Image is probably packed - has " +
                            pNode.ProcessItem.ImportFunctions.ToString() + " imports over " +
                            pNode.ProcessItem.ImportModules.ToString() + " modules.";
                    else if (pNode.ProcessItem.IsPacked)
                        otherNotes += "\n    Image is probably packed - error reading PE file.";

                    if (pNode.ProcessItem.FileName != null)
                    {
                        if (pNode.ProcessItem.VerifyResult == VerifyResult.Trusted)
                        {
                            if (!string.IsNullOrEmpty(pNode.ProcessItem.VerifySignerName))
                                otherNotes += "\n    Signer: " + pNode.ProcessItem.VerifySignerName;
                            else
                                otherNotes += "\n    Signed.";
                        }
                        else if (pNode.ProcessItem.VerifyResult == VerifyResult.Unknown &&
                            !Settings.Instance.VerifySignatures)
                        {
                            otherNotes += string.Empty;
                        }
                        else if (pNode.ProcessItem.VerifyResult == VerifyResult.Unknown &&
                            Settings.Instance.VerifySignatures && !_tree.DumpMode)
                        {
                            otherNotes += "\n    File has not been processed yet. Please wait..."; 
                        }
                        else if (pNode.ProcessItem.VerifyResult != VerifyResult.NoSignature)
                        {
                            otherNotes += "\n    Signature invalid.";
                        }

                        if (Program.ImposterNames.Contains(pNode.Name.ToLowerInvariant()) &&
                            pNode.ProcessItem.VerifyResult != VerifyResult.Trusted &&
                            pNode.ProcessItem.VerifyResult != VerifyResult.Unknown)
                            otherNotes += "\n    Process is using the name of a known process but its signature could not be verified.";
                    }

                    if (pNode.ProcessItem.IsInJob)
                        otherNotes += "\n    Process is in a job.";
                    if (pNode.ProcessItem.ElevationType == TokenElevationType.Full)
                        otherNotes += "\n    Process is elevated.";
                    if (pNode.ProcessItem.IsDotNet)
                        otherNotes += "\n    Process is managed (.NET).";
                    if (pNode.ProcessItem.IsPosix)
                        otherNotes += "\n    Process is POSIX.";
                    if (pNode.ProcessItem.IsWow64)
                        otherNotes += "\n    Process is 32-bit (running under WOW64).";

                    if (otherNotes != string.Empty)
                        otherNotes = "\nNotes:" + otherNotes;
                }
                catch (Exception ex)
                {
                    Logging.Log(ex);
                }

                return (cmdText + fileText + otherNotes + runDllText + dllhostText + servicesText).Trim(' ', '\n', '\r');
            }
            catch (Exception ex)
            {
                Logging.Log(ex);
            }

            return string.Empty;
        }
    }
}
