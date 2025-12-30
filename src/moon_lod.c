#include "modding.h"
#include "global.h"

#include "romfile.h"

extern void EnFall_Moon_Draw(Actor* thisx, PlayState* play);

RECOMP_PATCH void EnFall_LodMoon_Draw(Actor* thisx, PlayState* play) {
    EnFall_Moon_Draw(thisx, play);
}

DECLARE_ROM_SEGMENT(object_fall)

// We need to make sure that whenever the game wants to load object_lodmoon into memory, it actually loads object_fall.
// This is needed because otherwise EnFall_Moon_Draw tries to access gMoonDL, which isn't loaded into memory, and we get a segfault.
RECOMP_CALLBACK("*", recomp_after_play_init) void after_play_init(PlayState* this) {
    gObjectTable[OBJECT_LODMOON] = (RomFile)ROM_FILE(object_fall);
}