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

//HelloX Porting code.
#include <stdafx.h>
#include <kapi.h>
#include <io.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jam.h"
#include "frame.h"
#include "lock.h"
#include "class.h"
#include "symbol.h"
#include "excep.h"
#include "reflect.h"
#include "properties.h"

static char inited = FALSE;

static Class *class_array_class, *cons_array_class, *cons_reflect_class;
static Class *method_array_class, *method_reflect_class, *field_array_class;
static Class *field_reflect_class, *vmcons_reflect_class;
static Class *vmmethod_reflect_class, *vmfield_reflect_class;

int vm_cons_slot_offset, vm_cons_class_offset, vm_cons_param_offset;
int vm_cons_cons_offset, vm_mthd_slot_offset, vm_mthd_class_offset;
int vm_mthd_ret_offset, vm_mthd_param_offset, vm_mthd_m_offset;
int vm_fld_slot_offset, vm_fld_class_offset, vm_fld_type_offset;
int vm_fld_f_offset, cons_cons_offset, mthd_m_offset;
int fld_f_offset, acc_flag_offset;

static int initReflection() {
    Class *cls_ary_cls, *cons_ary_cls, *cons_ref_cls, *mthd_ary_cls;
    Class *mthd_ref_cls, *fld_ary_cls, *fld_ref_cls, *vm_cons_cls;
    Class *vm_mthd_cls, *vm_fld_cls;

    FieldBlock *vm_cons_slot_fb, *vm_cons_class_fb, *vm_cons_param_fb;
    FieldBlock *vm_cons_cons_fb, *vm_mthd_slot_fb, *vm_mthd_class_fb;
    FieldBlock *vm_mthd_ret_fb, *vm_mthd_param_fb, *vm_mthd_m_fb;
    FieldBlock *vm_fld_slot_fb, *vm_fld_class_fb, *vm_fld_type_fb;
    FieldBlock *vm_fld_f_fb, *cons_cons_fb, *mthd_m_fb, *fld_f_fb;
    FieldBlock *acc_flag_fb;

    cls_ary_cls = findArrayClass(SYMBOL(array_java_lang_Class));
    cons_ary_cls = findArrayClass(SYMBOL(array_java_lang_reflect_Constructor));
    cons_ref_cls = findSystemClass(SYMBOL(java_lang_reflect_Constructor));
    vm_cons_cls = findSystemClass(SYMBOL(java_lang_reflect_VMConstructor));
    mthd_ary_cls = findArrayClass(SYMBOL(array_java_lang_reflect_Method));
    mthd_ref_cls = findSystemClass(SYMBOL(java_lang_reflect_Method));
    vm_mthd_cls = findSystemClass(SYMBOL(java_lang_reflect_VMMethod));
    fld_ary_cls = findArrayClass(SYMBOL(array_java_lang_reflect_Field));
    fld_ref_cls = findSystemClass(SYMBOL(java_lang_reflect_Field));
    vm_fld_cls = findSystemClass(SYMBOL(java_lang_reflect_VMField));

    if(!cls_ary_cls || !cons_ary_cls || !cons_ref_cls || !mthd_ary_cls
                    || !mthd_ref_cls || !fld_ary_cls  || !fld_ref_cls
                    || !vm_cons_cls  || !vm_mthd_cls  || !vm_fld_cls)
        return FALSE;

    vm_cons_slot_fb  = findField(vm_cons_cls, SYMBOL(slot), SYMBOL(I));
    vm_cons_class_fb = findField(vm_cons_cls, SYMBOL(clazz),
                                              SYMBOL(sig_java_lang_Class));
    vm_cons_param_fb = findField(vm_cons_cls, SYMBOL(parameterTypes),
                                              SYMBOL(array_java_lang_Class));
    vm_cons_cons_fb  = findField(vm_cons_cls, SYMBOL(cons),
                                              SYMBOL(sig_java_lang_reflect_Constructor));

    vm_mthd_slot_fb  = findField(vm_mthd_cls, SYMBOL(slot), SYMBOL(I));
    vm_mthd_class_fb = findField(vm_mthd_cls, SYMBOL(clazz),
                                              SYMBOL(sig_java_lang_Class));
    vm_mthd_ret_fb   = findField(vm_mthd_cls, SYMBOL(returnType),
                                              SYMBOL(sig_java_lang_Class));
    vm_mthd_param_fb = findField(vm_mthd_cls, SYMBOL(parameterTypes),
                                              SYMBOL(array_java_lang_Class));
    vm_mthd_m_fb     = findField(vm_mthd_cls, SYMBOL(m),
                                              SYMBOL(sig_java_lang_reflect_Method));

    vm_fld_slot_fb   = findField(vm_fld_cls,  SYMBOL(slot), SYMBOL(I));
    vm_fld_class_fb  = findField(vm_fld_cls,  SYMBOL(clazz),
                                              SYMBOL(sig_java_lang_Class));
    vm_fld_type_fb   = findField(vm_fld_cls,  SYMBOL(type),
                                              SYMBOL(sig_java_lang_Class));
    vm_fld_f_fb      = findField(vm_fld_cls,  SYMBOL(f),
                                              SYMBOL(sig_java_lang_reflect_Field));

    cons_cons_fb  = findField(cons_ref_cls, SYMBOL(cons),
                                            SYMBOL(sig_java_lang_reflect_VMConstructor));
    mthd_m_fb     = findField(mthd_ref_cls, SYMBOL(m),
                                            SYMBOL(sig_java_lang_reflect_VMMethod));
    fld_f_fb      = findField(fld_ref_cls,  SYMBOL(f),
                                            SYMBOL(sig_java_lang_reflect_VMField));

    acc_flag_fb = lookupField(cons_ref_cls, SYMBOL(flag), SYMBOL(Z));

    if(!vm_cons_slot_fb   || !vm_cons_class_fb || !vm_cons_param_fb ||
         !vm_cons_cons_fb || !vm_mthd_slot_fb  || !vm_mthd_class_fb ||
         !vm_mthd_ret_fb  || !vm_mthd_m_fb     || !vm_mthd_param_fb ||
         !vm_fld_slot_fb  || !vm_fld_class_fb  || !vm_fld_type_fb   ||
         !vm_fld_f_fb     || !cons_cons_fb     || !mthd_m_fb        ||
         !fld_f_fb        || !acc_flag_fb) {

        /* Find Field/Method doesn't throw an exception... */
        signalException(java_lang_InternalError,
                        "Expected reflection field doesn't exist");
        return FALSE;
    }

    vm_cons_slot_offset = vm_cons_slot_fb->u.offset; 
    vm_cons_class_offset = vm_cons_class_fb->u.offset; 
    vm_cons_param_offset = vm_cons_param_fb->u.offset; 
    vm_cons_cons_offset = vm_cons_cons_fb->u.offset; 
    vm_mthd_slot_offset = vm_mthd_slot_fb->u.offset; 
    vm_mthd_class_offset = vm_mthd_class_fb->u.offset; 
    vm_mthd_ret_offset = vm_mthd_ret_fb->u.offset; 
    vm_mthd_param_offset = vm_mthd_param_fb->u.offset; 
    vm_mthd_m_offset = vm_mthd_m_fb->u.offset; 
    vm_fld_slot_offset = vm_fld_slot_fb->u.offset; 
    vm_fld_class_offset = vm_fld_class_fb->u.offset; 
    vm_fld_type_offset = vm_fld_type_fb->u.offset; 
    vm_fld_f_offset = vm_fld_f_fb->u.offset; 
    cons_cons_offset = cons_cons_fb->u.offset; 
    mthd_m_offset = mthd_m_fb->u.offset; 
    fld_f_offset = fld_f_fb->u.offset; 
    acc_flag_offset = acc_flag_fb->u.offset;

    registerStaticClassRefLocked(&class_array_class, cls_ary_cls);
    registerStaticClassRefLocked(&cons_array_class, cons_ary_cls);
    registerStaticClassRefLocked(&method_array_class, mthd_ary_cls);
    registerStaticClassRefLocked(&field_array_class, fld_ary_cls);
    registerStaticClassRefLocked(&cons_reflect_class, cons_ref_cls);
    registerStaticClassRefLocked(&vmcons_reflect_class, vm_cons_cls);
    registerStaticClassRefLocked(&method_reflect_class, mthd_ref_cls);
    registerStaticClassRefLocked(&vmmethod_reflect_class, vm_mthd_cls);
    registerStaticClassRefLocked(&field_reflect_class, fld_ref_cls);
    registerStaticClassRefLocked(&vmfield_reflect_class, vm_fld_cls);

    return inited = TRUE;
}

Class *convertSigElement2Class(char **sig_pntr, Class *declaring_class) {
    char *sig = *sig_pntr;
    Class *class;

    switch(*sig) {
        case '[': {
            char next;
            while(*++sig == '[');
            if(*sig == 'L')
                while(*++sig != ';');
            next = *++sig;
            *sig = '\0';
            class = findArrayClassFromClass(*sig_pntr, declaring_class);
            *sig = next;
            break;
        }

        case 'L':
            while(*++sig != ';');
            *sig++ = '\0';
            class = findClassFromClass((*sig_pntr)+1, declaring_class);
            break;

        default:
            class = findPrimitiveClass(*sig++);
            break;
    }

    *sig_pntr = sig;
    return class;
}

Object *convertSig2ClassArray(char **sig_pntr, Class *declaring_class) {
    char *sig = *sig_pntr;
    int no_params, i = 0;
    Class **params;
    Object *array;

    for(no_params = 0; *++sig != ')'; no_params++) {
        if(*sig == '[')
            while(*++sig == '[');
        if(*sig == 'L')
            while(*++sig != ';');
    }

    if((array = allocArray(class_array_class, no_params, sizeof(Class*))) == NULL)
        return NULL;

    params = ARRAY_DATA(array, Class*);

    *sig_pntr += 1;
    while(**sig_pntr != ')')
        if((params[i++] = convertSigElement2Class(sig_pntr, declaring_class)) == NULL)
            return NULL;

    return array;
}

Object *getExceptionTypes(MethodBlock *mb) {
    int i;
    Object *array;
    Class **excps;

    if((array = allocArray(class_array_class, mb->throw_table_size, sizeof(Class*))) == NULL)
        return NULL;

    excps = ARRAY_DATA(array, Class*);

    for(i = 0; i < mb->throw_table_size; i++)
        if((excps[i] = resolveClass(mb->class, mb->throw_table[i], FALSE)) == NULL)
            return NULL;

    return array;
}

Object *createConstructorObject(MethodBlock *mb) {
    Object *reflect_ob, *vm_reflect_ob, *classes;
    char *signature, *sig;

    if((reflect_ob = allocObject(cons_reflect_class)) == NULL)
        return NULL;

    if((vm_reflect_ob = allocObject(vmcons_reflect_class)) == NULL)
        return NULL;

    signature = sig = sysMalloc(strlen(mb->type) + 1);
    strcpy(sig, mb->type);

    classes = convertSig2ClassArray(&sig, mb->class);
    sysFree(signature);

    if(classes == NULL)
        return NULL;

    INST_DATA(vm_reflect_ob, Class*, vm_cons_class_offset) = mb->class;
    INST_DATA(vm_reflect_ob, Object*, vm_cons_param_offset) = classes;
    INST_DATA(vm_reflect_ob, int, vm_cons_slot_offset) =
                                     mb - CLASS_CB(mb->class)->methods;

    /* Link the Java-level and VM-level objects together */
    INST_DATA(vm_reflect_ob, Object*, vm_cons_cons_offset) = reflect_ob;
    INST_DATA(reflect_ob, Object*, cons_cons_offset) = vm_reflect_ob;

    return reflect_ob;
}

Object *getClassConstructors(Class *class, int public) {
    ClassBlock *cb = CLASS_CB(class);
    Object *array, **cons;
    int count = 0;
    int i, j;

    if(!inited && !initReflection())
        return NULL;

    for(i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];
        if((mb->name == SYMBOL(object_init)) &&
                      (!public || (mb->access_flags & ACC_PUBLIC)))
            count++;
    }

    if((array = allocArray(cons_array_class, count, sizeof(Object*))) == NULL)
        return NULL;

    cons = ARRAY_DATA(array, Object*);

    for(i = 0, j = 0; j < count; i++) {
        MethodBlock *mb = &cb->methods[i];

        if((mb->name == SYMBOL(object_init)) &&
                     (!public || (mb->access_flags & ACC_PUBLIC)))

            if((cons[j++] = createConstructorObject(mb)) == NULL)
                return NULL;
    }

    return array;
}

Object *createMethodObject(MethodBlock *mb) {
    Object *reflect_ob, *vm_reflect_ob, *classes;
    char *signature, *sig;
    Class *ret;

    if((reflect_ob = allocObject(method_reflect_class)) == NULL)
        return NULL;

    if((vm_reflect_ob = allocObject(vmmethod_reflect_class)) == NULL)
        return NULL;

    signature = sig = sysMalloc(strlen(mb->type) + 1);
    strcpy(sig, mb->type);

    classes = convertSig2ClassArray(&sig, mb->class);

    sig++;
    ret = convertSigElement2Class(&sig, mb->class);
    sysFree(signature);

    if(classes == NULL || ret == NULL)
        return NULL;

    INST_DATA(vm_reflect_ob, Class*, vm_mthd_class_offset) = mb->class;
    INST_DATA(vm_reflect_ob, Object*, vm_mthd_param_offset) = classes;
    INST_DATA(vm_reflect_ob, Class*, vm_mthd_ret_offset) = ret;
    INST_DATA(vm_reflect_ob, int, vm_mthd_slot_offset) =
                                     mb - CLASS_CB(mb->class)->methods;

    /* Link the Java-level and VM-level objects together */
    INST_DATA(vm_reflect_ob, Object*, vm_mthd_m_offset) = reflect_ob;
    INST_DATA(reflect_ob, Object*, mthd_m_offset) = vm_reflect_ob;

    return reflect_ob;
}

Object *getClassMethods(Class *class, int public) {
    ClassBlock *cb = CLASS_CB(class);
    Object *array, **methods;
    int count = 0;
    int i, j;

    if(!inited && !initReflection())
        return NULL;

    for(i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];
        if((mb->name[0] != '<') && (!public || (mb->access_flags & ACC_PUBLIC))
                                && ((mb->access_flags & ACC_MIRANDA) == 0))
            count++;
    }

    if((array = allocArray(method_array_class, count, sizeof(Object*))) == NULL)
        return NULL;

    methods = ARRAY_DATA(array, Object*);

    for(i = 0, j = 0; j < count; i++) {
        MethodBlock *mb = &cb->methods[i];

        if((mb->name[0] != '<') && (!public || (mb->access_flags & ACC_PUBLIC))
                                && ((mb->access_flags & ACC_MIRANDA) == 0))

            if((methods[j++] = createMethodObject(mb)) == NULL)
                return NULL;
    }

    return array;
}

Object *createFieldObject(FieldBlock *fb) {
    Object *reflect_ob, *vm_reflect_ob;
    char *signature, *sig;
    Class *type;

    if((reflect_ob = allocObject(field_reflect_class)) == NULL)
        return NULL;

    if((vm_reflect_ob = allocObject(vmfield_reflect_class)) == NULL)
        return NULL;

    signature = sig = sysMalloc(strlen(fb->type) + 1);
    strcpy(signature, fb->type);

    type = convertSigElement2Class(&sig, fb->class);
    sysFree(signature);

    if(type == NULL)
        return NULL;

    INST_DATA(vm_reflect_ob, Class*, vm_fld_class_offset) = fb->class;
    INST_DATA(vm_reflect_ob, Class*, vm_fld_type_offset) = type;
    INST_DATA(vm_reflect_ob, int, vm_fld_slot_offset) =
                                     fb - CLASS_CB(fb->class)->fields;

    /* Link the Java-level and VM-level objects together */
    INST_DATA(vm_reflect_ob, Object*, vm_fld_f_offset) = reflect_ob;
    INST_DATA(reflect_ob, Object*, fld_f_offset) = vm_reflect_ob;

    return reflect_ob;
}

Object *getClassFields(Class *class, int public) {
    ClassBlock *cb = CLASS_CB(class);
    Object *array, **fields;
    int count = 0;
    int i, j;

    if(!inited && !initReflection())
        return NULL;

    if(!public)
        count = cb->fields_count;
    else
        for(i = 0; i < cb->fields_count; i++)
            if(cb->fields[i].access_flags & ACC_PUBLIC)
                count++;

    if((array = allocArray(field_array_class, count, sizeof(Object*))) == NULL)
        return NULL;

    fields = ARRAY_DATA(array, Object*);

    for(i = 0, j = 0; j < count; i++) {
        FieldBlock *fb = &cb->fields[i];

        if(!public || (fb->access_flags & ACC_PUBLIC))
            if((fields[j++] = createFieldObject(fb)) == NULL)
                return NULL;
    }

    return array;
}

Object *getClassInterfaces(Class *class) {
    ClassBlock *cb = CLASS_CB(class);
    Object *array;

    if(!inited && !initReflection())
        return NULL;

    if((array = allocArray(class_array_class, cb->interfaces_count,
                           sizeof(Class*))) == NULL)
        return NULL;

    memcpy(ARRAY_DATA(array, Class*), cb->interfaces,
           cb->interfaces_count * sizeof(Class*));

    return array;
}

Object *getClassClasses(Class *class, int public) {
    ClassBlock *cb = CLASS_CB(class);
    int i, j, count = 0;
    Class **classes;
    Object *array;

    if(!inited && !initReflection())
        return NULL;

    for(i = 0; i < cb->inner_class_count; i++) {
        Class *iclass;

        if((iclass = resolveClass(class, cb->inner_classes[i], FALSE)) == NULL)
            return NULL;

        if(!public || (CLASS_CB(iclass)->inner_access_flags & ACC_PUBLIC))
            count++;
    }

    if((array = allocArray(class_array_class, count, sizeof(Class*))) == NULL)
        return NULL;

    classes = ARRAY_DATA(array, Class*);

    for(i = 0, j = 0; j < count; i++) {
        Class *iclass = resolveClass(class, cb->inner_classes[i], FALSE);

        if(!public || (CLASS_CB(iclass)->inner_access_flags & ACC_PUBLIC))
            classes[j++] = iclass;
    }

    return array;
}

Class *getDeclaringClass(Class *class) {
    ClassBlock *cb = CLASS_CB(class);

    return cb->declaring_class ? resolveClass(class, cb->declaring_class, FALSE)
                               : NULL;
}

Class *getEnclosingClass(Class *class) {
    ClassBlock *cb = CLASS_CB(class);

    return cb->enclosing_class ? resolveClass(class, cb->enclosing_class, FALSE)
                               : NULL;
}

MethodBlock *getEnclosingMethod(Class *class) {
    Class *enclosing_class = getEnclosingClass(class);

    if(enclosing_class != NULL) {
        ClassBlock *cb = CLASS_CB(class);

        if(cb->enclosing_method) {
            ConstantPool *cp = &cb->constant_pool;
            char *methodname = CP_UTF8(cp, CP_NAME_TYPE_NAME(cp,
                                               cb->enclosing_method));
            char *methodtype = CP_UTF8(cp, CP_NAME_TYPE_TYPE(cp,
                                               cb->enclosing_method));
            MethodBlock *mb = findMethod(enclosing_class, methodname,
                                         methodtype);

            if(mb != NULL)
                return mb;

            /* The "reference implementation" throws an InternalError if a
               method with the name and type cannot be found in the enclosing
               class */
            signalException(java_lang_InternalError,
                            "Enclosing method doesn't exist");
        }
    }

    return NULL;
}

Object *getEnclosingMethodObject(Class *class) {
    MethodBlock *mb = getEnclosingMethod(class);

    if(mb != NULL && mb->name == SYMBOL(object_init))
        return createMethodObject(mb);

    return NULL;
}

Object *getEnclosingConstructorObject(Class *class) {
    MethodBlock *mb = getEnclosingMethod(class);

    if(mb != NULL && mb->name == SYMBOL(object_init))
        return createConstructorObject(mb);

    return NULL;
}

static char anno_inited = FALSE;

static Class *enum_class, *map_class, *anno_inv_class, *obj_array_class;
static Class *anno_array_class, *dbl_anno_array_class;
static MethodBlock *map_init_mb, *map_put_mb, *anno_create_mb, *enum_valueof_mb;

static int initAnnotation() {
    Class *enum_cls, *map_cls, *anno_inv_cls, *obj_ary_cls;
    Class *anno_ary_cls, *dbl_anno_ary_cls;

    enum_cls = findSystemClass("java/lang/Enum");
    map_cls = findSystemClass("java/util/HashMap");
    anno_inv_cls = findSystemClass("sun/reflect/annotation/Annotation"
                                   "InvocationHandler");

    obj_ary_cls = findArrayClass("[Ljava/lang/Object;");
    anno_ary_cls = findArrayClass("[Ljava/lang/annotation/Annotation;");
    dbl_anno_ary_cls = findArrayClass("[[Ljava/lang/annotation/Annotation;");

    if(!enum_cls || !map_cls || !anno_inv_cls || !obj_ary_cls 
                 || !anno_ary_cls || !dbl_anno_ary_cls)
        return FALSE;

    map_init_mb = findMethod(map_cls, SYMBOL(object_init), SYMBOL(___V));
    map_put_mb = findMethod(map_cls, SYMBOL(put),
                            newUtf8("(Ljava/lang/Object;Ljava/lang/Object;)"
                                    "Ljava/lang/Object;"));

    anno_create_mb = findMethod(anno_inv_cls, newUtf8("create"),
                                newUtf8("(Ljava/lang/Class;Ljava/util/Map;)"
                                        "Ljava/lang/annotation/Annotation;"));

    enum_valueof_mb = findMethod(enum_cls, newUtf8("valueOf"),
                                 newUtf8("(Ljava/lang/Class;Ljava/lang/String;)"
                                         "Ljava/lang/Enum;"));

    if(!map_init_mb || !map_put_mb || !anno_create_mb || !enum_valueof_mb) {

        /* FindMethod doesn't throw an exception... */
        signalException(java_lang_InternalError,
                        "Expected field/method doesn't exist");
        return FALSE;
    }

    registerStaticClassRefLocked(&enum_class, enum_cls);
    registerStaticClassRefLocked(&map_class, map_cls);
    registerStaticClassRefLocked(&anno_inv_class, anno_inv_cls);
    registerStaticClassRefLocked(&obj_array_class, obj_ary_cls);
    registerStaticClassRefLocked(&anno_array_class, anno_ary_cls);
    registerStaticClassRefLocked(&dbl_anno_array_class, dbl_anno_ary_cls);

    return anno_inited = TRUE;
}

Class *findClassFromSignature(char *type_name, Class *class) {
    Class *type_class;
    char *name, *pntr;

    name = pntr = sysMalloc(strlen(type_name) + 1);
    strcpy(name, type_name);

    type_class = convertSigElement2Class(&pntr, class);
    sysFree(name);

    return type_class;
}

/* Forward declarations */
Object *createWrapperObject(int prim_type_no, void *pntr, int flags);
Object *parseAnnotation(Class *class, u1 **data_ptr, int *data_len);

Object *parseElementValue(Class *class, u1 **data_ptr, int *data_len) {
    ClassBlock *cb = CLASS_CB(class);
    ConstantPool *cp = &cb->constant_pool;
    char tag;

    READ_U1(tag, *data_ptr, *data_len);

    switch(tag) {
        default: {
            int cp_tag = CONSTANT_Integer;
            int prim_type_no = 0;
            int const_val_idx;

            switch(tag) {
                case 'Z':
                    prim_type_no = PRIM_IDX_BOOLEAN;
                    break;
                case 'B':
                    prim_type_no = PRIM_IDX_BYTE;
                    break;
                case 'C':
                    prim_type_no = PRIM_IDX_CHAR;
                    break;
                case 'S':
                    prim_type_no = PRIM_IDX_SHORT;
                    break;
                case 'I':
                    prim_type_no = PRIM_IDX_INT;
                    break;
                case 'F':
                    cp_tag = CONSTANT_Float;
                    prim_type_no = PRIM_IDX_FLOAT;
                    break;
                case 'J':
                    cp_tag = CONSTANT_Long;
                    prim_type_no = PRIM_IDX_LONG;
                    break;
                case 'D':
                    cp_tag = CONSTANT_Double;
                    prim_type_no = PRIM_IDX_DOUBLE;
                    break;
            }

            READ_TYPE_INDEX(const_val_idx, cp, cp_tag, *data_ptr, *data_len);

            return createWrapperObject(prim_type_no,
                                &CP_INFO(cp, const_val_idx), REF_SRC_OSTACK);
        }

        case 's': {
            int const_str_idx;

            READ_TYPE_INDEX(const_str_idx, cp, CONSTANT_Utf8, *data_ptr,
                            *data_len);

            return createString(CP_UTF8(cp, const_str_idx));
        }

        case 'e': {
            int type_name_idx, const_name_idx;
            Object *const_name, *enum_obj;
            Class *type_class;

            READ_TYPE_INDEX(type_name_idx, cp, CONSTANT_Utf8, *data_ptr, *data_len);
            READ_TYPE_INDEX(const_name_idx, cp, CONSTANT_Utf8, *data_ptr, *data_len);
            type_class = findClassFromSignature(CP_UTF8(cp, type_name_idx), class);
            const_name = createString(CP_UTF8(cp, const_name_idx));

            if(type_class == NULL || const_name == NULL)
                return NULL;

            enum_obj = *(Object**)executeStaticMethod(enum_class, enum_valueof_mb,
                                                      type_class, const_name);
            if(exceptionOccurred())
                return NULL;

            return enum_obj;
        }

        case 'c': {
            int class_info_idx;
            READ_TYPE_INDEX(class_info_idx, cp, CONSTANT_Utf8, *data_ptr, *data_len);
            return findClassFromSignature(CP_UTF8(cp, class_info_idx), class);
        }

        case '@':
            return parseAnnotation(class, data_ptr, data_len);

        case '[': {
            Object *array;
            Object **array_data;
            int i, num_values;

            READ_U2(num_values, *data_ptr, *data_len);
            if((array = allocArray(obj_array_class, num_values, sizeof(Object*))) == NULL)
                return NULL;

            array_data = ARRAY_DATA(array, Object*);

            for(i = 0; i < num_values; i++)
                if((array_data[i] = parseElementValue(class, data_ptr, data_len)) == NULL)
                    return NULL;

            return array;
        }
    }
}

Object *parseAnnotation(Class *class, u1 **data_ptr, int *data_len) {
    ClassBlock *cb = CLASS_CB(class);
    ConstantPool *cp = &cb->constant_pool;
    Object *map, *anno;
    int no_value_pairs;
    Class *type_class;
    int type_idx;
    int i;

    if((map = allocObject(map_class)) == NULL)
        return NULL;

    executeMethod(map, map_init_mb);
    if(exceptionOccurred())
        return NULL;

    READ_TYPE_INDEX(type_idx, cp, CONSTANT_Utf8, *data_ptr, *data_len);
    if((type_class = findClassFromSignature(CP_UTF8(cp, type_idx), class)) == NULL)
        return NULL;

    READ_U2(no_value_pairs, *data_ptr, *data_len);

    for(i = 0; i < no_value_pairs; i++) {
        Object *element_name, *element_value;
        int element_name_idx;

        READ_TYPE_INDEX(element_name_idx, cp, CONSTANT_Utf8, *data_ptr, *data_len);

        element_name = createString(CP_UTF8(cp, element_name_idx));
        element_value = parseElementValue(class, data_ptr, data_len);
        if(element_name == NULL || element_value == NULL)
            return NULL;

        executeMethod(map, map_put_mb, element_name, element_value);
        if(exceptionOccurred())
            return NULL;
    }

    anno = *(Object**)executeStaticMethod(anno_inv_class, anno_create_mb, type_class, map);
    if(exceptionOccurred())
        return NULL;

    return anno;
}

Object *parseAnnotations(Class *class, AnnotationData *annotations) {
    if(!anno_inited && !initAnnotation())
        return NULL;

    if(annotations == NULL)
        return allocArray(anno_array_class, 0, sizeof(Object*));
    else {
        u1 *data_ptr = annotations->data;
        int data_len = annotations->len;
        Object **array_data;
        Object *array;
        int no_annos;
        int i;

        READ_U2(no_annos, data_ptr, data_len);
        if((array = allocArray(anno_array_class, no_annos, sizeof(Object*))) == NULL)
            return NULL;

        array_data = ARRAY_DATA(array, Object*);

        for(i = 0; i < no_annos; i++)
            if((array_data[i] = parseAnnotation(class, &data_ptr, &data_len)) == NULL)
                return NULL;

        return array;
    }
}

Object *getClassAnnotations(Class *class) {
    return parseAnnotations(class, CLASS_CB(class)->annotations);
}

Object *getFieldAnnotations(FieldBlock *fb) {
    return parseAnnotations(fb->class, fb->annotations);
}

Object *getMethodAnnotations(MethodBlock *mb) {
    return parseAnnotations(mb->class, mb->annotations == NULL ?
                                NULL : mb->annotations->annotations);
}

Object *getMethodParameterAnnotations(MethodBlock *mb) {
    if(!anno_inited && !initAnnotation())
        return NULL;

    if(mb->annotations == NULL || mb->annotations->parameters == NULL)
        return allocArray(dbl_anno_array_class, 0, sizeof(Object*));
    else {
        u1 *data_ptr = mb->annotations->parameters->data;
        int data_len = mb->annotations->parameters->len;
        Object **outer_array_data;
        Object *outer_array;
        int no_params, i;

        READ_U1(no_params, data_ptr, data_len);
        if((outer_array = allocArray(dbl_anno_array_class, no_params, sizeof(Object*))) == NULL)
            return NULL;

        outer_array_data = ARRAY_DATA(outer_array, Object*);

        for(i = 0; i < no_params; i++) {
            Object **inner_array_data;
            Object *inner_array;
            int no_annos, j;

            READ_U2(no_annos, data_ptr, data_len);
            if((inner_array = allocArray(anno_array_class, no_annos, sizeof(Object*))) == NULL)
                return NULL;

            inner_array_data = ARRAY_DATA(inner_array, Object*);

            for(j = 0; j < no_annos; j++)
                if((inner_array_data[j] = parseAnnotation(mb->class, &data_ptr, &data_len)) == NULL)
                    return NULL;

            outer_array_data[i] = inner_array;
        }
        return outer_array;
    }
}

Object *getMethodDefaultValue(MethodBlock *mb) {
    if(!anno_inited && !initAnnotation())
        return NULL;

    if(mb->annotations == NULL || mb->annotations->dft_val == NULL)
        return NULL;
    else {
        u1 *data = mb->annotations->dft_val->data;
        int len = mb->annotations->dft_val->len;

        return parseElementValue(mb->class, &data, &len);
    }
}

int getWrapperPrimTypeIndex(Object *arg) {
    if(arg != NULL)  {
        ClassBlock *cb = CLASS_CB(arg->class);

        if(cb->name == SYMBOL(java_lang_Boolean))
            return PRIM_IDX_BOOLEAN;

        if(cb->name == SYMBOL(java_lang_Character))
            return PRIM_IDX_CHAR;

        if(cb->super_name == SYMBOL(java_lang_Number)) {

            if(cb->name == SYMBOL(java_lang_Byte))
                return PRIM_IDX_BYTE;

            if(cb->name == SYMBOL(java_lang_Short))
                return PRIM_IDX_SHORT;

            if(cb->name == SYMBOL(java_lang_Integer))
                return PRIM_IDX_INT;

            if(cb->name == SYMBOL(java_lang_Float))
                return PRIM_IDX_FLOAT;

            if(cb->name == SYMBOL(java_lang_Long))
                return PRIM_IDX_LONG;

            if(cb->name == SYMBOL(java_lang_Double))
                return PRIM_IDX_DOUBLE;
        }
    }

    return PRIM_IDX_VOID;
}

Object *createWrapperObject(int prim_type_no, void *pntr, int flags) {
    static char *wrapper_names[] = {"java/lang/Boolean",
                                    "java/lang/Byte",
                                    "java/lang/Character",
                                    "java/lang/Short",
                                    "java/lang/Integer",
                                    "java/lang/Float",
                                    "java/lang/Long",
                                    "java/lang/Double"};
    Object *wrapper = NULL;

    if(prim_type_no > PRIM_IDX_VOID) {
        Class *wrapper_class;

        if((wrapper_class = findSystemClass(wrapper_names[prim_type_no - 1]))
                  && (wrapper = allocObject(wrapper_class)) != NULL) {
            if(prim_type_no > PRIM_IDX_FLOAT)
                INST_BASE(wrapper, u8)[0] = *(u8*)pntr;
            else
                if(flags == REF_SRC_FIELD)
                    INST_BASE(wrapper, u4)[0] = *(u4*)pntr;
                else
                    INST_BASE(wrapper, u4)[0] = *(uintptr_t*)pntr;
        }
    }

    return wrapper;
}

Object *getReflectReturnObject(Class *type, void *pntr, int flags) {
    ClassBlock *type_cb = CLASS_CB(type);

    if(IS_PRIMITIVE(type_cb))
        return createWrapperObject(type_cb->state - CLASS_PRIM, pntr, flags);

    return *(Object**)pntr;
}

//For debugging.
static int u4_o2f(void* src,void* dest) /* ostack -> field */
{
    *(u4*)dest = *(uintptr_t*)src;
    return 1;
}

static int u4_f2o(void* src,void* dest) /* field -> ostack */
{
    *(uintptr_t*)dest = *(u4*)src;
    return 1;
}

static int u4_f2f(void* src,void* dest) /* field -> field */
{
    *(u4*)dest = *(u4*)src;
    return 1;
}

static int u8__(void* src,void* dest)
{
    *(u8*)dest = *(u8*)src;
    return 2;
}

static int i2f_o2f(void* src,void* dest) /* ostack -> field */
{
    *(float*)dest = (float)(int)*(uintptr_t*)src;
    return 1;
}

static int i2f_f2o(void* src,void* dest) /* field -> ostack */
{
    *((float*)dest + IS_BE64) = (float)*(int*)src;
    return 1;
}

static int i2f_f2f(void* src,void* dest) /* field -> field */
{
    *(float*)dest = (float)*(int*)src;
    return 1;
}

static int i2d_so(void* src,void* dest) /* src ostack */
{
    *(double*)dest = (double)(int)*(uintptr_t*)src;
    return 2;
}

static int i2d_sf(void* src,void* dest) /* src field */
{
    *(double*)dest = (double)*(int*)src;
    return 2;
}

static int i2j_so(void* src,void* dest) /* src ostack */
{
    *(long long*)dest = (long long)(int)*(uintptr_t*)src;
    return 2;
}

static int i2j_sf(void* src,void* dest) /* src field */
{
    *(long long*)dest = (long long)*(int*)src;
    return 2;
}

static int j2f_do(void* src,void* dest) /* dst ostack */
{
    *((float*)dest + IS_BE64) = (float)*(long long*)src;
    return 1;
}

static int j2f_df(void* src,void* dest) /* dst field */
{
    *(float*)dest = (float)*(long long*)src;
    return 1;
}

static int j2d(void* src,void* dest)
{
    *(double*)dest = (double)*(long long*)src;
    return 2;
}

static int f2d_so(void* src,void* dest) /* src ostack */
{
    *(double*)dest = (double)*((float*)src + IS_BE64);
    return 2;
}

static int f2d_sf(void* src,void* dest) /* src field */
{
    *(double*)dest = (double)*(float*)src;
    return 2;
}

static int illegal_arg(void* src,void* dest)
{
    return 0;
}

typedef int (*__CONVERT_HANDLER)(void*,void*);

int widenPrimitiveValue(int src_idx, int dest_idx, void *src, void *dest,
                        int flags) {

#define err 0
#define U4  1
#define U8  2
#define I2F 3
#define I2D 4
#define I2J 5
#define J2F 6
#define J2D 7
#define F2D 8

    static char conv_table[9][8] = {
        /*  bool byte char shrt int  flt long  dbl             */
           {err, err, err, err, err, err, err, err},  /* !prim */
           {U4,  err, err, err, err, err, err, err},  /* bool  */
           {err, U4,  err, U4,  U4,  I2F, I2J, I2D},  /* byte  */
           {err, err, U4,  err, U4,  I2F, I2J, I2D},  /* char  */
           {err, err, err, U4,  U4,  I2F, I2J, I2D},  /* short */
           {err, err, err, err, U4,  I2F, I2J, I2D},  /* int   */
           {err, err, err, err, err, U4,  err, F2D},  /* float */
           {err, err, err, err, err, J2F, U8,  J2D},  /* long  */
           {err, err, err, err, err, err, err, U8 }   /* dbl   */
    };

    static __CONVERT_HANDLER handlers[3][9] = {
         /* field -> field */
         {illegal_arg, u4_f2f, u8__, i2f_f2f, i2d_sf,
                         i2j_sf, j2f_df, j2d, f2d_sf},
         /* ostack -> field */
         {illegal_arg, u4_o2f, u8__, i2f_o2f, i2d_so,
                         i2j_so, j2f_df, j2d, f2d_so},
         /* field -> ostack */
         {illegal_arg, u4_f2o, u8__, i2f_f2o, i2d_sf,
                         i2j_sf, j2f_do, j2d, f2d_sf}
    };

    int handler = conv_table[src_idx][dest_idx - 1];
    return handlers[flags][handler](src,dest);
}

int unwrapAndWidenObject(Class *type, Object *arg, void *pntr, int flags) {
    ClassBlock *type_cb = CLASS_CB(type);

    if(IS_PRIMITIVE(type_cb)) {
        int formal_idx = getPrimTypeIndex(type_cb);
        int actual_idx = getWrapperPrimTypeIndex(arg);
        void *data = INST_BASE(arg, void);

        return widenPrimitiveValue(actual_idx, formal_idx, data, pntr,
                                   flags | REF_SRC_FIELD);
    }

    if((arg == NULL) || isInstanceOf(type, arg->class)) {
        *(uintptr_t*)pntr = (uintptr_t)arg;
        return 1;
    }

    return 0;
}

Object *invoke(Object *ob, MethodBlock *mb, Object *arg_array,
                Object *param_types) {

    Object **args = ARRAY_DATA(arg_array, Object*);
    Class **types = ARRAY_DATA(param_types, Class*);

    int types_len = ARRAY_LEN(param_types);
    int args_len = arg_array ? ARRAY_LEN(arg_array) : 0;

    ExecEnv *ee = getExecEnv();
    uintptr_t *sp;
    Object *excep;
    void *ret;
    int i;

    if(args_len != types_len) {
        signalException(java_lang_IllegalArgumentException,
                        "wrong number of args");
        return NULL;
    }

    CREATE_TOP_FRAME(ee, mb->class, mb, sp, ret);

    if(ob) *sp++ = (uintptr_t)ob;

    for(i = 0; i < args_len; i++) {
        int size = unwrapAndWidenObject(*types++, *args++, sp, REF_DST_OSTACK);

        if(size == 0) {
            POP_TOP_FRAME(ee);
            signalException(java_lang_IllegalArgumentException,
                            "arg type mismatch");
            return NULL;
        }

        sp += size;
    }

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectLock(ob ? ob : mb->class);

    if(mb->access_flags & ACC_NATIVE)
        (*mb->native_invoker)(mb->class, mb, ret);
    else
        executeJava();

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectUnlock(ob ? ob : mb->class);

    POP_TOP_FRAME(ee);

    if((excep = exceptionOccurred())) {
        Object *ite_excep;
        MethodBlock *init;
        Class *ite_class;

        clearException();        
        ite_class = findSystemClass("java/lang/reflect/InvocationTargetException");

        if(!exceptionOccurred() && (ite_excep = allocObject(ite_class)) &&
                      (init = lookupMethod(ite_class, SYMBOL(object_init),
                                           SYMBOL(_java_lang_Throwable__V)))) {
            executeMethod(ite_excep, init, excep);
            setException(ite_excep);
        }
        return NULL;
    }

    return ret;
}

/* Functions to get values from the VM-level reflection objects */

MethodBlock *getConsMethodBlock(Object *vm_cons_obj) {
    Class *decl_class = INST_DATA(vm_cons_obj, Class*, vm_cons_class_offset);
    int slot = INST_DATA(vm_cons_obj, int, vm_cons_slot_offset);

    return &CLASS_CB(decl_class)->methods[slot];
}

int getConsAccessFlag(Object *vm_cons_obj) {
    Object *cons_obj = INST_DATA(vm_cons_obj, Object*, vm_cons_cons_offset);
    return INST_DATA(cons_obj, int, acc_flag_offset);
}

int getMethodAccessFlag(Object *vm_mthd_obj) {
    Object *mthd_obj = INST_DATA(vm_mthd_obj, Object*, vm_mthd_m_offset);
    return INST_DATA(mthd_obj, int, acc_flag_offset);
}

MethodBlock *getMethodMethodBlock(Object *vm_mthd_obj) {
    Class *decl_class = INST_DATA(vm_mthd_obj, Class*, vm_mthd_class_offset);
    int slot = INST_DATA(vm_mthd_obj, int, vm_mthd_slot_offset);

    return &CLASS_CB(decl_class)->methods[slot];
}

FieldBlock *getFieldFieldBlock(Object *vm_fld_obj) {
    Class *decl_class = INST_DATA(vm_fld_obj, Class*, vm_fld_class_offset);
    int slot = INST_DATA(vm_fld_obj, int, vm_fld_slot_offset);

    return &(CLASS_CB(decl_class)->fields[slot]);
}

int getFieldAccessFlag(Object *vm_fld_obj) {
    Object *fld_obj = INST_DATA(vm_fld_obj, Object*, vm_fld_f_offset);
    return INST_DATA(fld_obj, int, acc_flag_offset);
}

/* Reflection access from JNI */

Object *createReflectConstructorObject(MethodBlock *mb) {
    if(!inited && !initReflection())
        return NULL;

    return createConstructorObject(mb);
}

Object *createReflectMethodObject(MethodBlock *mb) {
    if(!inited && !initReflection())
        return NULL;

    return createMethodObject(mb);
}

Object *createReflectFieldObject(FieldBlock *fb) {
    if(!inited && !initReflection())
        return NULL;

    return createFieldObject(fb);
}

MethodBlock *mbFromReflectObject(Object *reflect_ob) {
    MethodBlock *mb;

    if(reflect_ob->class == cons_reflect_class) {
        Object *vm_cons_obj = INST_DATA(reflect_ob, Object*, cons_cons_offset);
        mb = getConsMethodBlock(vm_cons_obj);
    } else {
        Object *vm_mthd_obj = INST_DATA(reflect_ob, Object*, mthd_m_offset);
        mb = getMethodMethodBlock(vm_mthd_obj);
    }

    return mb;
}

FieldBlock *fbFromReflectObject(Object *reflect_ob) {
    Object *vm_fld_obj = INST_DATA(reflect_ob, Object*, fld_f_offset);
    return getFieldFieldBlock(vm_fld_obj);
}

/* Needed for stack walking */

Class *getReflectMethodClass() {
    return method_reflect_class;
}
