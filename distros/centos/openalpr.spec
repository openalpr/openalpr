Name:     openalpr
Version:  2.2.0
Release:  1%{?dist}
Summary:  OpenALPR
License:  AGPLv3
URL:      https://github.com/openalpr/openalpr  
source:   openalpr.tar.gz
requires: tesseract, log4cplus
 
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

%changelog
* Sun Feb 28 2016 mhill
  First spec release


%prep

# Extract the Source0 tar
%setup -n openalpr/src


%build

# Manually set all paths
cmake -DCMAKE_INSTALL_PREFIX=/usr .

make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


mv $RPM_BUILD_ROOT/usr/lib/python2.7/dist-packages $RPM_BUILD_ROOT/usr/lib/python2.7/site-packages 



%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
   
  /etc/openalpr/openalpr.conf 
  /usr/bin/alpr
  /usr/share/man/man1/alpr.1.gz


   %exclude /usr/lib/python2.7/site-packages/openalpr/*.pyc
   %exclude /usr/lib/python2.7/site-packages/openalpr/*.pyo
   %exclude /usr/lib/pkgconfig/openalpr.pc
   %exclude /usr/include/openalprgo.h
   %exclude /usr/include/state_detector.h


%files utils
%defattr(-,root,root,-)
  /usr/bin/openalpr-utils-*

%files daemon
%defattr(-,root,root,-)
  /etc/openalpr/alprd.conf
  /usr/bin/alprd

%files -n libopenalpr-devel
%defattr(-,root,root,-)
  /usr/include/alpr.h
  /usr/lib/libopenalpr-static.a

%files -n libopenalpr2-data
%defattr(-,root,root,-)
  /usr/share/openalpr/runtime_data/

%files -n libopenalpr2
%defattr(-,root,root,-)
  /usr/lib/*.so*

%files -n python-openalpr
%defattr(-,root,root,-)
  /usr/lib/python2.7/site-packages/openalpr/*.py
