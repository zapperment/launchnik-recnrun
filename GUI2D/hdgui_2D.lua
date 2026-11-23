format_version = "2.0"

-- Lights first, the click areas afterwards: later widgets are on top.
local front_widgets = {
	jbox.device_name{
		graphics = { node = "DeviceName" },
	},
	jbox.patch_name{
		graphics = { node = "PatchName" },
		text_style = "Arial medium bold font",
		fg_color = { 85, 85, 85 },
		loader_alt_color = { 160, 160, 160 },
		center = true,
	},
	jbox.patch_browse_group{
		graphics = { node = "PatchBrowseGroup" },
	},
	jbox.toggle_button{
		graphics = { node = "OnButton" },
		value = "/custom_properties/on",
	},
	jbox.sequence_meter{
		graphics = { node = "RecLight" },
		value = "/custom_properties/rec_light_frame",
	},
	jbox.custom_display{
		graphics = { node = "RecClick" },
		background = "transparent",
		display_width_pixels = 210,
		display_height_pixels = 215,
		draw_function = "Transparent_Draw",
		gesture_function = "RecButton_Gesture",
		values = { "/custom_properties/rec", "/custom_properties/on", "/environment/player_bypassed" },
	},
	jbox.sequence_meter{
		graphics = { node = "RunLight" },
		value = "/custom_properties/run_light_frame",
	},
	jbox.custom_display{
		graphics = { node = "RunClick" },
		background = "transparent",
		display_width_pixels = 210,
		display_height_pixels = 215,
		draw_function = "Transparent_Draw",
		gesture_function = "RunButton_Gesture",
		values = { "/custom_properties/run", "/custom_properties/on", "/environment/player_bypassed" },
	},
	jbox.sequence_meter{
		graphics = { node = "PatternLights" },
		value = "/custom_properties/pattern_light_frame",
	},
}

for pattern = 1, 8 do
	table.insert(front_widgets, jbox.custom_display{
		graphics = { node = "PatternClick" .. pattern },
		background = "transparent",
		display_width_pixels = 100,
		display_height_pixels = 180,
		draw_function = "Transparent_Draw",
		gesture_function = "PatternButton" .. pattern .. "_Gesture",
		values = { "/transport/pattern_index" },
	})
end

front = jbox.panel{
	graphics = { node = "Bg" },
	widgets = front_widgets,
}

back = jbox.panel{
	graphics = { node = "Bg" },
	widgets = {
		jbox.device_name{
			graphics = { node = "DeviceName" },
		},
	},
}
