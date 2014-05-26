#add_subdirectory( tests )

include_directories( ${KOABSTRACTION_INCLUDES}
    ${KDE4_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/words/part
    ${KDE4_INCLUDE_DIR}/QtGui
    ${KDE4_INCLUDE_DIR}/QtCore
    ${KDE4_INCLUDE_DIR}/QtXml
    ${KDE4_INCLUDE_DIR}/KDE)

########### next target ###############

set(koabstraction_LIB_SRCS
    KoAbstractApplicationController.cpp
    KoAbstractApplicationWindow.cpp
    RemoveSheetCommand.cpp
    KoCellToolFactory.cpp
    KoCellTool.cpp
    FoCellEditor.cpp #TODO move back to tools/f-office
)

kde4_add_library(koabstraction ${LIBRARY_TYPE} ${koabstraction_LIB_SRCS})

target_link_libraries(koabstraction ${extra_libs}
    ${QT_LIBRARIES} 
    flake
    komain
    kotext
    kopageapp
    wordsprivate
    calligrasheetscommon
    ${MISSING_LIBS}
)

target_link_libraries(koabstraction LINK_INTERFACE_LIBRARIES
    kotext
    flake
    komain
    kotext
    calligrasheetscommon
    ${KDE4_KPARTS_LIBS}
    ${QT_LIBRARIES}
)

set_target_properties(koabstraction PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koabstraction ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES
    koabstraction_export.h
    KoAbstractApplicationController.h
    KoAbstractApplicationWindow.h
    KoCellTool.h
    FoCellEditor.h
    KoExternalEditorInterface.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
