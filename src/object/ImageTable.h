#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#pragma once

#include <vector>
#include "../common.h"

extern "C"
{
    #include "../drawing/drawing.h"
}

interface IReadObjectContext;
interface IStream;

class ImageTable
{
private:
    std::vector<rct_g1_element> _entries;
    void *                      _data       = nullptr;
    size_t                      _dataSize   = 0;

public:
    ~ImageTable();

    void                    Read(IReadObjectContext * context, IStream * stream);
    const rct_g1_element *  GetImages() const { return _entries.data(); }
    uint32                  GetCount() const { return _entries.size(); };
};