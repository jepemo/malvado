# Resource lifetime contract

This is the alpha.5 target for resource lifetime. The [alpha.5 checklist](../versions/2.0.0-alpha.5.md) tracks the remaining implementation and tests. The rules below do not claim that every current binding implements them yet.

## Ownership and sharing

A resource created during a process callback belongs to that process. A resource created outside a process belongs to the game context. Creation failure leaves ownership with the caller and does not create a live resource. Process ownership does not change when a process is reparented or ends. Cleanup releases the owner's reference once. The resource stays alive until all independent references and pending work release it.

In Lua, assigning a resource to a table, process field, or another variable shares the same object. It does not copy the resource or transfer its owner. Each reachable wrapper holds a reference independently of the owner. The Lua API does not currently provide an explicit resource release or ownership transfer operation. Drop Lua references to allow collection; do not depend on when collection runs. A queued draw, persistent text object, or active playback holds the references it needs. These references are released when the work is removed or finishes.

In C, `mv_resource_create` registers data and a destructor with a context. A null owner means the game context; a non-null owner is a live process reference from that context. The output is borrowed. Use `mv_resource_retain` to keep a reference, and pair each successful retain with `mv_resource_release`. Use `mv_resource_transfer` to move the owner reference to another live process in that context or to the game context. A failed transfer leaves the owner and references unchanged. Release never releases the owner reference. See the [C contract](c-contract.md) for errors and context lifetime.

## Resource families

| Family | Acquisition and assignment | Retention and release |
| --- | --- | --- |
| Image | Load a PNG with a window. A Lua image wrapper can be shared by assignment. | A queued image draw retains the image until the draw is replaced or removed. The owner and Lua wrapper references are separate. |
| Atlas | Parse metadata for an image. A Lua atlas keeps the image used to create it. The C atlas parser owns metadata only; its caller owns the image. | Pending sprite draws retain what they need to draw. Release the atlas metadata and image references separately. |
| Font | `font` uses the default font; `load_font` loads a custom font with a window. Lua font tables can be shared. | A text object or pending text draw keeps its font alive. A custom font is destroyed after its owner, wrappers, and pending work release it. |
| Text | `write` creates persistent text and stores its font. `change_text` assigns the replacement font without changing the font's owner. | The creating process owns the text, or the game context owns it outside a process. `delete_text` explicitly removes it and releases its font reference. Owner cleanup removes remaining text once. |
| Sound | Load a WAV sound or clone an instance. A clone has independent playback state and shares sample data. | Playing or paused instances retain their data. Stop or completion releases playback retention. Each clone and pending playback releases its own reference. |
| Music | Load a WAV or OGG stream with an audio device. Assignment shares the stream. | Playing or paused streams retain their data. Stop or completion releases playback retention. Release the stream only after its owner, wrappers, and playback release it. |

A renderer or audio backend must not access a native object after its last reference is released. If work is asynchronous, it must retain an independent reference until completion. A resource cannot be released early because its creating process ends.

## Direct C platform objects and shutdown

`MvImage`, `MvFont`, `MvSound`, and `MvMusic` created through `malvado/platform.h` are owned by the C caller. They are not automatically registered with a context. The caller can manage their lifetime directly or register them with `mv_resource_create` and a matching destructor. Do not destroy the same native object through both paths. `MvAtlas` metadata is also caller-owned and has its own destructor.

Use a context and its resource manager on one owner thread. Use window, image, and font operations on the window thread, and audio operations on the audio device owner thread. Calls on one context must not race context destruction. A retained reference does not make a graphics object usable after its window or an audio object usable after its device closes. Direct C callers must release those objects before closing the corresponding platform service.

On runner shutdown, stop process execution and reject new spawns. Close process state and release process-owned resources, then release game-owned resources and Lua references. Finish or discard queued drawing and audio work before closing the window and audio device. A retained C reference can outlive a context only when its destructor and underlying data remain safe after the context and relevant platform services close. The caller is responsible for that condition.
