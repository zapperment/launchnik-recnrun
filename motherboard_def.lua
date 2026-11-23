format_version = "3.0"

-- Vocabulary follows CONTEXT.md.
--
-- The selected pattern is Reason's own /transport/pattern_index (see
-- `patterns` below), so the device has no property of its own for it.
--
-- Property tags, as used by Tags.h:
--   ON:                                    1
--   RUN, pressed (a change either way is one press):  2
--   REC:                                   3
--   film strip frame of the pattern buttons:  10
--   film strip frame of the RUN button:       11
--   film strip frame of the REC button:       12
--   patterns with notes not yet kept, one bit each:  13
--   kept patterns 1 to 8:                  21 to 28
--
-- Recorded notes are kept in the strings pattern_1 to pattern_8, which only a
-- click on the panel can write (display.lua): the realtime code shows each
-- pattern as it holds it in the strings pattern_<n>_part_<k>, and a click
-- copies those over. RecNRun.h describes the text.

local PATTERN_COUNT = 8
-- A pattern's text has at most 1 + 1,000 x 10 characters. Recon allows at
-- most 2,048 characters in a realtime string, so it is shown in five parts.
local PART_COUNT = 5
local PART_SIZE = 2048

local document_properties = {
	on = jbox.boolean{
		property_tag = 1,
		default = true,
		persistence = "song",
		ui_name = jbox.ui_text("text_on"),
		ui_type = jbox.ui_selector{ jbox.ui_text("text_off_value"), jbox.ui_text("text_on_value") },
	},
	-- Running is the realtime code's to decide (it follows Reason's
	-- transport), so this only carries presses of the RUN button: each
	-- change, either way, is one press. Not automatable, as the SDK
	-- requires for Run.
	run = jbox.boolean{
		property_tag = 2,
		default = false,
		persistence = "none",
		ui_name = jbox.ui_text("text_run"),
		ui_type = jbox.ui_selector{ jbox.ui_text("text_off_value"), jbox.ui_text("text_on_value") },
	},
	-- Kept while the device is off or bypassed, when REC is inactive.
	rec = jbox.boolean{
		property_tag = 3,
		default = false,
		persistence = "none",
		ui_name = jbox.ui_text("text_rec"),
		ui_type = jbox.ui_selector{ jbox.ui_text("text_off_value"), jbox.ui_text("text_on_value") },
	},
}

-- Saved with the song and the patch.
for pattern = 1, PATTERN_COUNT do
	document_properties["pattern_" .. pattern] = jbox.string{
		property_tag = 20 + pattern,
		default = "",
		ui_name = jbox.ui_text("text_pattern_" .. pattern),
	}
end

-- Film strip frames and the patterns as the realtime code holds them.
local realtime_properties = {
	-- 0 when no pattern is selected, else 1 to 8 for the selected one.
	pattern_light_frame = jbox.number{
		property_tag = 10,
		default = 1,
		steps = 9,
		ui_name = jbox.ui_text("text_pattern_light_frame"),
		ui_type = jbox.ui_linear{ min = 0, max = 8, units = { { decimals = 0 } } },
	},
	-- 0 not running, 1 running, 2 inactive (while the device is off or
	-- bypassed).
	run_light_frame = jbox.number{
		property_tag = 11,
		default = 0,
		steps = 3,
		ui_name = jbox.ui_text("text_run_light_frame"),
		ui_type = jbox.ui_linear{ min = 0, max = 2, units = { { decimals = 0 } } },
	},
	-- 0 not recording, 1 recording, 2 inactive (while the device is off
	-- or bypassed).
	rec_light_frame = jbox.number{
		property_tag = 12,
		default = 0,
		steps = 3,
		ui_name = jbox.ui_text("text_rec_light_frame"),
		ui_type = jbox.ui_linear{ min = 0, max = 2, units = { { decimals = 0 } } },
	},
	-- Bit n - 1 is set while pattern n has notes not yet kept.
	unkept = jbox.number{
		property_tag = 13,
		default = 0,
		steps = 256,
		ui_name = jbox.ui_text("text_unkept"),
		ui_type = jbox.ui_linear{ min = 0, max = 255, units = { { decimals = 0 } } },
	},
}

for pattern = 1, PATTERN_COUNT do
	for part = 1, PART_COUNT do
		realtime_properties["pattern_" .. pattern .. "_part_" .. part] = jbox.string{
			max_size = PART_SIZE,
		}
	end
end

custom_properties = jbox.property_set{
	document_owner = {
		properties = document_properties,
	},

	rtc_owner = {
		properties = {
			instance = jbox.native_object{},
		},
	},

	rt_owner = {
		properties = realtime_properties,
	},
}

patterns = {
	num_patterns = PATTERN_COUNT,
}

midi_implementation_chart = {
	midi_cc_chart = {},
}

-- Named as on Reason's own players. Reason adds Pattern Select, Device Name,
-- Patch Name and the patch selection items by itself.
remote_implementation_chart = {
	["/custom_properties/on"] = {
		internal_name = "On",
		short_ui_name = jbox.ui_text("text_on_short"),
		shortest_ui_name = jbox.ui_text("text_on_shortest"),
	},
	["/custom_properties/run"] = {
		internal_name = "Run",
		short_ui_name = jbox.ui_text("text_run_short"),
		shortest_ui_name = jbox.ui_text("text_run_shortest"),
	},
}
