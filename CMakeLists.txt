cmake_minimum_required(VERSION 3.0)

option(BUILD_VT_TRACE "Build vt-trace options" OFF)
option(BUILD_VT_TRACE_CONTEXT "Build vt-trace options" OFF)

if (${BUILD_VT_TRACE_CONTEXT} AND ${BUILD_VT_TRACE})
  set(VT_VERSION "5.62.0")
else()
  set(VT_VERSION "1.0.1")
endif()

project(CliSyntaxHighlighting VERSION ${VT_VERSION})

# find_package(ECM 5.31.0 REQUIRED NO_MODULE)
find_package (ECM 5.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

include(FeatureSummary)
include(ECMMarkNonGuiExecutable)
# include(CMakePackageConfigHelpers)
# include(ECMPoQmTools)
# include(ECMQtDeclareLoggingCategory)


# ecm_setup_version(PROJECT
#     VARIABLE_PREFIX VtSyntaxHighlighting
#     VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/vtsyntaxhighlighting_version.h"
#     PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/VtSyntaxHighlightingConfigVersion.cmake"
# )

#
# Compiler flags
#
include(jln.cmake)
jln_init_flags(
  SUGGESTIONS on RTTI off LINKER gold
  BUILD_TYPE Release    LTO on CPU native WHOLE_PROGRAM strip_all
  BUILD_TYPE MinSizeRel LTO on CPU native WHOLE_PROGRAM strip_all
  BUILD_TYPE Debug      SANITIZERS on DEBUG on STL_DEBUG on
)
jln_target_interface(compflags INTERFACE)

#
# Dependencies
#
set(QT_MIN_VERSION "5.4.0")
find_package(Qt5 ${QT_MIN_VERSION} NO_MODULE REQUIRED COMPONENTS Core)

set(CMAKE_CXX_STANDARD 17)

include_directories(".")

set(vt_syntax_highlighting_srcs
  vt-kate-syntax-highlighter.cpp
  vthighlighter.cpp)
set(use_KF5SyntaxHighlighting 1)

if(${BUILD_VT_TRACE})
  set(vt_syntax_highlighting_srcs ${vt_syntax_highlighting_srcs} vttracehighlighter.cpp)
  add_compile_definitions(BUILD_VT_TRACE=1)

  if (${BUILD_VT_TRACE_CONTEXT})
    set(use_KF5SyntaxHighlighting 0)
    set(LibSyntaxHighlighting VtSyntaxHighlighting)
    add_compile_definitions(BUILD_VT_TRACE_CONTEXT=1)

    add_definitions(-DQT_NO_FOREACH)
    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)

    set(KSYNTAX_HIGHLIGHTING_ROOT_PROJECT "ksyntax-highlighting")

    include(GenerateExportHeader)
    include(ECMSetupVersion)
    include(ECMGenerateHeaders)
    include(ECMQtDeclareLoggingCategory)

    ecm_setup_version(PROJECT
      VARIABLE_PREFIX SyntaxHighlighting
      VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksyntaxhighlighting_version.h"
      PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SyntaxHighlightingConfigVersion.cmake"
    )

    find_package(Qt5 ${QT_MIN_VERSION} NO_MODULE REQUIRED COMPONENTS Network Gui)

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
      CATEGORY_NAME kf.syntaxhighlighting
      OLD_CATEGORY_NAMES org.kde.ksyntaxhighlighting
      DESCRIPTION "Syntax Highlighting"
      EXPORT KSYNTAXHIGHLIGHTING
    )

    qt5_add_resources(themes_QRC ${KSYNTAX_HIGHLIGHTING_ROOT_PROJECT}/data/themes/theme-data.qrc)
    add_library(SyntaxHighlightingData OBJECT ${themes_QRC})
    set_property(TARGET SyntaxHighlightingData PROPERTY POSITION_INDEPENDENT_CODE 1)
    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0")
      target_link_libraries(SyntaxHighlightingData PRIVATE Qt5::Core)
    endif()

    add_library(VtSyntaxHighlighting STATIC ${KSYNTAX_HIGHLIGHTING_PRIVATE_SRC} $<TARGET_OBJECTS:SyntaxHighlightingData>)
    generate_export_header(VtSyntaxHighlighting BASE_NAME KSyntaxHighlighting)
    set_target_properties(VtSyntaxHighlighting PROPERTIES
      VERSION ${SyntaxHighlighting_VERSION_STRING}
      SOVERSION ${SyntaxHighlighting_SOVERSION}
      EXPORT_NAME SyntaxHighlighting
    )
    target_link_libraries(VtSyntaxHighlighting LINK_PUBLIC Qt5::Gui LINK_PRIVATE Qt5::Network)
  endif()
endif()

if (${use_KF5SyntaxHighlighting})
  set(KF5_MIN_VERSION "5.28.0")
  find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS SyntaxHighlighting)
  set(LibSyntaxHighlighting KF5::SyntaxHighlighting)
endif()

# ecm_qt_declare_logging_category(vt_syntax_highlighting_srcs
#   HEADER vtsyntaxhighlighting_logging.h
#   IDENTIFIER VtSyntaxHighlighting::Log
#   CATEGORY_NAME syntaxhighlighting
# )

add_executable(vt-kate-syntax-highlighter ${vt_syntax_highlighting_srcs})
ecm_mark_nongui_executable(vt-kate-syntax-highlighter)
target_link_libraries(vt-kate-syntax-highlighter compflags ${LibSyntaxHighlighting})


install(TARGETS vt-kate-syntax-highlighter ${INSTALL_TARGETS_DEFAULT_ARGS})


#
# Translations
#
# if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
#   ecm_install_po_files_as_qm(po)
# endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
