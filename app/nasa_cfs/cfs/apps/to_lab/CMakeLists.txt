cmake_minimum_required(VERSION 3.5)
project(CFS_TO_LAB C)

set(TO_TABLE_COMPILE_DEFS)

# These references are specifically needed for the table build
# it is easiest to add them as directory properties so they won't
# be considered include directories for TO_LAB itself.  Each one
# gets a macro for conditional inclusion in the subscription table.
foreach(EXT_APP ci_lab sample_app hs fm ds sc lc)
  list (FIND TGTSYS_${SYSVAR}_APPS ${EXT_APP} HAVE_APP)
  if (HAVE_APP GREATER_EQUAL 0)
    include_directories($<TARGET_PROPERTY:${EXT_APP},INTERFACE_INCLUDE_DIRECTORIES>)
    string(TOUPPER "HAVE_${EXT_APP}" APP_MACRO)
    add_definitions(-D${APP_MACRO})
  endif()
endforeach()

set(APP_SRC_FILES
    fsw/src/to_lab_app.c
)

# Create the app module
add_cfe_app(to_lab ${APP_SRC_FILES})
add_cfe_tables(to_lab fsw/tables/to_lab_sub.c)

target_include_directories(to_lab PUBLIC
    fsw/mission_inc
    fsw/platform_inc
)
