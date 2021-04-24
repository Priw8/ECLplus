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
#include "ins2300.h"
#include "ECLplus.h"

DATASTRUCTLIST structs = { NULL };

static VOID StructRegister(DATASTRUCT* strct) {
    if (structs.head == NULL) {
        structs.head = strct;
    } else {
        structs.head->prev = strct;
        strct->next = structs.head;
        structs.head = strct;
    }
}

static VOID StructRemove(DATASTRUCT* strct) {
    if (structs.head == strct) {
        structs.head = strct->next;
        if (structs.head)
            structs.head->prev = NULL;
    } else {
        strct->prev->next = strct->next;
        if (strct->next)
            strct->next->prev = strct->prev;
    }
}

static DATASTRUCT* StructNew(DATASTRUCT_TYPE type, DWORD owner) {
    DATASTRUCT* strct = new DATASTRUCT;
    if (type == DATASTRUCT_ARRAY) {
        strct->arr = new std::vector<DATAVALUE>;
    } else {
        strct->map = new std::map<LONG, DATAVALUE>;
    }
    strct->type = type;
    strct->owner = owner;
    strct->next = NULL;
    strct->prev = NULL;
    StructRegister(strct);
    return strct;
}

static VOID StructDelete(DATASTRUCT* strct, BOOL deep) {
    if (deep) {
        if (strct->type == DATASTRUCT_ARRAY) {
            for (unsigned int i=0; i<strct->arr->size(); ++i) {
                if ((*strct->arr)[i].type == DATAVALUE_HANDLE) {
                    StructDelete((*strct->arr)[i].handle, TRUE);
                }
            } 
        } else if (strct->type == DATASTRUCT_MAP) {
            for (auto it = strct->map->begin(); it != strct->map->end(); ++it) {
                if (it->second.type == DATAVALUE_HANDLE) {
                    StructDelete(it->second.handle, TRUE);
                }
            }
        }
    }
    if (strct->type == DATASTRUCT_ARRAY) {
        delete strct->arr;
    } else if (strct->type == DATASTRUCT_MAP) {
        delete strct->map;
    }
    StructRemove(strct);
    delete strct;
}

static DATASTRUCT* StructCopy(DATASTRUCT* strct, BOOL deep) {
    DATASTRUCT* copy = new DATASTRUCT;
    memcpy(copy, strct, sizeof(DATASTRUCT));
    if (copy->type == DATASTRUCT_ARRAY) {
        copy->arr = new std::vector<DATAVALUE>;
        copy->arr->resize(strct->arr->size());
        if (deep) {
            for (unsigned int i = 0; i<strct->arr->size(); ++i) {
                (*copy->arr)[i] = (*strct->arr)[i];
                if ((*copy->arr)[i].type == DATAVALUE_HANDLE) {
                    (*copy->arr)[i].handle = StructCopy((*copy->arr)[i].handle, TRUE);
                }
            }
        } else {
            for (unsigned int i = 0; i < strct->arr->size(); ++i) {
                (*copy->arr)[i] = (*strct->arr)[i];
            }
        }
        return copy;
    } else {
        copy->map = new std::map<LONG, DATAVALUE>;
        if (deep) {
            for (auto it = strct->map->begin(); it != strct->map->end(); ++it) {
                (*copy->map)[it->first] = it->second;
                if (it->second.type == DATAVALUE_HANDLE) {
                    (*copy->map)[it->first].handle = StructCopy(it->second.handle, TRUE);
                }
            }
        } else {
            for (auto it = strct->map->begin(); it != strct->map->end(); ++it) {
                (*copy->map)[it->first] = it->second;
            }
      
        }
        return copy;
    }
}

static BOOL StructArrValid(DATASTRUCT* strct) {
    if (strct->type != DATASTRUCT_ARRAY) {
        EclMsg("Invalid data struct access: handle is not an array");
        return FALSE;
    }
    return TRUE;
}

static BOOL StructArrValidIndex(DATASTRUCT* strct, DWORD index) {
    if (!StructArrValid(strct)) {
        return FALSE;
    } else if (strct->arr->size() <= index || index < 0) {
        EclMsg("Invalid data struct access: array index out of bounds");
        return FALSE;
    }
    return TRUE;
}

static BOOL StructMapValid(DATASTRUCT* strct) {
    if (strct->type != DATASTRUCT_MAP) {
        EclMsg("Invalid data struct access: handle is not a map");
        return FALSE;
    }
    return TRUE;
}

static BOOL StructMapValidField(DATASTRUCT* strct, DWORD field) {
    if (!StructMapValid(strct)) {
        return FALSE;
    }
    else if (strct->map->find(field) == strct->map->end()) {
        EclMsg("Invalid data struct access: map does not have the given field");
        return FALSE;
    }
    return TRUE;
}

#define EclStackPushHandle(ctx, ins, strct) EclStackPushInt(ctx, ins, (LONG)strct)
#define GetHandleArg(enm, n) ((DATASTRUCT*)GetIntArg(enm, n))
BOOL ins_2300(ENEMY* enm, INSTR* ins) {
    switch(ins->id) {
        case INS_STRUCT_NEW_ARR: {
            DATASTRUCT* arr = StructNew(DATASTRUCT_ARRAY, enm->id);
            arr->arr->resize(GetIntArg(enm, 0));
            EclStackPushHandle(enm->full->eclCtx, ins, arr);
            break;
        }
        case INS_STRUCT_NEW_MAP:
            EclStackPushHandle(enm->full->eclCtx, ins, StructNew(DATASTRUCT_MAP, enm->id));
            break;
        case INS_STRUCT_COPY:
            EclStackPushHandle(enm->full->eclCtx, ins, StructCopy(GetHandleArg(enm, 0), FALSE));
            break;
        case INS_STRUCT_COPY_DEEP:
            EclStackPushHandle(enm->full->eclCtx, ins, StructCopy(GetHandleArg(enm, 0), TRUE));
            break;
        case INS_STRUCT_DELETE:
            StructDelete(GetHandleArg(enm, 0), FALSE);
            break;
        case INS_STRUCT_DELETE_DEEP:
            StructDelete(GetHandleArg(enm, 0), TRUE);
            break;
        case INS_STRUCT_DELETE_ENM: {
            DWORD id = GetIntArg(enm, 0);
            DATASTRUCT* strct = structs.head;  
            while(strct) {
                DATASTRUCT* nextStrct = strct->next;
                if (strct->owner == id) {
                    StructDelete(strct, FALSE);
                }
                strct = nextStrct;
            }
            break;
        }
        case INS_STRUCT_ARR_SET_INT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            DWORD index = GetIntArg(enm, 1);
            if (StructArrValidIndex(strct, index)) {
                (*strct->arr)[index].S = GetIntArg(enm, 2);
                (*strct->arr)[index].type = DATAVALUE_INT;
            }
            break;
        }
        case INS_STRUCT_ARR_SET_FLOAT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            DWORD index = GetIntArg(enm, 1);
            if (StructArrValidIndex(strct, index)) {
                (*strct->arr)[index].f = GetFloatArg(enm, 2);
                (*strct->arr)[index].type = DATAVALUE_FLOAT;
            }
            break;
        }
        case INS_STRUCT_ARR_SET_HANDLE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            DWORD index = GetIntArg(enm, 1);
            if (StructArrValidIndex(strct, index)) {
                (*strct->arr)[index].handle = GetHandleArg(enm, 2);
                (*strct->arr)[index].type = DATAVALUE_HANDLE;
            }
            break;
        }
        case INS_STRUCT_ARR_APPEND_INT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                DATAVALUE val;
                val.S = GetIntArg(enm, 1);
                val.type = DATAVALUE_INT;
                strct->arr->push_back(val);
            }
            break;
        }
        case INS_STRUCT_ARR_APPEND_FLOAT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                DATAVALUE val;
                val.f = GetFloatArg(enm, 1);
                val.type = DATAVALUE_FLOAT;
                strct->arr->push_back(val);
            }
            break;
        }
        case INS_STRUCT_ARR_APPEND_HANDLE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                DATAVALUE val;
                val.handle = GetHandleArg(enm, 1);
                val.type = DATAVALUE_HANDLE;
                strct->arr->push_back(val);
            }
            break;
        }
        case INS_STRUCT_ARR_PREPEND_INT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                DATAVALUE val;
                val.S = GetIntArg(enm, 1);
                val.type = DATAVALUE_INT;
                strct->arr->insert(strct->arr->begin(), val);
            }
            break;
        }
        case INS_STRUCT_ARR_PREPEND_FLOAT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                DATAVALUE val;
                val.f = GetFloatArg(enm, 1);
                val.type = DATAVALUE_FLOAT;
                strct->arr->insert(strct->arr->begin(), val);
            }
            break;
        }
        case INS_STRUCT_ARR_PREPEND_HANDLE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                DATAVALUE val;
                val.handle = GetHandleArg(enm, 1);
                val.type = DATAVALUE_HANDLE;
                strct->arr->insert(strct->arr->begin(), val);
            }
            break;
        }
        case INS_STRUCT_ARR_SPLICE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            LONG index = GetIntArg(enm, 1);
            LONG cnt = GetIntArg(enm, 2);
            if (StructArrValid(strct)) {
                if (cnt > 0 && index >= 0) {
                    // -1 because it uses the given index before indices after it
                    if ((DWORD)(index + cnt - 1) < strct->arr->size()) {
                        auto begin = strct->arr->begin() + index;
                        auto end = begin + cnt;
                        strct->arr->erase(begin, end);
                    } else {
                        EclMsg("Invalid parameters of dataArrSplice: selected region is not in bounds of the array.");
                    }
                } else {
                    EclMsg("Invalid cnt or index parameter of dataArrSplice");
                }
            }
            break;
        }
        case INS_STRUCT_ARR_RESIZE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                strct->arr->resize(GetIntArg(enm, 1));
            }
            break;
        }
        case INS_STRUCT_MAP_SET_INT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructMapValid(strct)) {
                LONG field = GetIntArg(enm, 1);
                DATAVALUE val;
                val.S = GetIntArg(enm, 2);
                val.type = DATAVALUE_INT;
                (*strct->map)[field] = val;
            }
            break;
        }
        case INS_STRUCT_MAP_SET_FLOAT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructMapValid(strct)) {
                LONG field = GetIntArg(enm, 1);
                DATAVALUE val;
                val.f = GetFloatArg(enm, 2);
                val.type = DATAVALUE_FLOAT;
                (*strct->map)[field] = val;
            }
            break;
        }
        case INS_STRUCT_MAP_SET_HANDLE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructMapValid(strct)) {
                LONG field = GetIntArg(enm, 1);
                DATAVALUE val;
                val.handle = GetHandleArg(enm, 2);
                val.type = DATAVALUE_HANDLE;
                (*strct->map)[field] = val;
            }
            break;
        }
        case INS_STRUCT_MAP_DELETE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            LONG field = GetIntArg(enm, 1);
            if (StructMapValidField(strct, field)) {
                strct->map->erase(field);
            }
            break;
        }

        case INS_STRUCT_TYPE:
            EclStackPushInt(enm->full->eclCtx, ins, GetHandleArg(enm, 0)->type);
            break;
        case INS_STRUCT_ARR_GET_TYPE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            DWORD index = GetIntArg(enm, 1);
            if (StructArrValidIndex(strct, index)) {
                EclStackPushInt(enm->full->eclCtx, ins, (*strct->arr)[index].type);
            }
            else {
                /* To prevent the ECL stack from getting completely hecked by having 1 less value than it should. */
                EclStackPushInt(enm->full->eclCtx, ins, DATAVALUE_INVALID);
            }
            break;
        }
        case INS_STRUCT_ARR_POP_INT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                if (strct->arr->size() > 0) {
                    EclStackPushInt(enm->full->eclCtx, ins, strct->arr->back().S);
                    strct->arr->pop_back();
                } else {
                    EclMsg("Invalid data struct access: can't pop from array of size=0");
                    EclStackPushInt(enm->full->eclCtx, ins, 0);
                }
            }
            else {
                EclStackPushInt(enm->full->eclCtx, ins, 0);
            }
            break;
        }
        case INS_STRUCT_ARR_POP_FLOAT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                if (strct->arr->size() > 0) {
                    EclStackPushFloat(enm->full->eclCtx, ins, strct->arr->back().f);
                    strct->arr->pop_back();
                }
                else {
                    EclMsg("Invalid data struct access: can't pop from array of size=0");
                    EclStackPushFloat(enm->full->eclCtx, ins, 0.0f);
                }
            }
            else {
                EclStackPushFloat(enm->full->eclCtx, ins, 0.0f);
            }
            break;
        }
        case INS_STRUCT_ARR_POP_HANDLE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                if (strct->arr->size() > 0) {
                    EclStackPushHandle(enm->full->eclCtx, ins, strct->arr->back().handle);
                    strct->arr->pop_back();
                }
                else {
                    EclMsg("Invalid data struct access: can't pop from array of size=0");
                    EclStackPushHandle(enm->full->eclCtx, ins, NULL);
                }
            }
            else {
                EclStackPushHandle(enm->full->eclCtx, ins, NULL);
            }
            break;
        }
        case INS_STRUCT_ARR_GET_INT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            DWORD index = GetIntArg(enm, 1);
            if (StructArrValidIndex(strct, index)) {
                EclStackPushInt(enm->full->eclCtx, ins, (*strct->arr)[index].S);
            } else {
                EclStackPushInt(enm->full->eclCtx, ins, 0);
            }
            break;
        }
        case INS_STRUCT_ARR_GET_FLOAT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            DWORD index = GetIntArg(enm, 1);
            if (StructArrValidIndex(strct, index)) {
                EclStackPushFloat(enm->full->eclCtx, ins, (*strct->arr)[index].f);
            }
            else {
                EclStackPushFloat(enm->full->eclCtx, ins, 0.0f);
            }
            break;
        }
        case INS_STRUCT_ARR_GET_HANDLE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            DWORD index = GetIntArg(enm, 1);
            if (StructArrValidIndex(strct, index)) {
                EclStackPushHandle(enm->full->eclCtx, ins, (*strct->arr)[index].handle);
            }
            else {
                EclStackPushHandle(enm->full->eclCtx, ins, NULL);
            }
            break;
        }
        case INS_STRUCT_ARR_GET_LENGTH: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructArrValid(strct)) {
                EclStackPushInt(enm->full->eclCtx, ins, strct->arr->size());
            } else {
                EclStackPushInt(enm->full->eclCtx, ins, 0);
            }
            break;
        }
        case INS_STRUCT_MAP_EXISTS: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            if (StructMapValid(strct)) {
                if (strct->map->find(GetIntArg(enm, 1)) != strct->map->end()) {
                    EclStackPushInt(enm->full->eclCtx, ins, 1);
                } else {
                    EclStackPushInt(enm->full->eclCtx, ins, 0);
                }
            } else {
                EclStackPushInt(enm->full->eclCtx, ins, 0);
            }
            break;
        }
        case INS_STRUCT_MAP_GET_INT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            LONG field = GetIntArg(enm, 1);
            if (StructMapValidField(strct, field)) {
                EclStackPushInt(enm->full->eclCtx, ins, (*strct->map)[field].S);
            } else {
                EclStackPushInt(enm->full->eclCtx, ins, 0);
            }
            break;
        }
        case INS_STRUCT_MAP_GET_FLOAT: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            LONG field = GetIntArg(enm, 1);
            if (StructMapValidField(strct, field)) {
                EclStackPushFloat(enm->full->eclCtx, ins, (*strct->map)[field].f);
            } else {
                EclStackPushFloat(enm->full->eclCtx, ins, 0.0f);
            }
            break;
        }
        case INS_STRUCT_MAP_GET_HANDLE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            LONG field = GetIntArg(enm, 1);
            if (StructMapValidField(strct, field)) {
                EclStackPushHandle(enm->full->eclCtx, ins, (*strct->map)[field].handle);
            }
            else {
                EclStackPushHandle(enm->full->eclCtx, ins, NULL);
            }
            break;
        }
        case INS_STRUCT_MAP_GET_TYPE: {
            DATASTRUCT* strct = GetHandleArg(enm, 0);
            LONG field = GetIntArg(enm, 1);
            if (StructMapValidField(strct, field)) {
                EclStackPushInt(enm->full->eclCtx, ins, (*strct->map)[field].type);
            }
            else {
                EclStackPushInt(enm->full->eclCtx, ins, DATAVALUE_INVALID);
            }
            break;
        }
        case INS_STRUCT_DELETE_ALL: {
            DATASTRUCT* strct = structs.head;
            while (strct) {
                DATASTRUCT* nextStrct = strct->next;
                StructDelete(strct, FALSE);
                strct = nextStrct;
            }
            break;
        }
        default:
            return FALSE;
    }
    return TRUE;
}