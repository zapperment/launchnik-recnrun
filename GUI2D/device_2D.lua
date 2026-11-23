format_version = "2.0"

-- Positions are taken to the pixel from the screen designs (3770 x 2070, like
-- the panel images). Players have no folded panels.
panel_type = "note_player"

-- The pattern buttons are one film strip, frame 0 with none lit and frames 1
-- to 8 with that one lit, with a transparent click area on top of each
-- button. The buttons are 100 pixels apart.
local PATTERN_LEFT = 2094
local PATTERN_TOP = 29
local PATTERN_PITCH = 100

-- The RUN and REC buttons are film strips with an inactive frame, each with
-- a transparent click area on top.
local RUN_LEFT = 1661
local RUN_TOP = 25
local REC_LEFT = 1443
local REC_TOP = 25

front = {
	Bg = {
		{ path = "Player_Panel_Front_6U" },
	},
	PatchBrowserBg = {
		offset = { 324, 30 },
		{ path = "Patch_Browser_BG_Player_1frame" },
	},
	PatchBrowseGroup = {
		offset = { 330, 39 },
		{ path = "PatchBrowseGroup" },
	},
	PatchName = {
		offset = { 615, 50 },
		{ size = { 600, 80 } },
	},
	DeviceName = {
		offset = { 704, 175 },
		{ path = "Tape_Horizontal_1frames" },
	},
	OnButton = {
		offset = { 152, 30 },
		{ path = "Button_Player_On_2frames", frames = 2 },
	},
	RecLight = {
		offset = { REC_LEFT, REC_TOP },
		{ path = "Button_Player_Rec_3frames", frames = 3 },
	},
	RecClick = {
		offset = { REC_LEFT + 10, REC_TOP + 10 },
		{ size = { 210, 215 } },
	},
	RunLight = {
		offset = { RUN_LEFT, RUN_TOP },
		{ path = "Button_Player_Run_3frames", frames = 3 },
	},
	RunClick = {
		offset = { RUN_LEFT + 10, RUN_TOP + 10 },
		{ size = { 210, 215 } },
	},
	PatternLights = {
		offset = { PATTERN_LEFT, PATTERN_TOP },
		{ path = "Button_Player_Pattern_9frames", frames = 9 },
	},
	PatternLabel = {
		offset = { 2401, 232 },
		{ path = "Label_Pattern_1frame" },
	},
	Logo = {
		offset = { 3125, 29 },
		{ path = "Logo_RecNRun_1frame" },
	},
}

for pattern = 1, 8 do
	front["PatternClick" .. pattern] = {
		offset = { PATTERN_LEFT + 10 + (pattern - 1) * PATTERN_PITCH, PATTERN_TOP + 10 },
		{ size = { 100, 180 } },
	}
end

back = {
	Bg = {
		{ path = "Player_Panel_Back_6U" },
	},
	DeviceName = {
		offset = { 1685, 1982 },
		{ path = "Tape_Horizontal_1frames" },
	},
	NameLabel = {
		offset = { 355, 1993 },
		{ path = "Label_RecNRun_Player_1frame" },
	},
	MadeWithLoveLabel = {
		offset = { 2713, 1993 },
		{ path = "Label_Made_With_Love_1frame" },
	},
}
