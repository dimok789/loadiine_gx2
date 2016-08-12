/****************************************************************************
 * Copyright (C) 2016 Maschell
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
#ifndef _CursorDrawer_H_
#define _CursorDrawer_H_

#include <string>

#include <stdio.h>
#include <gctypes.h>
#include <gctypes.h>

#include "dynamic_libs/gx2_types.h"

class CursorDrawer
{

public:
    static CursorDrawer *getInstance() {
        if(!instance)
            instance = new CursorDrawer();
        return instance;
    }

    static void destroyInstance() {
        if(instance){
            delete instance;
            instance = NULL;
        }
    }

    static void draw(f32 x, f32 y)
    {
        CursorDrawer * cur_instance = getInstance();
        if(cur_instance ==  NULL) return;
        cur_instance->draw_Cursor(x,y);
    }

private:
    //!Constructor
    CursorDrawer();
    //!Destructor
    ~CursorDrawer();
    static CursorDrawer *instance;
    void draw_Cursor(f32 x, f32 y);
    void init_colorVtxs();

    u8 * colorVtxs = NULL;
};

#endif
