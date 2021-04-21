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

#pragma once
#include "pch.h"
#include "ECLplus.h"

enum INS2300 {
    INS_STRUCT_NEW_ARR = 2300,
    INS_STRUCT_NEW_MAP,
    INS_STRUCT_COPY,
    INS_STRUCT_COPY_DEEP,
    INS_STRUCT_DELETE,
    INS_STRUCT_DELETE_DEEP,
    INS_STRUCT_DELETE_ENM,
    INS_STRUCT_ARR_SET_INT,
    INS_STRUCT_ARR_SET_FLOAT,
    INS_STRUCT_ARR_SET_HANDLE,
    INS_STRUCT_ARR_APPEND_INT,
    INS_STRUCT_ARR_APPEND_FLOAT,
    INS_STRUCT_ARR_APPEND_HANDLE,
    INS_STRUCT_ARR_PREPEND_INT,
    INS_STRUCT_ARR_PREPEND_FLOAT,
    INS_STRUCT_ARR_PREPEND_HANDLE,
    INS_STRUCT_ARR_SPLICE,
    INS_STRUCT_ARR_RESIZE,
    INS_STRUCT_MAP_SET_INT,
    INS_STRUCT_MAP_SET_FLOAT,
    INS_STRUCT_MAP_SET_HANDLE,
    INS_STRUCT_MAP_DELETE,
    INS_STRUCT_TYPE,
    INS_STRUCT_ARR_GET_TYPE,
    INS_STRUCT_ARR_POP_INT,
    INS_STRUCT_ARR_POP_FLOAT,
    INS_STRUCT_ARR_POP_HANDLE,
    INS_STRUCT_ARR_GET_INT,
    INS_STRUCT_ARR_GET_FLOAT,
    INS_STRUCT_ARR_GET_HANDLE,
    INS_STRUCT_ARR_GET_LENGTH,
    INS_STRUCT_MAP_EXISTS,
    INS_STRUCT_MAP_GET_TYPE,
    INS_STRUCT_MAP_GET_INT,
    INS_STRUCT_MAP_GET_FLOAT,
    INS_STRUCT_MAP_GET_HANDLE,
};

enum DATASTRUCT_TYPE {
    DATASTRUCT_ARRAY,
    DATASTRUCT_MAP
};
typedef struct DATAVALUE DATAVALUE;
typedef struct DATASTRUCT DATASTRUCT;
typedef struct DATASTRUCT {
    union {
        std::vector<DATAVALUE>* arr;
        std::map<LONG, DATAVALUE>* map;
    };
    DATASTRUCT_TYPE type;
    DWORD owner;

    DATASTRUCT* next;
    DATASTRUCT* prev;
} DATASTRUCT;

enum DATAVALUE_TYPE {
    DATAVALUE_INT,
    DATAVALUE_FLOAT,
    DATAVALUE_HANDLE
};
typedef struct DATAVALUE {
    union {
        LONG S;
        FLOAT f;
        DATASTRUCT* handle;
    };
    DATAVALUE_TYPE type;
} DATAVALUE;

typedef struct DATASTRUCTLIST {
    DATASTRUCT* head;
} DATASTRUCTLIST;

BOOL ins_2300(ENEMY* enm, INSTR* ins);
