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
BuildRequires: pkgconfig(libtzplatform-config)

%description
Description: attach-panel-gallery UG

%prep
%setup -q

%build

%define _app_license_dir          %{TZ_SYS_SHARE}/license

%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif

%ifarch %{arm}
CXXFLAGS+=" -D_ARCH_ARM_ -mfpu=neon"
%endif

cmake . -DCMAKE_INSTALL_PREFIX=%{TZ_SYS_RO_UG} \
		-DARCH=%{ARCH} \
		-DTZ_SYS_RO_PACKAGES=%{TZ_SYS_RO_PACKAGES}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

%make_install

mkdir -p %{buildroot}%{_app_license_dir}
cp LICENSE %{buildroot}%{_app_license_dir}/attach-panel-gallery


%post
mkdir -p /usr/ug/bin/
ln -sf /usr/bin/ug-client %{TZ_SYS_RO_UG}/bin/attach-panel-gallery
%postun

%files
%manifest attach-panel-gallery.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_UG}/lib/libug-attach-panel-gallery.so*
%{TZ_SYS_RO_UG}/res/edje/ug-attach-panel-gallery/*
%{TZ_SYS_RO_UG}/res/images/attach-panel-gallery/*
%{TZ_SYS_RO_UG}/res/locale/*/*/attach-panel-gallery.mo
%{TZ_SYS_RO_PACKAGES}/attach-panel-gallery.xml
%{TZ_SYS_RO_UG}/res/images/attach-panel-gallery/attach-panel-gallery.png
%{TZ_SYS_SHARE}/license/attach-panel-gallery

