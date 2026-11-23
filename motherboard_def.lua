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

custom_properties = jbox.property_set{
	document_owner = {
		properties = {
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
		},
	},

	rtc_owner = {
		properties = {
			instance = jbox.native_object{},
		},
	},

	-- Film strip frames, set by the realtime code.
	rt_owner = {
		properties = {
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
		},
	},
}

patterns = {
	num_patterns = 8,
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
