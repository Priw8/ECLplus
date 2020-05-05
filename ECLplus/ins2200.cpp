/*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce this list of conditions and the following disclaimer
*    in the documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
* OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "pch.h"
#include "ins2200.h"
#include "ECLplus.h"

static std::map<DWORD, MESSAGELIST*> msgmap;

#define ENMARR_LEN 2000
static std::array<ENEMY*, ENMARR_LEN> enmArr;

static VOID ChannelReset(MESSAGELIST* list) {
    for (auto itr = list->begin(); itr != list->end(); ++itr) {
        delete *itr;
    }
    list->clear();
}

static VOID MsgReset() {
    while (!msgmap.empty()) {
        auto itr = msgmap.begin();
        ChannelReset(itr->second);
        delete itr->second;
        msgmap.erase(itr);
    }
}

static VOID MsgReset(LONG channel) {
    auto itr = msgmap.find(channel);
    if (itr != msgmap.end()) {
        ChannelReset(itr->second);
        delete itr->second;
        msgmap.erase(itr);
    }
}

static VOID MsgSend(LONG channel, MESSAGE* msg) {
    if (msgmap.find(channel) == msgmap.end()) {
        msgmap.insert({channel, new MESSAGELIST});
    }
    auto list = msgmap.at(channel);
    list->push_back(msg);
}

static MESSAGE* MsgReceive(LONG channel, BOOL pop) {
    auto itr = msgmap.find(channel);
    if (itr != msgmap.end()) {
        if (!itr->second->empty()) {
            MESSAGE* msg = itr->second->front();
            if (pop)
                itr->second->pop_front();
            return msg;
        }
    }
    return NULL;
}

static DWORD GetOptionalIntArg(INSTR* ins, ENEMY* enm, DWORD paramN, DWORD default_) {
    return (ins->paramCount <= paramN) ? default_ : GetIntArg(enm, paramN);
}

static LONG ApplyBombShields(LONG dmg, ENEMY* target, DWORD isBomb) {
    if (!isBomb)
        return dmg;

    if (target->flags & FLAG_BOMBSHIELD)
        return 0;

    return (LONG)((FLOAT)dmg * target->bombInvuln);
}

struct COLLISION_CIRCLE {
    FLOAT x;
    FLOAT y;
    FLOAT radius;
};

struct COLLISION_RECT {
    FLOAT x;
    FLOAT y;
    FLOAT w;
    FLOAT h;
    FLOAT rotation;
};

struct COLLISION_SHAPE {
    BOOL is_rect;
    union {
        COLLISION_CIRCLE circ;
        COLLISION_RECT rect;
        POINTFLOAT pos;
    } u;
};

static BOOL CollisionCheckCircleCircle(COLLISION_CIRCLE *a, COLLISION_CIRCLE *b) {
    FLOAT x = b->x - a->x;
    FLOAT y = b->y - a->y;
    FLOAT effectiveRadius = a->radius + b->radius;
    return x * x + y * y < effectiveRadius * effectiveRadius;
}

static BOOL CollisionCheckCircleRect(COLLISION_CIRCLE *circ, COLLISION_RECT *rect) {
    // translate so that the rectangle is centered on the origin
    FLOAT xDiff = circ->x - rect->x;
    FLOAT yDiff = circ->y - rect->y;

    // rotate so that rectangle is on the axes
    if (rect->rotation != 0) {
        FLOAT c = cos(-rect->rotation);
        FLOAT s = sin(-rect->rotation);
        FLOAT xDiffNew = xDiff * c - yDiff * s;
        yDiff = xDiff * s + yDiff * c;
        xDiff = xDiffNew;
    }

    // apply mirror symmetry to move circle to upper right quadrant
    FLOAT xDist = fabsf(xDiff);
    FLOAT yDist = fabsf(yDiff);

    if (xDist > 0.5f * rect->w + circ->radius) return FALSE;
    else if (yDist > 0.5f * rect->h + circ->radius) return FALSE;

    // overlapping a single edge?
    else if (xDist <= 0.5 * rect->w) return TRUE;
    else if (yDist <= 0.5 * rect->h) return TRUE;

    else {
        // overlapping a corner?
        FLOAT distSq = powf(xDist - 0.5f * rect->w, 2.0f) + powf(yDist - 0.5f * rect->h, 2.0f);
        return distSq <= powf(circ->radius, 2.0f);
    }
}

void (*COLLIDE_RECT_RECT)() = (void (*)())0x404320;

static __declspec(naked) BOOL __stdcall CollisionCheckRectRect(COLLISION_RECT *a, COLLISION_RECT *b) {
    __asm {
        push  ebp
        mov   ebp, esp
        sub   esp, 0x18

        mov   ecx, [ebp + 0x08] // a
        mov   eax, [ecx + 0x00] // a.x
        mov   [esp + 0x04], eax
        mov   eax, [ecx + 0x04] // a.y
        mov   [esp + 0x08], eax
        mov   eax, [ecx + 0x08] // a.w
        mov   [esp + 0x0c], eax
        mov   eax, [ecx + 0x0c] // a.h
        mov   [esp + 0x10], eax
        mov   eax, [ecx + 0x10] // a.rotation
        mov   [esp + 0x14], eax

        mov   ecx, [ebp + 0x0c] // b
        movss xmm0, [ecx + 0x00] // b.x
        movss xmm1, [ecx + 0x04] // b.y
        movss xmm2, [ecx + 0x08] // b.w
        movss xmm3, [ecx + 0x0c] // b.h
        mov   eax, [ecx + 0x10] // b.rotation
        mov   [esp + 0x00], eax

        call  [COLLIDE_RECT_RECT]
        mov   esp, ebp
        pop   ebp
        ret   0x8
    }
}

static void GetEnemyCollisionShape(COLLISION_SHAPE *shape, const ENEMY *enm) {
    if (enm->flags & FLAG_RECT_HITBOX) {
        shape->is_rect = true;
        shape->u.rect = { enm->pos.x, enm->pos.y, enm->hurtbox.x, enm->hurtbox.y, enm->rotation };
    }
    else {
        FLOAT radius = 0.5f * enm->hurtbox.x;
        shape->is_rect = false;
        shape->u.circ = { enm->pos.x, enm->pos.y, radius };
    }
}

static BOOL CollisionCheck(COLLISION_SHAPE *a, COLLISION_SHAPE *b) {
    if (a->is_rect) {
        if (b->is_rect) {
            return CollisionCheckRectRect(&a->u.rect, &b->u.rect);
        } else {
            return CollisionCheckCircleRect(&b->u.circ, &a->u.rect);
        }
    } else {
        if (b->is_rect) {
            return CollisionCheckCircleRect(&a->u.circ, &b->u.rect);
        } else {
            return CollisionCheckCircleCircle(&a->u.circ, &b->u.circ);
        }
    }
}

static LONG DamageEnemiesImpl(COLLISION_SHAPE *dmgShape, LONG dmg, LONG maxcnt, DWORD isBomb) {
    ENEMYLISTNODE* node = GameEnmMgr->head;
    DWORD i = 0;

    while(node != NULL) {
        ENEMY* iterEnm = &node->obj->enm;
        if (!(iterEnm->flags & (FLAG_INTANGIBLE | FLAG_NO_HURTBOX))) {
            COLLISION_SHAPE enmShape;
            GetEnemyCollisionShape(&enmShape, iterEnm);

            if (CollisionCheck(&enmShape, dmgShape)) {
                enmArr[i] = iterEnm;
                ++i;
                if (i == ENMARR_LEN) {
                    EclMsg("enmDamage: ran out of space for enemies in the preallocated buffer. Why are you using over 2000 real enemies?");
                    return -1;
                }
            }
        }
        node = node->next;
    }

    std::sort(enmArr.begin(), enmArr.begin() + i, [dmgShape](ENEMY* enm1, ENEMY* enm2) {
        return
            powf(enm1->pos.x - dmgShape->u.pos.x, 2) + powf(enm1->pos.y - dmgShape->u.pos.y, 2)
            <
            powf(enm2->pos.x - dmgShape->u.pos.x, 2) + powf(enm2->pos.y - dmgShape->u.pos.y, 2);
    });

    LONG cnt = 0;
    for (DWORD itr = 0; itr < i; ++itr) {
        if (maxcnt > 0 && cnt >= maxcnt)
            break;
        enmArr[itr]->pendingDmg += ApplyBombShields(dmg, enmArr[itr], isBomb);
        ++cnt;
    }
}

BOOL ins_2200(ENEMY* enm, INSTR* ins) {
    switch (ins->id) {
        case INS_MSG_RESET:
            MsgReset();
            break;
        case INS_MSG_RESET_CHANNEL:
            MsgReset(GetIntArg(enm, 0));
            break;
        case INS_MSG_SEND: {
            MESSAGE* msg = new MESSAGE;
            msg->a = GetFloatArg(enm, 0);
            msg->b = GetFloatArg(enm, 1);
            msg->c = GetFloatArg(enm, 2);
            msg->d = GetFloatArg(enm, 3);
            MsgSend(GetIntArg(enm, 4), msg);
            break;
        }
        case INS_MSG_RECEIVE:
        case INS_MSG_PEEK: {
            MESSAGE* msg = MsgReceive(GetIntArg(enm, 5), ins->id != INS_MSG_PEEK);
            
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) *ptr = msg == NULL ? 0 : 1;
            if (msg == NULL) break;

            FLOAT* fptr;
            fptr = GetFloatArgAddr(enm, 1);
            if (fptr != NULL) *fptr = msg->a;
            fptr = GetFloatArgAddr(enm, 2);
            if (fptr != NULL) *fptr = msg->b;
            fptr = GetFloatArgAddr(enm, 3);
            if (fptr != NULL) *fptr = msg->c;
            fptr = GetFloatArgAddr(enm, 4);
            if (fptr != NULL) *fptr = msg->d;

            if (ins->id != INS_MSG_PEEK)
                delete msg;

            break;
        }
        case INS_MSG_CHECK: {
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) *ptr = MsgReceive(GetIntArg(enm, 1), FALSE) != NULL;
            break;
        }
        case INS_GET_CLOSEST_ENM: {
            FLOAT x = GetFloatArg(enm, 2), y = GetFloatArg(enm, 3);
            ENEMYLISTNODE* node = GameEnmMgr->head;
            ENEMYFULL* closestEnm = NULL;
            FLOAT closest = INFINITY;
            while(node != NULL) {
                if (!(node->obj->enm.flags & (FLAG_INTANGIBLE | FLAG_NO_HURTBOX))) {
                    FLOAT dist = powf(node->obj->enm.pos.x - x, 2) + powf(node->obj->enm.pos.y - y, 2);
                    if (dist < closest) {
                        closestEnm = node->obj;
                        closest = dist;
                    }
                }
                node = node->next;
            }
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) {
                if (closestEnm != NULL)
                    *ptr = closestEnm->enm.id;
                else
                    *ptr = 0;
            }
            FLOAT* fptr = GetFloatArgAddr(enm, 1);
            if (fptr != NULL) {
                *fptr = sqrtf(closest);
            }
            break;
        }
        /* Eneme damaging instr can take an extra parameter (0 by default),
         * determines whether this is "bomb damage" and bombshield/invluln should take effect. */
        case INS_ENM_DAMAGE: {
            ENEMYFULL* foundEnm = GetEnmById(GetIntArg(enm, 0));
            DWORD isBomb = GetOptionalIntArg(ins, enm, 2, 0);
            if (foundEnm != NULL) {
                foundEnm->enm.pendingDmg += ApplyBombShields(GetIntArg(enm, 1), &foundEnm->enm, isBomb);
            }
            break;
        }
        case INS_ENM_DAMAGE_ITER: {
            ENEMYLISTNODE* foundEnm = (ENEMYLISTNODE*)GetIntArg(enm, 0);
            DWORD isBomb = GetOptionalIntArg(ins, enm, 2, 0);
            if (foundEnm != NULL) {
                foundEnm->obj->enm.pendingDmg += ApplyBombShields(GetIntArg(enm, 1), &foundEnm->obj->enm, isBomb);
            }
            break;
        }
        case INS_ENM_DAMAGE_RADIUS: {
            FLOAT x = GetFloatArg(enm, 1), 
                  y = GetFloatArg(enm, 2),
                  rad = GetFloatArg(enm, 3);

            LONG maxcnt = GetIntArg(enm, 4);
            LONG dmg = GetIntArg(enm, 5);
            DWORD isBomb = GetOptionalIntArg(ins, enm, 6, 0);

            COLLISION_SHAPE dmgShape;
            dmgShape.is_rect = false;
            dmgShape.u.circ = { x, y, rad };

            LONG cnt = DamageEnemiesImpl(&dmgShape, dmg, maxcnt, isBomb);
            if (cnt < 0)
                return TRUE; // error

            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) *ptr = cnt;

            break;
        }
        case INS_ENM_DAMAGE_RECT: {
            FLOAT x = GetFloatArg(enm, 1),
                  y = GetFloatArg(enm, 2),
                  w = GetFloatArg(enm, 3),
                  h = GetFloatArg(enm, 4);

            LONG maxcnt = GetIntArg(enm, 5);
            LONG dmg = GetIntArg(enm, 6);
            DWORD isBomb = GetOptionalIntArg(ins, enm, 7, 0);

            COLLISION_SHAPE dmgShape;
            dmgShape.is_rect = true;
            dmgShape.u.rect = { x, y, w, h, 0.0 };

            LONG cnt = DamageEnemiesImpl(&dmgShape, dmg, maxcnt, isBomb);
            if (cnt < 0)
                return TRUE; // error

            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) *ptr = cnt;

            break;
        }
        case INS_ENM_DAMAGE_RECT_ROT: {
            FLOAT x = GetFloatArg(enm, 1),
                  y = GetFloatArg(enm, 2),
                  w = GetFloatArg(enm, 3),
                  h = GetFloatArg(enm, 4),
                  rot = GetFloatArg(enm, 5);

            LONG maxcnt = GetIntArg(enm, 6);
            LONG dmg = GetIntArg(enm, 7);
            DWORD isBomb = GetOptionalIntArg(ins, enm, 8, 0);

            COLLISION_SHAPE dmgShape;
            dmgShape.is_rect = true;
            dmgShape.u.rect = { x, y, w, h, rot };

            LONG cnt = DamageEnemiesImpl(&dmgShape, dmg, maxcnt, isBomb);
            if (cnt < 0)
                return TRUE; // error

            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) *ptr = cnt;

            break;
        }
        case INS_ENM_ITERATOR: {
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) {
                ENEMYLISTNODE* prev = (ENEMYLISTNODE*)GetIntArg(enm, 1), *next;
                if (prev == NULL)
                    next = GameEnmMgr->head;
                else
                    next = prev->next;
                *ptr = (LONG)next;
            }
            break;
        }
        case INS_ENM_ID_FROM_ITER: {
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) {
                ENEMYLISTNODE* node = (ENEMYLISTNODE*)GetIntArg(enm, 1);
                if (node == NULL)
                    *ptr = 0;
                else
                    *ptr = node->obj->enm.id;
            }
            break;
        }
        case INS_ENM_ITER_FROM_ID: {
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) {
                LONG id = GetIntArg(enm, 1);
                ENEMYLISTNODE* node = GameEnmMgr->head;
                while(node != NULL && node->obj->enm.id != id) {
                    node = node->next;   
                }
                *((ENEMYLISTNODE**)ptr) = node;
            }
            break;
        }
        case INS_ENM_FLAGS: {
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) {
                ENEMYFULL* nenm = GetEnmById(GetIntArg(enm, 1));
                if (nenm != NULL)
                    *ptr = nenm->enm.flags;
                else
                    *ptr = 0;
            }
            break;
        }
        case INS_ENM_FLAGS_ITER: {
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) {
                ENEMYLISTNODE* nenm = (ENEMYLISTNODE*)GetIntArg(enm, 1);
                if (nenm != NULL)
                    *ptr = nenm->obj->enm.flags;
                else
                    *ptr = 0;
            }
            break;
        }
        case INS_ENM_HP: {
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) {
                ENEMYFULL* nenm = GetEnmById(GetIntArg(enm, 1));
                if (nenm != NULL)
                    *ptr = nenm->enm.hp;
                else
                    *ptr = 0;
            }
            break;
        }
        case INS_ENM_HP_ITER: {
            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) {
                ENEMYLISTNODE* nenm = (ENEMYLISTNODE*)GetIntArg(enm, 1);
                if (nenm != NULL)
                    *ptr = nenm->obj->enm.hp;
                else
                    *ptr = 0;
            }
            break;
        }
        case INS_ENM_POS_ITER: {
            FLOAT* fptrx = GetFloatArgAddr(enm, 0);
            FLOAT* fptry = GetFloatArgAddr(enm, 1);
            if (fptrx != NULL || fptry != NULL) {
                ENEMYLISTNODE* nenm = (ENEMYLISTNODE*)GetIntArg(enm, 2);
                FLOAT x, y;
                if (nenm == NULL) {
                    x = 0.0f;
                    y = 0.0f;
                } else {
                    x = nenm->obj->enm.pos.x;
                    y = nenm->obj->enm.pos.y;
                }
                if (fptrx != NULL)
                    *fptrx = x;
                if (fptry != NULL)
                    *fptry = y;
            }
            break;
        }
        case INS_ENM_BOMBINVULN: {
            FLOAT* fptr = GetFloatArgAddr(enm, 0);
            if (fptr != NULL) {
                ENEMYFULL* nenm = GetEnmById(GetIntArg(enm, 1));
                if (nenm != NULL)
                    *fptr = nenm->enm.bombInvuln;
                else
                    *fptr = 0.0f;
            }
        }
        case INS_ENM_BOMBINVULN_ITER: {
            FLOAT* fptr = GetFloatArgAddr(enm, 0);
            if (fptr != NULL) {
                ENEMYLISTNODE* nenm = (ENEMYLISTNODE*)GetIntArg(enm, 2);
                if (nenm != NULL)
                    *fptr = nenm->obj->enm.bombInvuln;
                else
                    *fptr = 0.0f;
            }
        }
        default:
            return FALSE;
    }
    return TRUE;
}