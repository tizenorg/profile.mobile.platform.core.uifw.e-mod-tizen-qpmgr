%bcond_with x
%bcond_with wayland

Name:       e-mod-tizen-qpmgr
Summary:    The Enlightenment Quickpanel Manager 
Version:    0.1.12
Release:    1
Group:      Graphics & UI Framework/Other
License:    BSD-2-Clause
Source0:    %{name}-%{version}.tar.bz2
BuildRequires: pkgconfig(enlightenment)
%if %{with x}
BuildRequires: pkgconfig(x11) 
%endif
%if %{with wayland}
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(edje)
%endif

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
mkdir -p %{buildroot}/usr/share/license
cp -a %{_builddir}/%{buildsubdir}/COPYING %{buildroot}/usr/share/license/%{name}

# install
%make_install

%files
%defattr(-,root,root,-)
%{_libdir}/enlightenment/modules/e-mod-tizen-qpmgr
/usr/share/license/%{name}

%define _unpackaged_files_terminate_build 0
