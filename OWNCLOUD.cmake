set( APPLICATION_NAME       "S-BOX SVD-Bau" )
set( APPLICATION_EXECUTABLE "sbos_SVD-Bau" )
set( APPLICATION_DOMAIN     "sbox-svd-bau.svdgmbh.at" )
set( APPLICATION_VENDOR     "ecqui GmbH" )
set( APPLICATION_UPDATE_URL "https://sbox-svd-bau.svdgmbh.at/client/" CACHE string "URL for updater" )

set( THEME_CLASS            "ownCloudTheme" )
set( APPLICATION_REV_DOMAIN "com.ecqui.owncloud.sbox-svd-bau.desktopclient" )
set( WIN_SETUP_BITMAP_PATH  "${CMAKE_SOURCE_DIR}/admin/win/nsi" )

set( MAC_INSTALLER_BACKGROUND_FILE "${CMAKE_SOURCE_DIR}/admin/osx/installer-background.png" CACHE STRING "The MacOSX installer background image")

# set( THEME_INCLUDE          "${OEM_THEME_DIR}/mytheme.h" )
# set( APPLICATION_LICENSE    "${OEM_THEME_DIR}/license.txt )

option( WITH_CRASHREPORTER "Build crashreporter" OFF )
set( CRASHREPORTER_SUBMIT_URL "https://sbox-svd-bau.svdgmbh.at/submit" CACHE string "URL for crash reporter" )
set( CRASHREPORTER_ICON ":/owncloud-icon.png" )
