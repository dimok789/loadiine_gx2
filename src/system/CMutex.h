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
#ifndef _CMUTEX_H_
#define _CMUTEX_H_

#include <malloc.h>
#include "dynamic_libs/os_functions.h"

class CMutex
{
public:
    CMutex() {
        pMutex = malloc(OS_MUTEX_SIZE);
        if(!pMutex)
            return;

        OSInitMutex(pMutex);
    }
    virtual ~CMutex() {
        if(pMutex)
            free(pMutex);
    }

    void lock(void) {
        if(pMutex)
            OSLockMutex(pMutex);
    }
    void unlock(void) {
        if(pMutex)
            OSUnlockMutex(pMutex);
    }
    bool tryLock(void) {
        if(!pMutex)
            return false;

        return (OSTryLockMutex(pMutex) != 0);
    }
private:
    void *pMutex;
};

class CMutexLock
{
public:
    CMutexLock() {
        mutex.lock();
    }
    virtual ~CMutexLock() {
        mutex.unlock();
    }
private:
    CMutex mutex;
};

#endif // _CMUTEX_H_
