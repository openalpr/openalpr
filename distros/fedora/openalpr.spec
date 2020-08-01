Name:     openalpr
Version:  2.3.0
Release:  1%{?dist}
Summary:  OpenALPR
License:  AGPLv3
URL:      https://github.com/openalpr/openalpr  
source:   openalpr.tar.gz
Requires: tesseract, log4cplus
BuildRequires: tesseract-devel, log4cplus-devel, opencv-devel
 
%package utils
Summary:  OpenALPR miscellaneous utilities
%description utils
OpenALPR miscellaneous utilities

%package daemon
Summary:  OpenALPR Daemon
%description daemon
OpenALPR daemon runs license plate recognition in the background

%package -n libopenalpr-devel
Summary:  OpenALPR Development headers
%description -n libopenalpr-devel
OpenALPR Development headers

%package -n libopenalpr2-data
Summary:  OpenALPR runtime data
%description -n libopenalpr2-data
OpenALPR runtime data

%package -n libopenalpr2
Summary:  OpenALPR libs
%description -n libopenalpr2
OpenALPR libs

%package -n python-openalpr
Summary:  OpenALPR Python bindings
%description -n python-openalpr
OpenALPR Python bindings


%description
OpenALPR is an open source Automatic License Plate Recognition library written in C++ 
with bindings in C#, Java, Node.js, Go, and Python. The library analyzes images and 
video streams to identify license plates. The output is the text representation of 
any license plate characters


%prep

# Extract the Source0 tar
%setup -n %{name}-%{version}/src


%build

# Manually set all paths
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} .

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

if [ %{buildroot}%{_prefix}/lib != %{buildroot}%{_libdir} ]; then
    mv %{buildroot}%{_prefix}/lib %{buildroot}%{_libdir} 
fi

mv %{buildroot}%{_libdir}/python2.7/dist-packages %{buildroot}%{_libdir}/python2.7/site-packages 


%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
   
  /etc/openalpr/openalpr.conf 
  /usr/bin/alpr
  /usr/share/man/man1/alpr.1.gz
  /usr/share/openalpr/config/openalpr.defaults.conf


   %exclude %{_libdir}/python2.7/site-packages/openalpr/*.pyc
   %exclude %{_libdir}/python2.7/site-packages/openalpr/*.pyo
   %exclude %{_libdir}/pkgconfig/openalpr.pc
   %exclude /usr/include/openalprgo.h


%files utils
%defattr(-,root,root,-)
  /usr/bin/openalpr-utils-*

%files daemon
%defattr(-,root,root,-)
  /etc/openalpr/alprd.conf
  /usr/bin/alprd
  /usr/share/openalpr/config/alprd.defaults.conf

%files -n libopenalpr-devel
%defattr(-,root,root,-)
  /usr/include/alpr.h
  %{_libdir}/libopenalpr-static.a

%files -n libopenalpr2-data
%defattr(-,root,root,-)
  /usr/share/openalpr/runtime_data/

%files -n libopenalpr2
%defattr(-,root,root,-)
  %{_libdir}/*.so*

%files -n python-openalpr
%defattr(-,root,root,-)
  %{_libdir}/python2.7/site-packages/openalpr/*.py


%changelog
* Fri Jan 20 2017 jceloria
  Bumped version to 2.3.0
  Replaced library path with _libdir macro

* Sun Feb 28 2016 mhill
  First spec release
