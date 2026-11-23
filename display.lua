format_version = "1.0"

-- The custom displays on the front panel are transparent click areas; what
-- lies underneath them is drawn by sequence meters.
Transparent_Draw = function(props, di, dr)
end

-- RUN and REC do nothing while the device is off or bypassed.
-- props: [2] ON, [3] whether Reason bypasses the player.
local function inactive(props)
	return not props[2] or props[3]
end

-- A click on RUN starts or pauses the player (the realtime code decides
-- which). Each click changes the run property, either way. props: [1] RUN.
RunButton_Gesture = function(props, di, gsp)
	return { custom_data = {}, handlers = { on_tap = "RunButton_on_tap" } }
end

RunButton_on_tap = function(props, di, gi, cd)
	if inactive(props) then
		return {}
	end
	return {
		gesture_ui_name = jbox.ui_text("text_run"),
		property_changes = { [1] = not props[1] },
	}
end

-- A click on REC switches recording on and off. props: [1] REC.
RecButton_Gesture = function(props, di, gsp)
	return { custom_data = {}, handlers = { on_tap = "RecButton_on_tap" } }
end

RecButton_on_tap = function(props, di, gi, cd)
	if inactive(props) then
		return {}
	end
	return {
		gesture_ui_name = jbox.ui_text("text_rec"),
		property_changes = { [1] = not props[1] },
	}
end

-- A click on a pattern button selects that pattern: Reason's own pattern
-- index, from 0. props: [1] the selected pattern's index.
local function select_pattern(index)
	return {
		gesture_ui_name = jbox.ui_text("text_select_pattern"),
		property_changes = { [1] = index },
	}
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

PatternButton1_on_tap = function() return select_pattern(0) end
PatternButton2_on_tap = function() return select_pattern(1) end
PatternButton3_on_tap = function() return select_pattern(2) end
PatternButton4_on_tap = function() return select_pattern(3) end
PatternButton5_on_tap = function() return select_pattern(4) end
PatternButton6_on_tap = function() return select_pattern(5) end
PatternButton7_on_tap = function() return select_pattern(6) end
PatternButton8_on_tap = function() return select_pattern(7) end
