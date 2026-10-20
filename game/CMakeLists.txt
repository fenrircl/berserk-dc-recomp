# The port target. Shape copied from soulcalibur-recomp (the reference port that plays end to end).
# No icon yet: add ICON/ICON_ICO here once there is original artwork for this port (never the
# game's own art).
dreamcomp_add_port(berserk
  TITLE "Sword of the Berserk: Guts' Rage"
  OUTPUT_NAME berserk-recomp
  SOURCES ${PROJECT_SOURCE_DIR}/src/berserk.cpp)
