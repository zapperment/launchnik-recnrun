# Launchnik Rec'n'Run

A Player rack extension for Reason that records notes into **patterns** and plays them back. It is a companion to Launchnik, which will select its patterns and control its recording, so that clips can be recorded from a Launchpad assigned to Launchnik.

## Language

**Pattern**:
One of eight note sequences held in the device. These are Reason's own player patterns, so they can also be chosen from a pattern lane in the sequencer.
_Avoid_: Clip (that is Launchnik's word for what a launch button holds), slot

**Selected pattern**:
The **pattern** that **REC** records into and **RUN** plays, chosen with the pattern buttons 1 to 8 or by a pattern lane. At most one pattern is selected at a time; none is while a pattern lane has no pattern clip at the play position. A new device has pattern 1 selected. Reason decides how it is saved.

**REC**:
Records the notes the device receives into the **selected pattern**. At first the notes come from a MIDI controller assigned to the device; later they will also come from Launchnik, which passes on the notes of the controller assigned to it. Inactive while the device is off or bypassed: it ignores clicks, and keeps whether it was on for when the device is switched on again, whatever the transport did meanwhile. Never saved, so a song never opens while recording.
_Avoid_: Arm, record mode

**Running**:
Said of the device while it plays the **selected pattern**. It starts and stops with Reason's transport, every time, and **RUN** starts or pauses it on its own. It is not saved: a song always opens with the device not running. While the device is off or bypassed it is not running and ignores **RUN**.
_Avoid_: Playing (that is Reason's transport)

**RUN**:
The button that shows whether the device is **running**. A press starts it while Reason is stopped, to hear a pattern on its own, and pauses it while Reason plays; the next transport start runs it again. Inactive while the device is off or bypassed. Available to a control surface, but not automatable.
_Avoid_: Play, start

**ON**:
The player's on/off button. While it is off the device passes on the notes it receives and does nothing else, and **RUN** looks inactive. Switched on again, the device is **running** if Reason is playing. Saved with the song, not with the **patch**, and available to a control surface. A new device has it on.
_Avoid_: Bypass, power

**Patch**:
The device settings saved in a `.repatch` file and loaded with the patch browser. For now, the device has none of its own: **ON** is saved only with the song, **REC** and **running** not at all.

## Relationships

- **Launchnik** will select the **selected pattern** and switch **REC**, and pass on notes to be recorded, through CV.
