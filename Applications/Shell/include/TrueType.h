#ifndef TRUE_TYPE_H
#define TRUE_TYPE_H

#include "OTOSCore/Definitions.h"

typedef u16 u2be;
typedef u32 u4be;
typedef i16 s2be;
typedef i32 s4be;

typedef struct {
	u32 * screen;
	i32 windowWidth;
	i32 windowHeight;
} ScreenData;

typedef struct {
	u8 * buffer;
	u32 fileSize;
} FileData;

typedef struct __attribute__((packed)) {
    u32 sfnt_version;
    u2be num_tables;
    u2be search_range;
    u2be entry_selector;
    u2be range_shift;
} TtfOffsetTable;

typedef struct __attribute__((packed)) {
    char tag[4];
    u4be checksum;
    u4be offset;
    u4be length;
} TtfDirTableEntry;

typedef struct __attribute__((packed)) {
    TtfOffsetTable OffsetTable;
} ttfRoot;

typedef struct __attribute__((packed)) {
    s2be number_of_contours;
    s2be x_min;
    s2be y_min;
    s2be x_max;
    s2be y_max;
} ttfGlyf;

typedef struct __attribute__((packed)) {
    u32 version;
    u32 font_revision;
    u4be checksum_adjustment;
    u32 magic_number;
    u2be flags;
    u2be units_per_em;

    u4be created_0;
    u4be created_1;

    u4be modified_0;
    u4be modified_1;
    s2be x_min;
    s2be y_min;
    s2be x_max;
    s2be y_max;
    u2be mac_style;
    u2be lowest_rec_ppem;
    s2be font_direction_hint;
    s2be index_to_loc_format;
    s2be glyph_data_format;

} ttfHead;

typedef struct __attribute__((packed)) {
    u32 version;
    s2be ascender;
    s2be descender;
    s2be line_gap;
    u2be advance_width_max;
    s2be min_left_wide_bearing;
    s2be min_right_side_bearing;
    s2be x_max_extend;
    s2be caret_slope_rise;
    s2be caret_slope_run;
    u8 reserved[10];
    s2be metric_data_format;
    u2be number_of_hmetrics;
} ttfHhea;

typedef struct __attribute__((packed)) {
    u2be advanceWidth;
    s2be leftSideBearing;
} ttfHMetric;

typedef struct __attribute__((packed)) {
    u2be version;
    u2be numTables;
} ttfCmapHeader;

typedef struct __attribute__((packed)) {
    u2be platformID;
    u2be encodingID;
    u4be subtableOffset;
} ttfCmapSubtableHeader;

typedef struct {
    u8 * flags;
    int nPoints;
    int * points;

    int nContours;
    int * endpoints;

    int w;
    int h;
    int x1;
    int y1;
    int x2;
    int y2;
    int advanceWidth;

    int X1;
    int Y1;
    int X2;
    int Y2;

    int glyphIndex;
} glyphInfo;

typedef struct __attribute__((packed)){
    u2be left;
    u2be right;
    s2be value;
} ttfKernSubtableFormat0KerningPair;

typedef struct __attribute__((packed)){
    u32 version;
    u2be format;
    u2be horizOffset;
    u2be vertOffset;
    u2be reserved;
} ttfTrackTable;

typedef struct __attribute__((packed)){
    u2be nTracks;
    u2be nSizes;
    u4be sizeTableOffset;
} ttfTrackData;

typedef struct __attribute__((packed)){
    i32 track;
    u2be nameIndex;
    u2be offset;
} ttfTrackDataEntry;

typedef struct {
	ScreenData * screen;
	float fontWidth;
	float fontHeight;
	float xBufferOffset;
	float yBufferOffset;
    float advanceWidth;
	int glyphIndex;

    float totalWidth;
    float totalHeight;
} fontCharacterCache;

struct trueTypeFontRenderer;
typedef struct trueTypeFontDescriptor {
    struct trueTypeFontDescriptor * nextDescriptor;
    struct trueTypeFontDescriptor * previousDescriptor;

    struct trueTypeFontRenderer * rootRenderer;

    char * fontFileName;
    FileData * fontFileRaw;
    int referenceCounter;

    glyphInfo * glyphInfoCache[128];
    unsigned char * kernTablePointer;
} trueTypeFontDescriptor;

typedef struct trueTypeFontRenderer {
    struct trueTypeFontRenderer * nextRenderer;
    struct trueTypeFontRenderer * previousRenderer;

    struct trueTypeFontDescriptor * descriptor;
    float fontSize;
    int aliasScale;
    float scale;

    bool8 monoSpace;
    float monoSpaceWidth;
    float tabWidth;

    fontCharacterCache * cache[128];
    int referenceCounter;
} trueTypeFontRenderer;

typedef struct trueTypeFontManager {
    trueTypeFontDescriptor * descriptor;
} trueTypeFontManager;

trueTypeFontDescriptor * openTrueTypeFont(trueTypeFontManager * fontManager, char * fontFileName);
trueTypeFontRenderer * openTrueTypeRenderer(trueTypeFontDescriptor * fontDescriptor, float fontSize);
fontCharacterCache * trueType_renderCharacter(int c, trueTypeFontRenderer * renderer);
void trueType_drawCharacter(fontCharacterCache * character, int x, int y, ScreenData * screen, u32 color);
void closeTrueTypeRenderer(trueTypeFontRenderer * renderer);

#endif