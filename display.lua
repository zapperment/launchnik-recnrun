format_version = "1.0"

-- The custom displays on the front panel are transparent click areas; what
-- lies underneath them is drawn by sequence meters.
Transparent_Draw = function(props, di, dr)
end

-- Every click on REC, RUN or a pattern button also keeps what has been
-- recorded (see CONTEXT.md). Each of their displays lists its own values
-- first, then the values for keeping, as hdgui_2D.lua builds them:
--   unkept (bit n - 1 set while pattern n has notes not yet kept),
--   the realtime strings pattern_<n>_part_<k>, pattern by pattern,
--   the kept patterns pattern_1 to pattern_8.
local PATTERN_COUNT = 8
local PART_COUNT = 5

-- Adds to `changes` a copy of each pattern with notes not yet kept; `first`
-- is the index of unkept in props. Returns whether there is any.
local function keep(props, first, changes)
	local bits = props[first]
	local any = false
	for pattern = 1, PATTERN_COUNT do
		local bit = bits % 2
		bits = (bits - bit) / 2
		if bit == 1 then
			local text = ""
			for part = 1, PART_COUNT do
				text = text .. props[first + (pattern - 1) * PART_COUNT + part]
			end
			changes[first + PATTERN_COUNT * PART_COUNT + pattern] = text
			any = true
		end
	end
	return any
end

-- The result of a click: its own changes and a keep, as one undo step named
-- after the keep if there is one.
local function click_result(props, first, changes, has_own_changes, name)
	if keep(props, first, changes) then
		return { gesture_ui_name = jbox.ui_text("text_keep_recording"), property_changes = changes }
	end
	if not has_own_changes then
		return {}
	end
	return { gesture_ui_name = jbox.ui_text(name), property_changes = changes }
end

-- RUN and REC do nothing but keep while the device is off or bypassed.
-- props: [2] ON, [3] whether Reason bypasses the player, [4] onwards the
-- values for keeping.
local BUTTON_KEEP_FIRST = 4

local function inactive(props)
	return not props[2] or props[3]
end

local function button_click(props, name)
	local changes = {}
	local active = not inactive(props)
	if active then
		changes[1] = not props[1]
	end
	return click_result(props, BUTTON_KEEP_FIRST, changes, active, name)
end

-- A click on RUN starts or pauses the player (the realtime code decides
-- which). Each click changes the run property, either way. props: [1] RUN.
RunButton_Gesture = function(props, di, gsp)
	return { custom_data = {}, handlers = { on_tap = "RunButton_on_tap" } }
end

RunButton_on_tap = function(props, di, gi, cd)
	return button_click(props, "text_run")
end

-- A click on REC switches recording on and off. props: [1] REC.
RecButton_Gesture = function(props, di, gsp)
	return { custom_data = {}, handlers = { on_tap = "RecButton_on_tap" } }
end

RecButton_on_tap = function(props, di, gi, cd)
	return button_click(props, "text_rec")
end

-- A click on a pattern button selects that pattern: Reason's own pattern
-- index, from 0. props: [1] the selected pattern's index, [2] onwards the
-- values for keeping.
local PATTERN_KEEP_FIRST = 2

local function select_pattern(props, index)
	return click_result(props, PATTERN_KEEP_FIRST, { [1] = index }, true, "text_select_pattern")
end

local function pattern_gesture(tap_handler)
	return { custom_data = {}, handlers = { on_tap = tap_handler } }
end

PatternButton1_Gesture = function() return pattern_gesture("PatternButton1_on_tap") end
PatternButton2_Gesture = function() return pattern_gesture("PatternButton2_on_tap") end
PatternButton3_Gesture = function() return pattern_gesture("PatternButton3_on_tap") end
PatternButton4_Gesture = function() return pattern_gesture("PatternButton4_on_tap") end
PatternButton5_Gesture = function() return pattern_gesture("PatternButton5_on_tap") end
PatternButton6_Gesture = function() return pattern_gesture("PatternButton6_on_tap") end
PatternButton7_Gesture = function() return pattern_gesture("PatternButton7_on_tap") end
PatternButton8_Gesture = function() return pattern_gesture("PatternButton8_on_tap") end

PatternButton1_on_tap = function(props) return select_pattern(props, 0) end
PatternButton2_on_tap = function(props) return select_pattern(props, 1) end
PatternButton3_on_tap = function(props) return select_pattern(props, 2) end
PatternButton4_on_tap = function(props) return select_pattern(props, 3) end
PatternButton5_on_tap = function(props) return select_pattern(props, 4) end
PatternButton6_on_tap = function(props) return select_pattern(props, 5) end
PatternButton7_on_tap = function(props) return select_pattern(props, 6) end
PatternButton8_on_tap = function(props) return select_pattern(props, 7) end
