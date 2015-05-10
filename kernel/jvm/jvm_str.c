/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

//HelloX Porting Code.
#include <stdafx.h>
#include <kapi.h>
#include <io.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jam.h"
#include "hash.h"
#include "symbol.h"

#define HASHTABSZE 1<<10
#define HASH(ptr) stringHash(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) (ptr1 == ptr2) || \
                  ((hash1 == hash2) && stringComp(ptr1, ptr2))
#define PREPARE(ptr) ptr
#define SCAVENGE(ptr) FALSE
#define FOUND(ptr1, ptr2) ptr2

static Class *string_class;
static int count_offset; 
static int value_offset;
static int offset_offset;

static HashTable hash_table;

int stringHash(Object *ptr) {
    int len = INST_DATA(ptr, int, count_offset);
    int offset = INST_DATA(ptr, int, offset_offset);
    Object *array = INST_DATA(ptr, Object*, value_offset); 
    unsigned short *dpntr = ARRAY_DATA(array, unsigned short) + offset;
    int hash = 0;

    for(; len > 0; len--)
        hash = hash * 37 + *dpntr++;

    return hash;
}

int stringComp(Object *ptr, Object *ptr2) {
    int len = INST_DATA(ptr, int, count_offset);
    int len2 = INST_DATA(ptr2, int, count_offset);

    if(len == len2) {
        Object *array = INST_DATA(ptr, Object*, value_offset);
        Object *array2 = INST_DATA(ptr2, Object*, value_offset);
        int offset = INST_DATA(ptr, int, offset_offset);
        int offset2 = INST_DATA(ptr2, int, offset_offset);
        unsigned short *src = ARRAY_DATA(array, unsigned short) + offset;
        unsigned short *dst = ARRAY_DATA(array2, unsigned short) + offset2;

        for(; (len > 0) && (*src++ == *dst++); len--);

        if(len == 0)
            return TRUE;
    }

    return FALSE;
}

Object *createString(char *utf8) {
    int len = utf8Len(utf8);
    unsigned short *data;
    Object *array;
    Object *ob;

    if((array = allocTypeArray(T_CHAR, len)) == NULL ||
       (ob = allocObject(string_class)) == NULL)
        return NULL;

    data = ARRAY_DATA(array, unsigned short);
    convertUtf8(utf8, data);

    INST_DATA(ob, int, count_offset) = len; 
    INST_DATA(ob, Object*, value_offset) = array; 

    return ob;
}

Object *findInternedString(Object *string) {
    Object *interned;

    /* Add if absent, no scavenge, locked */
    findHashEntry(hash_table, string, interned, TRUE, FALSE, TRUE);

    return interned;
}
 
#define ITERATE(ptr)          \
    if(!isMarked(*ptr)) {     \
        *ptr = NULL;          \
        unmarked++;           \
    }

void freeInternedStrings() {
    int unmarked = 0;

    hashIterateP(hash_table);

    if(unmarked) {
        int size;

        /* Update count to remaining number of strings */
        hash_table.hash_count -= unmarked;

        /* Calculate nearest multiple of 2 larger than count */
        for(size = 1; size < hash_table.hash_count; size <<= 1);

        /* Ensure new table is less than 2/3 full */
        size = hash_table.hash_count*3 > size*2 ? size<< 1 : size;

        resizeHash(&hash_table, size);
    }
}

#undef ITERATE
#define ITERATE(ptr) threadReference((Object**)ptr);

void threadInternedStrings() {
    hashIterateP(hash_table);
}

char *String2Buff0(Object *string, char *buff, int len) {
    int offset = INST_DATA(string, int, offset_offset);
    Object *array = INST_DATA(string, Object*, value_offset);
    unsigned short *str = ARRAY_DATA(array, unsigned short) + offset;
    char *pntr;

    for(pntr = buff; len > 0; len--)
        *pntr++ = *str++;

    *pntr = '\0';
    return buff;
}

char *String2Buff(Object *string, char *buff, int buff_len) {
    int str_len = INST_DATA(string, int, count_offset);
    int len = buff_len-1 < str_len ? buff_len-1 : str_len;

    return String2Buff0(string, buff, len);
}

char *String2Cstr(Object *string) {
    int len = INST_DATA(string, int, count_offset);
    char *buff = sysMalloc(len + 1);

    return String2Buff0(string, buff, len);
}

void initialiseString() {
    FieldBlock *count = NULL, *value = NULL, *offset = NULL;

    string_class = findSystemClass0(SYMBOL(java_lang_String));
    registerStaticClassRef(&string_class);

    if(string_class != NULL) {
        count = findField(string_class, SYMBOL(count), SYMBOL(I));
        value = findField(string_class, SYMBOL(value), SYMBOL(array_C));
        offset = findField(string_class, SYMBOL(offset), SYMBOL(I));
    }

    /* findField doesn't throw an exception... */
    if((count == NULL) || (value == NULL) || (offset == NULL)) {
        jam_fprintf(stderr, "Error initialising VM (initialiseString)\n");
        exitVM(1);
    }

    count_offset = count->u.offset;
    value_offset = value->u.offset;
    offset_offset = offset->u.offset;

    /* Init hash table and create lock */
    initHashTable(hash_table, HASHTABSZE, TRUE);
}

#ifndef NO_JNI
/* Functions used by JNI */

Object *createStringFromUnicode(unsigned short *unicode, int len) {
    Object *array = allocTypeArray(T_CHAR, len);
    Object *ob = allocObject(string_class);

    if(array != NULL && ob != NULL) {
        unsigned short *data = ARRAY_DATA(array, unsigned short);
        memcpy(data, unicode, len * sizeof(unsigned short));

        INST_DATA(ob, int, count_offset) = len; 
        INST_DATA(ob, Object*, value_offset) = array; 
        return ob;
    }
    return NULL;
}

Object *getStringCharsArray(Object *string) {
    return INST_DATA(string, Object*, value_offset);
}

unsigned short *getStringChars(Object *string) {
    Object *array = INST_DATA(string, Object*, value_offset);
    int offset = INST_DATA(string, int, offset_offset);
    return ARRAY_DATA(array, unsigned short) + offset;
}

int getStringLen(Object *string) {
    return INST_DATA(string, int, count_offset);
}

int getStringUtf8Len(Object *string) {
    return utf8CharLen(getStringChars(string), getStringLen(string));
}

char *StringRegion2Utf8(Object *string, int start, int len, char *utf8) {
    return unicode2Utf8(getStringChars(string) + start, len, utf8);
}

char *String2Utf8(Object *string) {
    int len = getStringLen(string);
    unsigned short *unicode = getStringChars(string);
    char *utf8 = sysMalloc(utf8CharLen(unicode, len) + 1);

    return unicode2Utf8(unicode, len, utf8);
}
#endif
