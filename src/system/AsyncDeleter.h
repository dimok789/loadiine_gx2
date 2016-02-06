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
#ifndef _ASYNC_DELETER_H
#define _ASYNC_DELETER_H

#include <queue>
#include "CThread.h"
#include "CMutex.h"

class AsyncDeleter : public CThread
{
public:
    static void destroyInstance()
    {
        delete deleterInstance;
        deleterInstance = NULL;
    }

    class Element
    {
    public:
        Element() {}
        virtual ~Element() {}
    };

    static void pushForDelete(AsyncDeleter::Element *e)
    {
        if(!deleterInstance)
            deleterInstance = new AsyncDeleter;

        deleterInstance->deleteElements.push(e);
    }

    static void triggerDeleteProcess(void);

private:
    AsyncDeleter();
    virtual ~AsyncDeleter();

    static AsyncDeleter *deleterInstance;

    void executeThread(void);

    bool exitApplication;
    std::queue<AsyncDeleter::Element *> deleteElements;
    std::queue<AsyncDeleter::Element *> realDeleteElements;
    CMutex deleteMutex;
};

#endif // _ASYNC_DELETER_H
