#include "modding.h"
#include "global.h"

RECOMP_IMPORT("ProxyMM_ObjDepLoader", bool ObjDepLoader_Load(PlayState* play, u8 segment, s16 objectId));
RECOMP_IMPORT("ProxyMM_ObjDepLoader", void ObjDepLoader_Unload(PlayState* play, u8 segment, s16 objectId));

extern void EnFall_Moon_Draw(Actor* thisx, PlayState* play);

RECOMP_PATCH void EnFall_LodMoon_Draw(Actor* thisx, PlayState* play) {
    if (ObjDepLoader_Load(play, 0x06, OBJECT_FALL)) {
        EnFall_Moon_Draw(thisx, play);

        ObjDepLoader_Unload(play, 0x06, OBJECT_FALL);
    }
}