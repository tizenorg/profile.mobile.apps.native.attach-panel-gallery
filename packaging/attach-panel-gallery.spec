%define _optdir	/opt/usr
%define _usrdir	/usr
%define _appdir	%{_usrdir}/apps
%define _appdatadir	%{_optdir}/apps
%define _ugdir	%{_usrdir}/ug

%define _datadir /opt%{_ugdir}/data
%define _sharedir /opt/usr/media/.iv
Name:       attach-panel-gallery
Summary:    attach-panel-gallery UX
Version:    1.3.21
Release:    1
Group:      Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable" || "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  cmake
BuildRequires:  gettext-tools
BuildRequires:  edje-tools
BuildRequires:  prelink
BuildRequires:  libicu-devel

BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(storage)

%description
Description: attach-panel-gallery UG

%prep
%setup -q

%build

%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif

%ifarch %{arm}
CXXFLAGS+=" -D_ARCH_ARM_ -mfpu=neon"
%endif

cmake . -DCMAKE_INSTALL_PREFIX=%{_ugdir}/ug-attach-panel-gallery  -DCMAKE_DATA_DIR=%{_datadir} -DARCH=%{ARCH}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
if [ ! -d %{buildroot}/opt/usr/apps/attach-panel-gallery/data ]
then
        mkdir -p %{buildroot}/opt/usr/apps/attach-panel-gallery/data
fi

%make_install

mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}%{_sharedir}
cp LICENSE %{buildroot}/usr/share/license/attach-panel-gallery


%post
mkdir -p /usr/ug/bin/
ln -sf /usr/bin/ug-client /usr/ug/bin/attach-panel-gallery
%postun

%files
%manifest attach-panel-gallery.manifest
%defattr(-,root,root,-)
%{_ugdir}/lib/libug-attach-panel-gallery.so*
%{_ugdir}/res/edje/attach-panel-gallery/*
%{_ugdir}/res/images/attach-panel-gallery/*
%{_ugdir}/res/locale/*/*/attach-panel-gallery.mo
/usr/share/packages/attach-panel-gallery.xml
/usr/ug/res/images/attach-panel-gallery/attach-panel-gallery.png
/usr/share/license/attach-panel-gallery

