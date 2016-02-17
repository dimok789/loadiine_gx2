/****************************************************************************
 * Copyright (C) 2015 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef _MEMORY_AREA_TABLE_H_
#define _MEMORY_AREA_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Struct used to organize empty memory areas */
typedef struct _s_mem_area
{
    unsigned int        address;
    unsigned int        size;
    struct _s_mem_area* next;
} s_mem_area;

void memoryInitAreaTable();
s_mem_area * memoryGetAreaTable(void);


#ifdef __cplusplus
}
#endif

#endif // _MEMORY_AREA_TABLE_H_
