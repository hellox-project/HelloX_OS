/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009
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

#include <stdafx.h>
#include <kapi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <io.h>

#include "../../jam.h"

void *nativeStackBase()
{
	return KernelThreadManager.lpCurrentKernelThread->lpInitStackPointer;
}

int nativeAvailableProcessors() {
	return 1;  //Can not support SMP yet.
}

char *nativeLibError() {
    return NULL;
}

char *nativeLibPath() {
    return "C:\\JVM\\";
}

void *nativeLibOpen(char *path) {
    return NULL;
}

void nativeLibClose(void *handle) {
    return;
}

void *nativeLibSym(void *handle, char *symbol) {
    return NULL;
}

char *nativeLibMapName(char *name) {
   return NULL;
}
