cmake_minimum_required(VERSION 3.0)
# cmake_minimum_required(VERSION 2.8.12)

project(CliSyntaxHighlighting VERSION 0.1.0)

# find_package(ECM 5.31.0 REQUIRED NO_MODULE)
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
# if(POLICY CMP0063)
#     cmake_policy(SET CMP0063 NEW)
# endif()

# include(FeatureSummary)
# include(GenerateExportHeader)
# include(ECMSetupVersion)
# include(ECMGenerateHeaders)
# include(CMakePackageConfigHelpers)
include(ECMPoQmTools)
# include(ECMQtDeclareLoggingCategory)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMMarkNonGuiExecutable)

#
# Dependencies
#
set(QT_MIN_VERSION "5.4.0")
# find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED COMPONENTS Core)
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core)
# find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE QUIET OPTIONAL_COMPONENTS Widgets XmlPatterns)
# set_package_properties(Qt5 PROPERTIES URL "http://qt-project.org/")
# set_package_properties(Qt5Widgets PROPERTIES PURPOSE "Example application.")
# set_package_properties(Qt5XmlPatterns PROPERTIES PURPOSE "Compile-time validation of syntax definition files.")

set(KF5_MIN_VERSION "5.28.0")
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS SyntaxHighlighting)

set(CMAKE_CXX_STANDARD 17)

include_directories(".")

option(BUILD_VT_TRACE "Build vt-trace options" OFF)

set(vt_syntax_highlighting_srcs
  vt-kate-syntax-highlighter.cpp
  vthighlighter.cpp)
set(vt_trace 0)
if(${BUILD_VT_TRACE})
  set(vt_syntax_highlighting_srcs ${vt_syntax_highlighting_srcs} vttracehighlighter.cpp)
  set(vt_trace 1)

  set (KSYNTAX_HIGHLIGHTING_ROOT_PROJECT "ksyntax-highlighting")
  add_subdirectory(${KSYNTAX_HIGHLIGHTING_ROOT_PROJECT})

  include(GenerateExportHeader)
  include(ECMSetupVersion)
  include(ECMGenerateHeaders)
  include(ECMQtDeclareLoggingCategory)

  ecm_setup_version(PROJECT
    VARIABLE_PREFIX SyntaxHighlighting
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksyntaxhighlighting_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SyntaxHighlightingConfigVersion.cmake"
  )

  set(REQUIRED_QT_VERSION 5.10.0)
  find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED COMPONENTS Core Network Test)
  find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED COMPONENTS Gui)

  set (KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT
    ${KSYNTAX_HIGHLIGHTING_ROOT_PROJECT}/src/lib)
  set (KSYNTAX_HIGHLIGHTING_PRIVATE_SRC
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/abstracthighlighter.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/context.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/contextswitch.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/definition.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/definitiondownloader.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/foldingregion.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/format.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/htmlhighlighter.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/keywordlist.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/repository.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/rule.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/state.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/syntaxhighlighter.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/theme.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/themedata.cpp
    ${KSYNTAX_HIGHLIGHTING_SRC_LIB_PROJECT}/wildcardmatcher.cpp
  )
  ecm_qt_declare_logging_category(KSYNTAX_HIGHLIGHTING_PRIVATE_SRC
    HEADER ksyntaxhighlighting_logging.h
    IDENTIFIER KSyntaxHighlighting::Log
    CATEGORY_NAME org.kde.ksyntaxhighlighting
  )
  add_library(VtSyntaxHighlighting STATIC ${KSYNTAX_HIGHLIGHTING_PRIVATE_SRC} $<TARGET_OBJECTS:SyntaxHighlightingData>)
  generate_export_header(VtSyntaxHighlighting BASE_NAME KSyntaxHighlighting)
  set_target_properties(VtSyntaxHighlighting PROPERTIES
    VERSION ${SyntaxHighlighting_VERSION_STRING}
    SOVERSION ${SyntaxHighlighting_SOVERSION}
    EXPORT_NAME SyntaxHighlighting
  )
  target_include_directories(VtSyntaxHighlighting INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KSyntaxHighlighting;${KDE_INSTALL_INCLUDEDIR_KF5}>")
  target_include_directories(VtSyntaxHighlighting PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")
  target_link_libraries(VtSyntaxHighlighting LINK_PUBLIC Qt5::Gui LINK_PRIVATE Qt5::Network)
endif()

set_source_files_properties(vt-kate-syntax-highlighter.cpp COMPILE_FLAGS -DBUILD_VT_TRACE=${vt_trace})

# ecm_qt_declare_logging_category(vt_syntax_highlighting_srcs
#   HEADER vtsyntaxhighlighting_logging.h
#   IDENTIFIER VtSyntaxHighlighting::Log
#   CATEGORY_NAME syntaxhighlighting
# )

add_executable(vt-kate-syntax-highlighter ${vt_syntax_highlighting_srcs})
# set_target_properties(vt-kate-syntax-highlighter PROPERTIES COMPILE_FLAGS -flto)
# set_target_properties(vt-kate-syntax-highlighter PROPERTIES LINK_FLAGS -flto)
ecm_mark_nongui_executable(vt-kate-syntax-highlighter)
if(${BUILD_VT_TRACE})
  target_link_libraries(vt-kate-syntax-highlighter VtSyntaxHighlighting)
else()
  target_link_libraries(vt-kate-syntax-highlighter KF5::SyntaxHighlighting)
endif()

install(TARGETS vt-kate-syntax-highlighter ${INSTALL_TARGETS_DEFAULT_ARGS})


#
# Translations
#
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
  ecm_install_po_files_as_qm(po)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
