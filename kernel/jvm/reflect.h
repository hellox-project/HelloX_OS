/*
 * Copyright (C) 2009 Robert Lougher <rob@jamvm.org.uk>.
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

#define REF_SRC_FIELD  0
#define REF_DST_FIELD  0
#define REF_SRC_OSTACK 1
#define REF_DST_OSTACK 2

#define getPrimTypeIndex(cb) (cb->state - CLASS_PRIM)

#define getConsParamTypes(vm_cons_obj) \
    INST_DATA(vm_cons_obj, Object*, vm_cons_param_offset)

#define getMethodParamTypes(vm_method_obj) \
    INST_DATA(vm_method_obj, Object*, vm_mthd_param_offset)

#define getMethodReturnType(vm_method_obj) \
    INST_DATA(vm_method_obj, Class*, vm_mthd_ret_offset)

#define getFieldType(vm_field_obj) \
    INST_DATA(vm_field_obj, Class*, vm_fld_type_offset)

extern MethodBlock *getConsMethodBlock(Object *cons_ref_obj);
extern int getConsAccessFlag(Object *cons_ref_obj);
extern MethodBlock *getMethodMethodBlock(Object *mthd_ref_obj);
extern int getMethodAccessFlag(Object *mthd_ref_obj);
extern FieldBlock *getFieldFieldBlock(Object *fld_ref_obj);
extern int getFieldAccessFlag(Object *fld_ref_obj);

extern int vm_cons_param_offset, vm_mthd_param_offset, vm_mthd_ret_offset;
extern int vm_fld_type_offset;

extern Object *getClassConstructors(Class *class, int public);
extern Object *getClassMethods(Class *class, int public);
extern Object *getClassFields(Class *class, int public);
extern Object *getClassInterfaces(Class *class);
extern Object *getClassClasses(Class *class, int public);
extern Class *getDeclaringClass(Class *class);
extern Class *getEnclosingClass(Class *class);
extern Object *getEnclosingMethodObject(Class *class);
extern Object *getEnclosingConstructorObject(Class *class);
extern Object *getClassAnnotations(Class *class);
extern Object *getFieldAnnotations(FieldBlock *fb);
extern Object *getMethodAnnotations(MethodBlock *mb);
extern Object *getMethodParameterAnnotations(MethodBlock *mb);
extern Object *getMethodDefaultValue(MethodBlock *mb);
extern Object *getExceptionTypes(MethodBlock *mb);

extern Object *getReflectReturnObject(Class *type, void *pntr, int flags);
extern int widenPrimitiveValue(int src_idx, int dest_idx, void *src,
                               void *dest, int flags);
extern int unwrapAndWidenObject(Class *type, Object *arg, void *pntr,
                                int flags);
extern Object *invoke(Object *ob, MethodBlock *mb, Object *arg_array,
                      Object *param_types);

extern MethodBlock *mbFromReflectObject(Object *reflect_ob);
extern FieldBlock *fbFromReflectObject(Object *reflect_ob);

extern Object *createReflectConstructorObject(MethodBlock *mb);
extern Object *createReflectMethodObject(MethodBlock *mb);
extern Object *createReflectFieldObject(FieldBlock *fb);
extern Class *getReflectMethodClass();
