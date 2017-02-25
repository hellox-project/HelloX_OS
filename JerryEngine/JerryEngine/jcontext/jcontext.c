/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jcontext.h"

/** \addtogroup context Context
 * @{
 */

/**
 * Global context.
 */
jerry_context_t jerry_global_context;

/**
 * Jerry global heap section attribute.
 */
#ifndef JERRY_HEAP_SECTION_ATTR
#define JERRY_GLOBAL_HEAP_SECTION
#else /* JERRY_HEAP_SECTION_ATTR */
#define JERRY_GLOBAL_HEAP_SECTION __attribute__ ((section (JERRY_HEAP_SECTION_ATTR)))
#endif /* !JERRY_HEAP_SECTION_ATTR */

/**
 * Global heap.
 */
//jmem_heap_t jerry_global_heap __attribute__ ((aligned (JMEM_ALIGNMENT))) JERRY_GLOBAL_HEAP_SECTION;

/**
 * We should use __declspec(align(x)) macro to define the jerry_global_heap in Microsoft
 * Visual studio,and we encapsulate this premitive to __HXCL_DEFINE_ALIGNED_OBJECT macro,
 * but the macro can only accept direct value than a expression(JMEM_ALIGNMENT is a expression
 * of (1u << JMEM_ALIGNMENT_LOG),so we have to define the
 * object several times according JMEM_ALIGNMENT_LOG's value.
 * An error will be dispatched if the JMEM_ALIGNMENT_LOG's value is not covered,you should
 * add the corresponding sentence manually.
 */
#if (0 == JMEM_ALIGNMENT_LOG)
__HXCL_DEFINE_ALIGNED_OBJECT(jmem_heap_t, jerry_global_heap, 1) JERRY_GLOBAL_HEAP_SECTION;
#elif (1 == JMEM_ALIGNMENT_LOG)
__HXCL_DEFINE_ALIGNED_OBJECT(jmem_heap_t, jerry_global_heap, 2) JERRY_GLOBAL_HEAP_SECTION;
#elif (2 == JMEM_ALIGNMENT_LOG)
__HXCL_DEFINE_ALIGNED_OBJECT(jmem_heap_t, jerry_global_heap, 4) JERRY_GLOBAL_HEAP_SECTION;
#elif (3 == JMEM_ALIGNMENT_LOG)
__HXCL_DEFINE_ALIGNED_OBJECT(jmem_heap_t, jerry_global_heap, 8) JERRY_GLOBAL_HEAP_SECTION;
#elif (4 == JMEM_ALIGNMENT_LOG)
__HXCL_DEFINE_ALIGNED_OBJECT(jmem_heap_t, jerry_global_heap, 16) JERRY_GLOBAL_HEAP_SECTION;
#else
error "Un expected JMEM_ALIGNMENT_LOG value,please append."
#endif //JMEM_ALIGNMENT_LOG.

#ifndef CONFIG_ECMA_LCACHE_DISABLE

/**
 * Global hash table.
 */
jerry_hash_table_t jerry_global_hash_table;

#endif /* !CONFIG_ECMA_LCACHE_DISABLE */

/**
 * @}
 */
