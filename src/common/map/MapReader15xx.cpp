#include "MapReader.h"

#include "map.h"
#include "FileIO.h"
#include "TilesetManager.h"

#include <iostream>

extern CTilesetManager* g_tilesetmanager;
extern const char* g_szBackgroundConversion[26];
extern short g_iMusicCategoryConversion[26];
extern short g_iTileTypeConversion[NUMTILETYPES];
extern short g_iDefaultPowerupPresets[NUM_POWERUP_PRESETS][NUM_POWERUPS];

void MapLoader1500::read_autofilters(CMap& map, FILE* mapfile)
{
    for (short iFilter = 0; iFilter < NUM_AUTO_FILTERS; iFilter++)
        map.fAutoFilter[iFilter] = false;
}

void MapLoader1500::read_tiles(CMap& map, FILE* mapfile)
{
    unsigned short i, j;

    // 2. load map data
    for (j = 0; j < MAPHEIGHT; j++) {
        for (i = 0; i < MAPWIDTH; i++) {
            //Read everything into layer 1
            short iTileID = (short)ReadInt(mapfile);

            TilesetTile * tile = &map.mapdata[i][j][1];
            tile->iID = g_tilesetmanager->GetClassicTilesetIndex();
            tile->iCol = iTileID % 32;
            tile->iRow = iTileID / 32;

            TileType iType = g_tilesetmanager->GetClassicTileset()->GetTileType(tile->iCol, tile->iRow);

            if (iType >= 0 && iType < NUMTILETYPES) {
                map.mapdatatop[i][j].iType = iType;
                map.mapdatatop[i][j].iFlags = g_iTileTypeConversion[iType];
            } else {
                map.mapdatatop[i][j].iType = tile_nonsolid;
                map.mapdatatop[i][j].iFlags = tile_flag_nonsolid;
            }

            map.mapdata[i][j][0].iID = TILESETNONE;
            map.mapdata[i][j][2].iID = TILESETNONE;
            map.mapdata[i][j][3].iID = TILESETNONE;
        }
    }

    // 3. load objects data
    for (j = 0; j < MAPHEIGHT; j++) {
        for (i = 0; i < MAPWIDTH; i++) {
            map.objectdata[i][j].iType = (short)ReadInt(mapfile);
            if (map.objectdata[i][j].iType == 6)
                map.objectdata[i][j].iType = -1;

            map.objectdata[i][j].fHidden = false;

            if (map.objectdata[i][j].iType == 1) {
                for (short iSetting = 0; iSetting < NUM_BLOCK_SETTINGS; iSetting++)
                    map.objectdata[i][j].iSettings[iSetting] = g_iDefaultPowerupPresets[0][iSetting];
            }
        }
    }
}

void MapLoader1500::read_background(CMap& map, FILE* mapfile)
{
    //Read old background IDs and convert that to a background filename
    map.backgroundID = (short)ReadInt(mapfile);
    strcpy(map.szBackgroundFile, g_szBackgroundConversion[map.backgroundID]);
}

void MapLoader1500::read_music_category(CMap& map, FILE* mapfile)
{
    map.musicCategoryID = g_iMusicCategoryConversion[map.backgroundID];
}

bool MapLoader1500::load(CMap& map, FILE* mapfile, ReadType readtype)
{
    read_autofilters(map, mapfile);

    if (readtype == read_type_summary)
        return true;

    map.clearPlatforms();

    std::cout << "loading map " << filename << " [Version 1.5 Map Detected]" << std::endl;

    //Reset position of read cursor
    rewind(mapfile);

    //clear map (we won't be reading in all the layers so it needs to be cleared)
    map.clearMap();

    read_tiles(map, mapfile);
    read_background(map, mapfile);
    read_music_category(map, mapfile);

    //All 1.5 maps will use cloud eyecandy
    map.eyecandy[2] = 1;

    for (short iSwitch = 0; iSwitch < 4; iSwitch++)
        map.iSwitches[iSwitch] = 0;
}