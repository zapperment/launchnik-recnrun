#include <cstring>
#include "Jukebox.h"
#include "RecNRun.h"

void* JBox_Export_CreateNativeObject(const char iOperation[], const TJBox_Value iParams[], TJBox_UInt32 iCount) {
	if (std::strcmp(iOperation, "Instance") == 0 && iCount >= 1) {
		// iParams[0] is the sample rate, passed by init_instance in
		// realtime_controller.lua.
		return new CRecNRun(JBox_GetNumber(iParams[0]));
	}
	return nullptr;
}

void JBox_Export_RenderRealtime(void* iPrivateState, const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount) {
	if (iPrivateState == nullptr) {
		return;
	}
	static_cast<CRecNRun*>(iPrivateState)->RenderBatch(iPropertyDiffs, iDiffCount);
}

// The custom displays are transparent click areas with Lua gestures
// (display.lua), which also keep recorded notes; nothing is drawn or handled
// here.
void JBox_Export_Draw(const TJBox_DisplayArgs* /*iArgs*/) {}

void JBox_Export_Gesture(TJBox_GestureArgs* /*iArgs*/) {}

void JBox_Export_DisplaySetup(const TJBox_DisplayArgs* /*iArgs*/) {}

void JBox_Export_Notify(TJBox_NotifyArgs* /*iArgs*/) {}
