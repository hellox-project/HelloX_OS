cmake_minimum_required(VERSION 3.5)
project(CFS_SCH_LAB C)

# These references are specifically needed for the table build
# it is easiest to add them as directory properties so they won't
# be considered include directories for TO_LAB itself.  Each one
# gets a macro for conditional inclusion in the subscription table.
foreach(EXT_APP ci_lab to_lab sample_app sc hs lc ds fm)
  list (FIND TGTSYS_${SYSVAR}_APPS ${EXT_APP} HAVE_APP)
  if (HAVE_APP GREATER_EQUAL 0)
    include_directories($<TARGET_PROPERTY:${EXT_APP},INTERFACE_INCLUDE_DIRECTORIES>)
    string(TOUPPER "HAVE_${EXT_APP}" APP_MACRO)
    add_definitions(-D${APP_MACRO})
  endif()
endforeach()

# Create the app module
add_cfe_app(sch_lab fsw/src/sch_lab_app.c)
add_cfe_tables(sch_lab fsw/tables/sch_lab_table.c)

target_include_directories(sch_lab PUBLIC
    fsw/mission_inc
    fsw/platform_inc
)
