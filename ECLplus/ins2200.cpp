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
        case INS_ENM_DAMAGE: {
            ENEMYFULL* foundEnm = GetEnmById(GetIntArg(enm, 0));
            if (foundEnm != NULL) {
                foundEnm->enm.pendingDmg += GetIntArg(enm, 1);
            }
            break;
        }
        case INS_ENM_DAMAGE_ITER: {
            ENEMYLISTNODE* foundEnm = (ENEMYLISTNODE*)GetIntArg(enm, 0);
            if (foundEnm != NULL) {
                foundEnm->obj->enm.pendingDmg += GetIntArg(enm, 1);
            }
            break;
        }
        case INS_ENM_DAMAGE_RADIUS: {
            FLOAT x = GetFloatArg(enm, 1), 
                  y = GetFloatArg(enm, 2),
                  rad = GetFloatArg(enm, 3);
            FLOAT radSq = powf(rad, 2.0f);
            LONG cnt = 0;
            LONG maxcnt = GetIntArg(enm, 4);
            LONG dmg = GetIntArg(enm, 5);

            ENEMYLISTNODE* node = GameEnmMgr->head;
            DWORD i = 0;

            while(node != NULL) {
                ENEMY* iterEnm = &node->obj->enm;
                if (!(iterEnm->flags & (FLAG_INTANGIBLE | FLAG_NO_HURTBOX))) {
                    /* Hurtbox is actually a square, unlike the hitbox. */
                    /* So we have to check circle-rectangle intersection (annoying) */
                    FLOAT xDist = fabsf(x - iterEnm->pos.x);
                    FLOAT yDist = fabsf(y - iterEnm->pos.y);
                    BOOL intersects;
                    if (xDist > iterEnm->hurtbox.x / 2.0f + rad) intersects = FALSE;
                    else if (yDist > iterEnm->hurtbox.y / 2.0f + rad) intersects = FALSE;

                    else if (xDist <= iterEnm->hurtbox.x / 2.0f) intersects = TRUE;
                    else if (yDist <= iterEnm->hurtbox.y / 2.0f) intersects = TRUE;

                    else {
                        FLOAT distSq = powf(xDist - iterEnm->hurtbox.x / 2.0f, 2.0f) + powf(yDist - iterEnm->hurtbox.y / 2.0f, 2.0f);
                        intersects = distSq <= radSq;
                    }

                    if (intersects) {
                        enmArr[i] = iterEnm;
                        ++i;
                        if (i == ENMARR_LEN) {
                            EclMsg("enmDamageRad: ran out of space for enemies in the preallocated buffer. Why are you using over 2000 real enemies?");
                            return TRUE;
                        }
                    }
                }
                node = node->next;
            }

            std::sort(enmArr.begin(), enmArr.begin() + i, [x, y](ENEMY* enm1, ENEMY* enm2) {
                return
                    powf(enm1->pos.x - x, 2) + powf(enm1->pos.y - y, 2)
                    <
                    powf(enm2->pos.x - x, 2) + powf(enm2->pos.y - y, 2);
            });


            for (DWORD itr = 0; itr < i; ++itr) {
                if (maxcnt > 0 && cnt >= maxcnt)
                    break;
                enmArr[itr]->pendingDmg += dmg;
                ++cnt;
            }

            LONG* ptr = GetIntArgAddr(enm, 0);
            if (ptr != NULL) *ptr = cnt;

            break;
        }
        case INS_ENM_DAMAGE_RECT: {
            FLOAT x = GetFloatArg(enm, 1),
                  y = GetFloatArg(enm, 2),
                  w = GetFloatArg(enm, 3),
                  h = GetFloatArg(enm, 4);

            LONG cnt = 0;
            LONG maxcnt = GetIntArg(enm, 5);
            LONG dmg = GetIntArg(enm, 6);

            ENEMYLISTNODE* node = GameEnmMgr->head;
            DWORD i = 0;

            while (node != NULL) {
                ENEMY* iterEnm = &node->obj->enm;
                if (
                    !(iterEnm->flags & (FLAG_INTANGIBLE | FLAG_NO_HURTBOX)) &&
                    x - w / 2.0f < iterEnm->pos.x + iterEnm->hurtbox.x / 2.0f &&
                    x + w / 2.0f > iterEnm->pos.x - iterEnm->hurtbox.x / 2.0f &&
                    y - h / 2.0f < iterEnm->pos.y + iterEnm->hurtbox.y / 2.0f &&
                    y + h / 2.0f > iterEnm->pos.y - iterEnm->hurtbox.y / 2.0f
                ) {
                    enmArr[i] = iterEnm;
                    ++i;
                    if (i == ENMARR_LEN) {
                        EclMsg("enmDamageRect: ran out of space for enemies in the preallocated buffer. Why are you using over 2000 real enemies?");
                        return TRUE;
                    }
                }
                node = node->next;
            }

            std::sort(enmArr.begin(), enmArr.begin() + i, [x, y](ENEMY* enm1, ENEMY* enm2) {
                return
                    powf(enm1->pos.x - x, 2) + powf(enm1->pos.y - y, 2)
                    <
                    powf(enm2->pos.x - x, 2) + powf(enm2->pos.y - y, 2);
            });

            for (DWORD itr = 0; itr < i; ++itr) {
                if (maxcnt > 0 && cnt >= maxcnt)
                    break;
                enmArr[itr]->pendingDmg += dmg;
                ++cnt;
            }

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