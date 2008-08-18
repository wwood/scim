%define version 0.9.4
%define release 1

Name:           scim-qtimm
Summary:        SCIM context plugin for qt-immodule
Version:        %{version}
Release:        %{release}
Group:		System/Internationalization
License:	GPL
URL:		http://freedesktop.org/bin/view/Software/ScimQtImm
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root
Requires:	scim qt3-devel
BuildRequires:	qt3-devel
BuildRequires:	scim-devel

%description
SCIM context plugin for qt-immodule


%prep
%setup -q

%build
./configure
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%find_lang %{name}

%clean
rm -rf $RPM_BUILD_ROOT


%files -f %{name}.lang
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog NEWS README
%{_libdir}/qt3/plugins/inputmethods/*


%changelog
* Thu Jul 17 2004 LiuCougar <liuspider@users.sourceforge.net> 0.5-0.r10.1
- update the version

* Thu Jul 15 2004 UTUMI Hirosi <utuhiro78@yahoo.co.jp> 0.0.1-0.r03.3
- spec for official scim-qtimm

* Mon Jul 12 2004 UTUMI Hirosi <utuhiro78@yahoo.co.jp> 0.0.1-0.r03.2ut
- fix URL
- remove Conflicts

* Tue Jul 06 2004 Thierry Vignaud <tvignaud@mandrakesoft.com> 0.0.1-0.r03.1mdk
- fix buildrequires
- initial spec for mdk (UTUMI Hirosi <utuhiro78@yahoo.co.jp>)
