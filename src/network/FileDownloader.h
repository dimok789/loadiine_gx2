/****************************************************************************
 * Copyright (C) 2016 Dimok
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
#ifndef _FILE_DOWNLOADER_H
#define _FILE_DOWNLOADER_H

#include <string>
#include "fs/CFile.hpp"

class FileDownloader
{
public:
    typedef void (*ProgressCallback)(void *arg, u32 done, u32 total);

    static bool getFile(const std::string & downloadUrl, std::string & fileBuffer, ProgressCallback callback = 0, void *arg = 0);
    static bool getFile(const std::string & downloadUrl, const std::string & outputPath, ProgressCallback callback = 0, void *arg = 0);
private:
    typedef struct
    {
        ProgressCallback progressCallback;
        void *progressArg;
        CFile * file;
        u8 *buffer;
        u32 filesize;
    } curl_private_data_t;

    static bool internalGetFile(const std::string & downloadUrl, curl_private_data_t * private_data);
    static int curlCallback(void *buffer, int size, int nmemb, void *userp);
    static int curlProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
};

#endif // _FILE_DOWNLOADER_H
