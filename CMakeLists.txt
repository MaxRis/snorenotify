project( SnoreNotify )
cmake_minimum_required( VERSION 2.8.12 )

include(FeatureSummary)

find_package(ECM 1.7.0 REQUIRED NO_MODULE)
set_package_properties(ECM      PROPERTIES
                                DESCRIPTION "Extra modules and scripts for CMake."
                                URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules"
                                TYPE REQUIRED)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)

include(GenerateExportHeader)
include(CMakeDependentOption)

include(ECMAddAppIcon)
include(ECMGeneratePriFile)
include(ECMInstallIcons)
include(ECMMarkNonGuiExecutable)
include(ECMOptionalAddSubdirectory)
include(ECMPackageConfigHelpers)
include(ECMPoQmTools)
include(ECMSetupVersion)

include(QtStatic)


set(SNORE_VERSION_MAJOR 0)
set(SNORE_VERSION_MINOR 7)
set(SNORE_VERSION_PATCH 1)

set(SNORE_SUFFIX "-qt5")
set(SNORE_CamelCase_SUFFIX "Qt5")


find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets QUIET)
find_package(Qt5Quick QUIET)

set_package_properties(Qt5Quick PROPERTIES
                                PURPOSE "Support builtin notifiaction backend."
                                TYPE OPTIONAL)

set_package_properties(Qt5Widgets PROPERTIES
                                  PURPOSE "Supprot for the daemon and the settings dialog as well as some backends."
                                  TYPE OPTIONAL)


option(SNORE_STATIC "Build a static snore" OFF)
option(SNORE_STATIC_QT "Build a static snore" OFF)

set(LIBSNORE_INCLUDE_DIR ${KDE_INSTALL_INCLUDEDIR}/libsnore)

set(LIBSNORE_PLUGIN_PATH ${KDE_INSTALL_PLUGINDIR}/libsnore${SNORE_SUFFIX})
set(SNORE_PLUGIN_INSTALL_PATH LIBRARY DESTINATION ${LIBSNORE_PLUGIN_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

add_subdirectory(data)
add_subdirectory(src)

if (NOT SNORE_STATIC)
    add_subdirectory(autotest)
endif()

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()


generate_snore_plugin_header()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES)
