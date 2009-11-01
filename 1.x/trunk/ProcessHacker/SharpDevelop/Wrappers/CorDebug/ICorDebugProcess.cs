// <file>
//     <copyright see="prj:///doc/copyright.txt"/>
//     <license see="prj:///doc/license.txt"/>
//     <owner name="David Srbeck�" email="dsrbecky@gmail.com"/>
//     <version>$Revision: 2921 $</version>
// </file>

#pragma warning disable 1591

namespace Debugger.Wrappers.CorDebug
{
	using System;

	public partial class ICorDebugProcess
	{
		public bool HasQueuedCallbacks()
		{
			int pbQueued;
			this.WrappedObject.HasQueuedCallbacks(null, out pbQueued);
			return pbQueued != 0;
		}
	}
}
