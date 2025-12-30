#include "modding.h"
#include "global.h"

extern void EnFall_Moon_Draw(Actor* thisx, PlayState* play);

RECOMP_PATCH void EnFall_LodMoon_Draw(Actor* thisx, PlayState* play) {
    EnFall_Moon_Draw(thisx, play);
}

// We need to make sure that whenever the game wants to load object_lodmoon into memory, it actually loads object_fall.
// This is needed because otherwise EnFall_Moon_Draw tries to access gMoonDL, which isn't loaded into memory, and we get a segfault.
RECOMP_HOOK("Main") void Main_Init(void* arg) {
    gObjectTable[OBJECT_LODMOON] = gObjectTable[OBJECT_FALL];
}