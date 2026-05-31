cmake_minimum_required(VERSION 3.16)
project(DawnFall VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp

    # ── Backend 1: Characters & Battle (Member 1 — Cathy) ──
    character.h character.cpp
    hero.h hero.cpp
    archer.h archer.cpp
    assassin.h assassin.cpp
    mage.h mage.cpp
    support.h support.cpp
    tank.h tank.cpp
    warrior.h warrior.cpp
    skill.h skill.cpp
    enemy.h enemy.cpp
    battlesystem.h battlesystem.cpp
    farm.h farm.cpp

    # ── Backend 2: Data & Systems (Member 2 — Rere) ──
    ItemType.h
    item.h item.cpp
    itemlist.h itemlist.cpp
    inventory.h inventory.cpp
    shop.h shop.cpp
    savemanager.h savemanager.cpp
    partymanager.h partymanager.cpp

    # ── Frontend 1: UI Screens (Member 3 — Nicole) ──
    mainmenuscreen.h mainmenuscreen.cpp
    lobbyscreen.h lobbyscreen.cpp
    shopscreen.h shopscreen.cpp
    inventoryscreen.h inventoryscreen.cpp
    questscreen.h questscreen.cpp
    partysetupscreen.h partysetupscreen.cpp

    # ── Frontend 2: Battle UI & Dialogue (Member 4 — Nicole) ──
    battlescreen.h battlescreen.cpp
    dialoguescreen.h dialoguescreen.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DawnFall MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(DawnFall ${PROJECT_SOURCES})
endif()

target_link_libraries(DawnFall PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

qt_add_resources(DawnFall "assets"
    PREFIX "/"
    FILES
        assets/portraits/mc_neutral.PNG
        assets/portraits/ethan_neutral.png
        assets/portraits/hubert_neutral.PNG
        assets/profilepic/mc_pfp.png
        assets/profilepic/ethan_pfp.png
        assets/profilepic/hubert_pfp.png
        assets/profilepic/ethan_pfp_dead.png
        assets/profilepic/mc_pfp_dead.png
        assets/profilepic/hubert_pfp_dead.png
)

set_target_properties(DawnFall PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS DawnFall
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DawnFall)
endif()
