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
#ifndef _GUIIMAGEASYNC_H_
#define _GUIIMAGEASYNC_H_

#include <vector>
#include "GuiImage.h"
#include "system/CThread.h"
#include "system/CMutex.h"
#include "dynamic_libs/os_functions.h"

class GuiImageAsync : public GuiImage
{
	public:
		GuiImageAsync(const u8 *imageBuffer, const u32 & imageBufferSize, GuiImageData * preloadImg);
		GuiImageAsync(const std::string & filename, GuiImageData * preloadImg);
		virtual ~GuiImageAsync();

		static void clearQueue();
		static void removeFromQueue(GuiImageAsync * image) {
		    threadRemoveImage(image);
		}
	private:
		static void threadInit();
		static void threadExit();

		GuiImageData *imgData;
	    std::string filename;
	    const u8 *imgBuffer;
	    const u32 imgBufferSize;

		static void guiImageAsyncThread(CThread *thread, void *arg);
		static void threadAddImage(GuiImageAsync* Image);
		static void threadRemoveImage(GuiImageAsync* Image);

		static std::vector<GuiImageAsync *> imageQueue;
		static CThread *pThread;
		static CMutex * pMutex;
		static u32 threadRefCounter;
		static GuiImageAsync * pInUse;
		static bool bExitRequested;
};

#endif /*_GUIIMAGEASYNC_H_*/
