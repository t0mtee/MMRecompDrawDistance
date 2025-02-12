#include "modding.h"
#include "global.h"

#include "overlays/actors/ovl_Obj_Grass/z_obj_grass.h"
#include "overlays/actors/ovl_En_Wood02/z_en_wood02.h"
#include "z64actor.h"

RECOMP_FORCE_PATCH s32 Actor_CullingVolumeTest(PlayState* play, Actor* actor, Vec3f* projPos, f32 projW) {
    if ((projPos->z > -actor->cullingVolumeScale) &&
        (projPos->z < (actor->cullingVolumeDistance + actor->cullingVolumeScale))) {
        f32 invW;
        f32 cullingVolumeScaleX;
        f32 cullingVolumeScaleY;
        f32 cullingVolumeDownward;

        // Clamping `projW` affects points behind the camera, so that the culling volume has
        // a frustum shape in front of the camera and a box shape behind the camera.
        invW = CLAMP_MIN(projW, 1.0f);

        if (play->view.fovy != 60.0f) {
            // If the fov isn't 60 degrees, make the cull parameters behave as if it were 60 degrees.
            // To do this, multiply by the ratios of the x and y diagonal elements of the projection matrix.
            // The x diagonal element is cot(0.5 * fov) / aspect and the y diagonal element is just cot(0.5 * fov).
            // When the fov is 60 degrees, cot(0.5 * 60 degrees) = sqrt(3) so the x element is 3sqrt(3)/4 and the y
            // element is sqrt(3). The current diagonal element divided by (or multiplied by their inverse) gives
            // the ratio.

            cullingVolumeScaleX = actor->cullingVolumeScale * play->projectionMtxFDiagonal.x *
                                  0.76980036f; // sqrt(16/27) = aspect / cot(0.5 * f) = (4/3) / sqrt(3)

            cullingVolumeDownward = play->projectionMtxFDiagonal.y * 0.57735026f; // 1 / sqrt(3) = 1 / cot(0.5 * f)
            cullingVolumeScaleY = actor->cullingVolumeScale * cullingVolumeDownward;
            cullingVolumeDownward *= actor->cullingVolumeDownward;
        } else {
            cullingVolumeScaleY = cullingVolumeScaleX = actor->cullingVolumeScale;
            cullingVolumeDownward = actor->cullingVolumeDownward;
        }

        if (((fabsf(projPos->x) - cullingVolumeScaleX) < invW) && ((-invW < (projPos->y + cullingVolumeDownward))) &&
            ((projPos->y - cullingVolumeScaleY) < invW)) {
            return true;
        }
    }

    // +++++ 2SHIP CODE +++++
    // Apply distance scale to forward cullzone check
    bool isWithingForwardCullZone =
        (-actor->cullingVolumeScale < projPos->z) &&
        (projPos->z < ((actor->cullingVolumeDistance + actor->cullingVolumeScale) * 5));

    if (isWithingForwardCullZone) {
        // Ensure the projected W value is at least 1.0
        f32 clampedprojW = CLAMP_MIN(projW, 1.0f);
        f32 aspectMultiplier = 1.0f;
        f32 cullingVolumeScaleDiagonal;
        f32 cullingVolumeScaleVertical;
        f32 cullingVolumeDownwardAdjusted;

        // Adjust calculations if the field of view is not the default 60 degrees
        if (play->view.fovy != 60.0f) {
            cullingVolumeScaleDiagonal =
                actor->cullingVolumeScale * play->projectionMtxFDiagonal.x * 0.76980036f; // sqrt(16/27)

            cullingVolumeScaleVertical = play->projectionMtxFDiagonal.y * 0.57735026f; // 1 / sqrt(3)
            cullingVolumeDownwardAdjusted = actor->cullingVolumeScale * cullingVolumeScaleVertical;
            cullingVolumeScaleVertical *= actor->cullingVolumeDownward;
        } else {
            cullingVolumeDownwardAdjusted = cullingVolumeScaleDiagonal = actor->cullingVolumeScale;
            cullingVolumeScaleVertical = actor->cullingVolumeDownward;
        }

        // if (CVarGetInteger("gEnhancements.Graphics.ActorCullingAccountsForWidescreen", 0)) {
        //     aspectMultiplier = Ship_GetExtendedAspectRatioMultiplier();
        // }

        // Apply adjsuted aspect ratio to just the horizontal cullzone check
        bool isWithinHorizontalCullZone =
            ((fabsf(projPos->x) - cullingVolumeScaleDiagonal) < (clampedprojW * aspectMultiplier));
        bool isAboveBottomOfCullZone = ((-clampedprojW < (projPos->y + cullingVolumeScaleVertical)));
        bool isBelowTopOfCullZone = ((projPos->y - cullingVolumeDownwardAdjusted) < clampedprojW);

        if (isWithinHorizontalCullZone && isAboveBottomOfCullZone && isBelowTopOfCullZone) {
            // Add additional overries here for glitch useful actors when those are reported
            return true;
        }
    }

    return false;

    // ----- 2SHIP CODE -----
}

extern s16 func_809A9110(PlayState* play, Vec3f* pos);

RECOMP_PATCH void ObjGrass_InitDraw(ObjGrass* this, PlayState* play) {
    ObjGrassGroup* grassGroup;
    ObjGrassElement* grassElem;
    s32 i;
    s32 j;
    f32 distSq;
    f32 eyeDist;

    for (i = 0; i < this->activeGrassGroups; i++) {
        grassGroup = &this->grassGroups[i];

        eyeDist = Math3D_Vec3fDistSq(&grassGroup->homePos, &GET_ACTIVE_CAM(play)->eye);
        eyeDist = SQ(sqrtf(eyeDist) / 10000000); // Hijacked

        if ((eyeDist < SQ(1280.0f)) && func_809A9110(play, &grassGroup->homePos)) {
            grassGroup->flags |= OBJ_GRASS_GROUP_DRAW;

            for (j = 0; j < grassGroup->count; j++) {
                grassElem = &grassGroup->elements[j];

                if (grassElem->flags & OBJ_GRASS_ELEM_REMOVED) {
                    grassElem->alpha = 255;
                    grassElem->flags &= ~OBJ_GRASS_ELEM_DRAW;
                } else {
                    grassElem->flags |= OBJ_GRASS_ELEM_DRAW;
                    if (eyeDist < SQ(980.0f)) {
                        grassElem->alpha = 255;
                    } else {
                        distSq = Math3D_Vec3fDistSq(&grassElem->pos, &GET_ACTIVE_CAM(play)->eye);
                        distSq = SQ(sqrtf(distSq) / 100000000); // Hijacked
                        if ((distSq <= SQ(1080.0f)) ||
                            ((grassElem->flags & OBJ_GRASS_ELEM_UNDERWATER) && (distSq < SQ(1180.0f)))) {
                            grassElem->alpha = 255;
                        } else if (distSq >= SQ(1180.0f)) {
                            grassElem->alpha = 0;
                        } else {
                            grassElem->alpha = (1180.0f - sqrtf(distSq)) * 2.55f;
                        }
                    }
                }
            }
        } else {
            grassGroup->flags &= ~OBJ_GRASS_GROUP_DRAW;
        }
    }
}

extern s32 Actor_CullingCheck(PlayState* play, Actor* actor);

RECOMP_PATCH s32 EnWood02_SpawnZoneCheck(EnWood02* this, PlayState* play, Vec3f* arg2) {
    f32 phi_f12;

    SkinMatrix_Vec3fMtxFMultXYZW(&play->viewProjectionMtxF, arg2, &this->actor.projectedPos, &this->actor.projectedW);

    return Actor_CullingCheck(play, &this->actor);

    // if (this->actor.projectedW == 0.0f) {
    //     phi_f12 = 1000.0f;
    // } else {
    //     phi_f12 = fabsf(1.0f / this->actor.projectedW);
    // }

    // if (((-this->actor.cullingVolumeScale < this->actor.projectedPos.z) &&
    //      (this->actor.projectedPos.z < (this->actor.cullingVolumeDistance + this->actor.cullingVolumeScale)) &&
    //      (((fabsf(this->actor.projectedPos.x) - this->actor.cullingVolumeScale) * phi_f12) < 1.0f)) &&
    //     (((this->actor.projectedPos.y + this->actor.cullingVolumeDownward) * phi_f12) > -1.0f) &&
    //     (((this->actor.projectedPos.y - this->actor.cullingVolumeScale) * phi_f12) < 1.0f)) {
    //     return true;
    // }
    // return false;
}