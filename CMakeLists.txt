project(photoSort2)
cmake_minimum_required(VERSION 2.8)

set(TARGET_QT_VERSION "5" CACHE STRING "Expected Qt version")

set_property(CACHE TARGET_QT_VERSION PROPERTY STRINGS 4 5)

if(NOT (TARGET_QT_VERSION VERSION_EQUAL "4" OR TARGET_QT_VERSION VERSION_EQUAL "5"))
  message(FATAL_ERROR "Expected value for TARGET_QT_VERSION is either '4' or '5'")
endif()

set(SRC_LIST
    main.cpp
    photosortmainwindow.h photosortmainwindow.cpp
    photosortpreview.h    photosortpreview.cpp
    photosortmodel.h      photosortmodel.cpp
    photosortitem.h       photosortitem.cpp
    detailscene.h         detailscene.cpp
    photodetaileditem.h   photodetaileditem.cpp
    )

set(Srcs ${SRC_LIST})
set(Hdrs "")
set(QRCs ${RC_LIST})
set(MOC_Hdrs ${MOC_LIST})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(TARGET_QT_VERSION VERSION_GREATER "4")
  find_package(Qt5Widgets REQUIRED)
  set(QRC_Srcs ${QRCs} )
  add_executable(${PROJECT_NAME}
      ${SRC_LIST}
      ${QRC_Srcs}
      )
  qt5_use_modules(${PROJECT_NAME} Core Gui Widgets)
else()
    error("Not implemented")
endif()

find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} Threads::Threads exif)
