# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. Example variables are:
#   CPACK_GENERATOR                     - Generator used to create package
#   CPACK_INSTALL_CMAKE_PROJECTS        - For each project (path, name, component)
#   CPACK_CMAKE_GENERATOR               - CMake Generator used for the projects
#   CPACK_INSTALL_COMMANDS              - Extra commands to install components
#   CPACK_INSTALLED_DIRECTORIES           - Extra directories to install
#   CPACK_PACKAGE_DESCRIPTION_FILE      - Description file for the package
#   CPACK_PACKAGE_DESCRIPTION_SUMMARY   - Summary of the package
#   CPACK_PACKAGE_EXECUTABLES           - List of pairs of executables and labels
#   CPACK_PACKAGE_FILE_NAME             - Name of the package generated
#   CPACK_PACKAGE_ICON                  - Icon used for the package
#   CPACK_PACKAGE_INSTALL_DIRECTORY     - Name of directory for the installer
#   CPACK_PACKAGE_NAME                  - Package project name
#   CPACK_PACKAGE_VENDOR                - Package project vendor
#   CPACK_PACKAGE_VERSION               - Package project version
#   CPACK_PACKAGE_VERSION_MAJOR         - Package project version (major)
#   CPACK_PACKAGE_VERSION_MINOR         - Package project version (minor)
#   CPACK_PACKAGE_VERSION_PATCH         - Package project version (patch)

# There are certain generator specific ones

# NSIS Generator:
#   CPACK_PACKAGE_INSTALL_REGISTRY_KEY  - Name of the registry key for the installer
#   CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS - Extra commands used during uninstall
#   CPACK_NSIS_EXTRA_INSTALL_COMMANDS   - Extra commands used during install


## Debian package generator using CPack

SET(CUR_SOURCE_DIR "/storage/projects/alpr/src")

INCLUDE (InstallRequiredSystemLibraries)

SET (CPACK_SET_DESTDIR "on")
#SET (CPACK_PACKAGING_INSTALL_PREFIX "/usr")
#SET (CPACK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

SET (CPACK_GENERATOR "DEB;TGZ;RPM")

SET (CPACK_PACKAGE_NAME "openalpr")

SET(CPACK_PACKAGE_VERSION "1.1.1")

SET(CPACK_INSTALL_CMAKE_PROJECTS "${CUR_SOURCE_DIR}/build;src;ALL;/")
SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")

SET(CPACK_STRIP_FILES "1")
#SET (${VERSION} CPACK_DEBIAN_PACKAGE_VERSION)
SET (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET (CPACK_DEBIAN_PACKAGE_SECTION "graphics")
SET (CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
SET (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1.3), libgcc1 (>= 4.1.1), libtesseract3 (>= 3.0.3), libopencv-core2.4 (>= 2.4.8), libopencv-objdetect2.4 (>= 2.4.8), libopencv-highgui2.4 (>= 2.4.8), libopencv-imgproc2.4 (>= 2.4.8), libopencv-flann2.4 (>= 2.4.8), libopencv-features2d2.4 (>= 2.4.8)")

SET (CPACK_RESOURCE_FILE_LICENSE "${CUR_SOURCE_DIR}/../LICENSE")
SET (CPACK_PACKAGE_DESCRIPTION "OpenALPR - Open Source Automatic License Plate Recognition")
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenALPR is an open source Automated License Plate Recognition library written in C++.  The library analyzes images and identifies license plates. The output is the text representation of any license plate characters found in the image.  Check out a live online demo here: http://www.openalpr.com/demo.html" )
SET (CPACK_PACKAGE_CONTACT "Matt Hill <matt@ndu.com>")
SET (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_ARCHITECTURE}")

SET (CPACK_COMPONENTS_ALL Libraries ApplicationData)
#INCLUDES (CPack Documentation)
