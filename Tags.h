#pragma once

#include "Jukebox.h"

// Property tags of /custom_properties, as listed in motherboard_def.lua.
const TJBox_Tag kOnTag = 1;
const TJBox_Tag kRunTag = 2;
const TJBox_Tag kRecTag = 3;
const TJBox_Tag kPatternLightFrameTag = 10;
const TJBox_Tag kRunLightFrameTag = 11;
const TJBox_Tag kRecLightFrameTag = 12;

// Frames of the RUN and REC buttons' film strips.
const int kButtonFrameUnlit = 0;
const int kButtonFrameLit = 1;
const int kButtonFrameInactive = 2;
