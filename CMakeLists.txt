#THIS FILE SHOULD BE PUT INTO THE OF PROJECT FOLDER

# ========================================================
# ===================== CMake Settings ===================
# ========================================================
cmake_minimum_required( VERSION 3.3 )

project( ofxMTAppFramework )
# add_subdirectory(example)

# --------------------- Source Files ---------------------

file(   GLOB_RECURSE
        APP_SRC
        "src/*.cpp"
        )

set( ${APP_NAME}_SOURCE_FILES
        ${APP_SRC} )


# ------------------------ AddOns  -----------------------
#set( OFX_ADDONS_ACTIVE
#        ofxXmlSettings
#        ofxGui
#        ofxPiMapper
##        ofxIO
#        )

# =========================================================================
# ============================== OpenFrameworks ===========================
# =========================================================================
#include( ${OF_DIRECTORY_BY_USER}/addons/ofxCMake/modules/main.cmake )
# =========================================================================
