# ======================= ofxCMake Vers. 0.1 =============
#  PUT THIS FILE INTO YOUR OPENFRAMEWORKS PROJECT FOLDER

# ========================================================
# ===================== CMake Settings ===================
# ========================================================
cmake_minimum_required( VERSION 3.3 )
set (CMAKE_BUILD_RPATH "build/")
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")


project( ofxMTAppFramework_Example )
add_subdirectory("../../../addons/ofxMTAppFramework" "build")


# ========================================================
# ===================== User Settings ====================
# ========================================================
# ---------------------- App name  -----------------------
set( APP_NAME   ofxMTAppFramework_Example )

# ------------------------ OF Path -----------------------
# --- If outside the OF structure, set an absolute OF path
set( OF_DIRECTORY_BY_USER "../../.." )

# --------------------- Source Files ---------------------

file(   GLOB_RECURSE
        APP_SRC
        "src/*.cpp"
        )

set( ${APP_NAME}_SOURCE_FILES
        ${APP_SRC} )

#set(CMAKE_VERBOSE_MAKEFILE  ON)

# ------------------------ AddOns  -----------------------
set( OFX_ADDONS_ACTIVE
#        ofxOpenCv
#        ofxCv
#        ofxScreenSetup
#        ofxFX
#        ofxOsc
#        ofxProjectorBlend
        ofxImGui
        ofxMTAppFramework
        )


# =========================================================================
# ============================== OpenFrameworks ===========================
# =========================================================================
include( ${OF_DIRECTORY_BY_USER}/addons/ofxCMake/modules/main.cmake )
# =========================================================================


