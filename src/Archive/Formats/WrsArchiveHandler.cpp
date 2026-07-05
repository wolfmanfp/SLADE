// -----------------------------------------------------------------------------
// SLADE - It's a Doom Editor
// Copyright(C) 2008 - 2026 Simon Judd
//
// Email:       sirjuddington@gmail.com
// Web:         http://slade.mancubus.net
// Filename:    WrsArchiveHandler.cpp
// Description: ArchiveFormatHandler for WRS format archives
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//
// Includes
//
// -----------------------------------------------------------------------------
#include "Main.h"
#include "WrsArchiveHandler.h"
#include "Archive/Archive.h"
#include "Archive/ArchiveDir.h"
#include "Archive/ArchiveEntry.h"
#include "UI/UI.h"
#include "Utility/Compression.h"
#include <fstream>


using namespace slade;

bool WrsArchiveHandler::open(Archive &archive, const MemChunk &mc)
{
    // Stop announcements (don't want to be announcing modification due to entries being added etc)
    ArchiveModSignalBlocker sig_blocker{ archive };

    mc.seek(0, SEEK_SET);
    do
    {
        // Update splash window progress
        ui::setSplashProgress(mc.currentPos(), mc.size());

        char filename[13] = {};
        uint32_t compressedSize;
        uint32_t uncompressedSize;
        mc.read(filename, 13);
        mc.read(&compressedSize, 4);
        mc.read(&uncompressedSize, 4);

        compressedSize   = wxUINT32_SWAP_ON_LE(compressedSize);
        uncompressedSize = wxUINT32_SWAP_ON_LE(uncompressedSize);

        auto entry = std::make_shared<ArchiveEntry>(filename, uncompressedSize);

        MemChunk temp(compressedSize);
        mc.exportMemChunk(temp, mc.currentPos(), compressedSize);
        MemChunk xdata(uncompressedSize);
        if (compression::lzssDecompress(temp, xdata))
            entry->importMemChunk(xdata);
        else
            return false;

        mc.seek(compressedSize, SEEK_CUR);
        archive.rootDir()->addEntry(entry);
        entry->setState(EntryState::Unmodified);
    } while (mc.currentPos() < mc.size());

    // Detect all entry types
    detectAllEntryTypes(archive);

    // Setup variables
    sig_blocker.unblock();
    archive.setModified(false);

    ui::setSplashProgressMessage("");

    return true;
}

bool WrsArchiveHandler::write(Archive &archive, MemChunk &mc)
{
    return ArchiveFormatHandler::write(archive, mc);
}

bool WrsArchiveHandler::isThisFormat(const MemChunk &mc)
{
    return false;
}

bool WrsArchiveHandler::isThisFormat(const string &filename)
{
    return true;
}
