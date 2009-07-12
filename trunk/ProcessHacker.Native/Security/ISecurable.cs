﻿using System;
using System.Collections.Generic;
using System.Text;
using ProcessHacker.Native.Api;
using ProcessHacker.Native.Security.AccessControl;

namespace ProcessHacker.Native.Security
{
    public interface ISecurable
    {
        SecurityDescriptor GetSecurity();
        void SetSecurity(SecurityDescriptor securityDescriptor);
        void SetSecurity(SecurityInformation securityInformation, SecurityDescriptor securityDescriptor);
    }
}
