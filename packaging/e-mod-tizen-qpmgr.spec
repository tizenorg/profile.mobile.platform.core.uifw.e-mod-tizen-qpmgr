%bcond_with x
%bcond_with wayland

Name:       e-mod-tizen-qpmgr
Summary:    The Enlightenment Quickpanel Manager 
Version:    0.1.13
Release:    1
Group:      Graphics & UI Framework/Other
License:    BSD-2-Clause
Source0:    %{name}-%{version}.tar.bz2

%if "%{profile}" == "wearable" || "%{profile}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires: pkgconfig(enlightenment)
%if %{with x}
BuildRequires: pkgconfig(x11) 
%endif
%if %{with wayland}
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(edje)
%endif

%global TZ_SYS_RO_SHARE  %{?TZ_SYS_RO_SHARE:%TZ_SYS_RO_SHARE}%{!?TZ_SYS_RO_SHARE:/usr/share}

%description
The Enlightenment Quickpanel Manager 

%prep
%setup -q -n %{name}-%{version}

%build
%autogen
%if %{with wayland}
%configure \
      --enable-wayland-only
%else
%configure
%endif
make %{?_smp_mflags}

%install
# for license notification
mkdir -p %{buildroot}/%{TZ_SYS_RO_SHARE}/license
cp -a %{_builddir}/%{buildsubdir}/COPYING %{buildroot}/%{TZ_SYS_RO_SHARE}/license/%{name}

# install
%make_install

%files
%defattr(-,root,root,-)
%{_libdir}/enlightenment/modules/e-mod-tizen-qpmgr
%{TZ_SYS_RO_SHARE}/license/%{name}

%define _unpackaged_files_terminate_build 0
