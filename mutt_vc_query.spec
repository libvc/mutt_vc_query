# RPM spec file for Red Hat Linux
# $Id: mutt_vc_query.spec,v 1.2 2003/05/19 00:49:28 ahsu Exp $
Summary: A vCard query utility for mutt.
Name: mutt_vc_query
Version: 002
Release: 1
Source: http://osdn.dl.sourceforge.net/sourceforge/rolo/%{name}-%{version}.tar.gz
License: GPL
URL: http://rolo.sf.net/
Packager: Andrew Hsu <ahsu@users.sf.net>
Distribution: Red Hat Linux
Vendor: Andrew Hsu
Group: Applications/Productivity
BuildRoot: %{_tmppath}/%{name}-buildroot
BuildRequires: libvc
Prefix: /usr

%description
This utility is intended to be used with mutt(1) and rolo(1). This
utility will enable mutt users to query email addresses that have
been stored by rolo.

%prep
%setup -q

%build
./configure --prefix=/usr --mandir=/usr/share/man
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/*
%{_mandir}/man1/*
%doc AUTHORS COPYING ChangeLog INSTALL NEWS README THANKS TODO
