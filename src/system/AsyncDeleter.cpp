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
#include "AsyncDeleter.h"

AsyncDeleter * AsyncDeleter::deleterInstance = NULL;

AsyncDeleter::AsyncDeleter()
	: CThread(CThread::eAttributeAffCore1 | CThread::eAttributePinnedAff)
	, exitApplication(false)
{
}

AsyncDeleter::~AsyncDeleter()
{
    exitApplication = true;
}

void AsyncDeleter::triggerDeleteProcess(void)
{
    if(!deleterInstance)
        deleterInstance = new AsyncDeleter;

    //! to trigger the event after GUI process is finished execution
    //! this function is used to swap elements from one to next array
    if(!deleterInstance->deleteElements.empty())
    {
        deleterInstance->deleteMutex.lock();
        while(!deleterInstance->deleteElements.empty())
        {
            deleterInstance->realDeleteElements.push(deleterInstance->deleteElements.front());
            deleterInstance->deleteElements.pop();
        }
        deleterInstance->deleteMutex.unlock();
        deleterInstance->resumeThread();
    }
}

void AsyncDeleter::executeThread(void)
{
    while(!exitApplication)
    {
        suspendThread();

        //! delete elements that require post process deleting
        //! because otherwise they would block or do invalid access on GUI thread
        while(!realDeleteElements.empty())
        {
            deleteMutex.lock();
            AsyncDeleter::Element *element = realDeleteElements.front();
            realDeleteElements.pop();
            deleteMutex.unlock();

            delete element;
        }
    }

}
