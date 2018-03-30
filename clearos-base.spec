Name: clearos-base
Version: 7.4.0
Release: 1%{dist}
Summary: ClearOS system base security driver.
License: GPLv3 or later
Group: Applications/System
Source: %{name}-%{version}.tar.gz
Requires: clearos-release >= 7
Provides: system-base-security

%description
ClearOS system base security driver.

%prep
%setup -q
%build

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p -m 755 $RPM_BUILD_ROOT/etc/logrotate.d
mkdir -p -m 755 $RPM_BUILD_ROOT/etc/security/limits.d

install -m 644 etc/logrotate.d/compliance $RPM_BUILD_ROOT/etc/logrotate.d/
install -m 755 etc/security/limits.d/95-clearos.conf $RPM_BUILD_ROOT/etc/security/limits.d/

#------------------------------------------------------------------------------
# I N S T A L L  S C R I P T
#------------------------------------------------------------------------------

%post
logger -p local6.notice -t installer "clearos-base - installing"

# Syslog customizations
#----------------------

if [ -z "`grep ^local5 /etc/rsyslog.conf`" ]; then
    logger -p local5.notice -t installer "clearos-base - adding compliance log file to rsyslog"
    echo "local5.*  /var/log/compliance" >> /etc/rsyslog.conf
    sed -i -e 's/[[:space:]]*\/var\/log\/messages/;local5.none \/var\/log\/messages/' /etc/rsyslog.conf
    /sbin/service rsyslog restart >/dev/null 2>&1
fi

# Disable SELinux
#----------------

if [ -d /etc/selinux ]; then
    CHECK=`grep ^SELINUX= /etc/selinux/config 2>/dev/null | sed 's/.*=//'`
    if [ -z "$CHECK" ]; then
        logger -p local6.notice -t installer "clearos-base - disabling SELinux with new configuration"
        echo "SELINUX=disabled" >> /etc/selinux/config
    elif [ "$CHECK" != "disabled" ]; then
        logger -p local6.notice -t installer "clearos-base - disabling SELinux"
        sed -i -e 's/^SELINUX=.*/SELINUX=disabled/' /etc/selinux/config
    fi
fi

# Enable audit by default
#------------------------

if [ $1 -eq 1 ]; then
    logger -p local6.notice -t installer "clearos-base - enabling audit on boot"
    /sbin/chkconfig auditd on >/dev/null 2>&1
fi

exit 0

%preun
if [ $1 -eq 0 ]; then
    logger -p local6.notice -t installer "clearos-base - uninstalling"
fi

%files
%defattr(-,root,root)
/etc/logrotate.d/compliance
/etc/security/limits.d/95-clearos.conf

%changelog
* Fri Mar 30 2018 ClearFoundation <developer@clearfoundation.com> - 7.4.0-1
- Split out security components into system-base-security

* Tue Oct 31 2017 ClearFoundation <developer@clearfoundation.com> - 7.0.2-1
- Added bin directory and PATH change

* Tue Aug 12 2014 ClearFoundation <developer@clearfoundation.com> - 7.0.0-1
- Updated RPM list for ClearOS 7
- Removed functions-automagic

* Thu Jun 26 2014 ClearFoundation <developer@clearfoundation.com> - 6.6.0-1
- Changed app-passwd to perform PAM authentication

* Thu May 31 2012 ClearFoundation <developer@clearfoundation.com> - 6.2.2-1
- Fixed password check space issue (tracker #628)
- Updated audit policies

* Fri Jan 27 2012 ClearFoundation <developer@clearfoundation.com> - 6.2.1-1
- Removed experimental postinstall script
- Removed deprecated perl functions references
- Cleaned up spec file

* Wed Nov 23 2011 ClearFoundation <developer@clearfoundation.com> - 6.1.0.beta2-1
- Started changelog
