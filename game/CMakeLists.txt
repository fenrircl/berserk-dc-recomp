# The port target. Shape copied from soulcalibur-recomp (the reference port that plays end to end).
# No icon yet: add ICON/ICON_ICO here once there is original artwork for this port (never the
# game's own art).
dreamcomp_add_port(berserk
  TITLE "Sword of the Berserk: Guts' Rage"
  OUTPUT_NAME berserk-recomp
  SOURCES ${PROJECT_SOURCE_DIR}/src/berserk.cpp
          ${PROJECT_SOURCE_DIR}/src/display_menu.cpp
          ${PROJECT_SOURCE_DIR}/src/menu_text.cpp
          ${PROJECT_SOURCE_DIR}/src/pc_settings.cpp
          ${PROJECT_SOURCE_DIR}/src/languages.cpp
          ${PROJECT_SOURCE_DIR}/src/port_startup.cpp)

# A normal double-click opens only the game window. `--debug` allocates a console
# explicitly for logs and opens the launcher before entering the game.
if(WIN32)
  set_target_properties(berserk_boot PROPERTIES WIN32_EXECUTABLE TRUE)
  target_link_options(berserk_boot PRIVATE "/ENTRY:mainCRTStartup")
endif()
