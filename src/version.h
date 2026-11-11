// Single source of truth for the product version.
// Both .rc files include this header. The release pipeline overwrites these
// values (this file only, ASCII) from the git tag before building.
#pragma once

#define APP_VER_MAJOR 1
#define APP_VER_MINOR 0
#define APP_VER_PATCH 0
#define APP_VER_BUILD 0

// Four-part version string used in VERSIONINFO resources.
#define APP_VER_STRING "1.0.0.0"
