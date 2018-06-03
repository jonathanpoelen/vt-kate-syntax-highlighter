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

set(vt_syntax_highlighting_srcs
    vthighlighter.cpp
)
# ecm_qt_declare_logging_category(vt_syntax_highlighting_srcs
#     HEADER vtsyntaxhighlighting_logging.h
#     IDENTIFIER VtSyntaxHighlighting::Log
#     CATEGORY_NAME syntaxhighlighting
# )

add_executable(vt-kate-syntax-highlighter vt-kate-syntax-highlighter.cpp ${vt_syntax_highlighting_srcs})
set_target_properties(vt-kate-syntax-highlighter PROPERTIES COMPILE_FLAGS -flto)
set_target_properties(vt-kate-syntax-highlighter PROPERTIES LINK_FLAGS -flto)
ecm_mark_nongui_executable(vt-kate-syntax-highlighter)
target_link_libraries(vt-kate-syntax-highlighter KF5::SyntaxHighlighting)

install(TARGETS vt-kate-syntax-highlighter ${INSTALL_TARGETS_DEFAULT_ARGS})


#
# Translations
#
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
