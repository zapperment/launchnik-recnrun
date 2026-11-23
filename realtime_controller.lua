format_version = "1.0"

rtc_bindings = {
	{ source = "/environment/system_sample_rate", dest = "/global_rtc/init_instance" },
}

global_rtc = {
	init_instance = function(source_property_path, new_value)
		local sample_rate = jbox.load_property("/environment/system_sample_rate")
		local new_no = jbox.make_native_object_rw("Instance", { sample_rate })
		jbox.store_property("/custom_properties/instance", new_no)
	end,
}

rt_input_setup = {
	notify = {
		"/note_states/*",
		"/transport/pattern_index",
		"/transport/playing",
		"/transport/request_run",
		"/transport/request_stop",
		"/environment/player_bypassed",
		"/custom_properties/on",
		"/custom_properties/run",
		"/custom_properties/rec",
	},
}

sample_rate_setup = {
	native = {
		22050,
		44100,
		48000,
		88200,
		96000,
		192000,
	},
}
