# Delivered C function inventory

Generated from the public headers by `python3 docs/tools/current_c_api.py`.
This is a lexical list of function names, not a complete 2.0 API decision.
It does not certify contracts, examples, tests, types, constants, or Lua parity.
See the linked headers and the [process](process-inventory.md),
[asset](asset-inventory.md), [input](input-inventory.md),
[audio](audio-inventory.md), and [collision](collision.md) guides.

## [animation.h](../../src/include/malvado/animation.h)

- `mv_animation_advance`
- `mv_animation_create`
- `mv_animation_destroy`
- `mv_animation_sample`

## [atlas.h](../../src/include/malvado/atlas.h)

- `mv_atlas_animation_color`
- `mv_atlas_animation_frame`
- `mv_atlas_animation_frame_at`
- `mv_atlas_animation_frame_count`
- `mv_atlas_animation_repeats`
- `mv_atlas_count`
- `mv_atlas_destroy`
- `mv_atlas_find`
- `mv_atlas_find_name`
- `mv_atlas_image`
- `mv_atlas_parse`
- `mv_atlas_parse_aseprite`
- `mv_atlas_pivot_color`

## [camera.h](../../src/include/malvado/camera.h)

- `mv_camera_center`
- `mv_camera_clip`
- `mv_camera_follow_step`
- `mv_camera_init`
- `mv_camera_move`
- `mv_camera_project`
- `mv_camera_set`

## [collision.h](../../src/include/malvado/collision.h)

- `mv_collision_overlap`
- `mv_collision_world_add_rect`
- `mv_collision_world_create`
- `mv_collision_world_destroy`
- `mv_collision_world_move`
- `mv_collision_world_query`
- `mv_collision_world_sweep`
- `mv_process_collider_bind`
- `mv_process_collider_clear`
- `mv_process_collider_find`
- `mv_process_collider_first`
- `mv_process_collider_set`
- `mv_process_collider_test`
- `mv_process_world_move`
- `mv_process_world_move_selected`
- `mv_process_world_query`
- `mv_process_world_query_selected`
- `mv_rect_contains`
- `mv_rect_overlaps`

## [fpg.h](../../src/include/malvado/fpg.h)

- `mv_fpg_at`
- `mv_fpg_copy_json`
- `mv_fpg_copy_png`
- `mv_fpg_copy_rgba`
- `mv_fpg_count`
- `mv_fpg_destroy`
- `mv_fpg_gamma`
- `mv_fpg_json_size`
- `mv_fpg_manifest_at`
- `mv_fpg_manifest_count`
- `mv_fpg_manifest_destroy`
- `mv_fpg_manifest_gamma`
- `mv_fpg_manifest_palette`
- `mv_fpg_manifest_parse`
- `mv_fpg_manifest_point`
- `mv_fpg_manifest_sprite`
- `mv_fpg_manifest_transparent_zero`
- `mv_fpg_palette`
- `mv_fpg_parse`
- `mv_fpg_png_size`
- `mv_fpg_point`
- `mv_fpg_sprite`

## [graphic.h](../../src/include/malvado/graphic.h)

- `mv_graphic_circle`
- `mv_graphic_rect`
- `mv_process_graphic_clear`
- `mv_process_graphic_get`
- `mv_process_graphic_set`

## [malvado.h](../../src/include/malvado/malvado.h)

- `mv_abi_version`
- `mv_cadence_step`
- `mv_context_advance`
- `mv_context_create`
- `mv_context_destroy`
- `mv_context_dropped_seconds`
- `mv_context_runtime_id`
- `mv_context_ticks`
- `mv_message_release`
- `mv_process_angle`
- `mv_process_children`
- `mv_process_count`
- `mv_process_creation_children`
- `mv_process_creation_descendants`
- `mv_process_descendants`
- `mv_process_draw_order`
- `mv_process_find`
- `mv_process_handle_create`
- `mv_process_handle_destroy`
- `mv_process_handle_snapshot`
- `mv_process_info`
- `mv_process_inspection_capture`
- `mv_process_inspection_snapshot`
- `mv_process_kill`
- `mv_process_lineage_snapshot`
- `mv_process_message_count`
- `mv_process_position`
- `mv_process_priority`
- `mv_process_receive`
- `mv_process_ref_create`
- `mv_process_ref_resolve`
- `mv_process_scale`
- `mv_process_send`
- `mv_process_set_angle`
- `mv_process_set_position`
- `mv_process_set_priority`
- `mv_process_set_scale`
- `mv_process_set_type`
- `mv_process_set_z`
- `mv_process_signal`
- `mv_process_signal_creation_tree`
- `mv_process_snapshot`
- `mv_process_spawn`
- `mv_process_spawn_related`
- `mv_process_status`
- `mv_process_tick`
- `mv_process_z`
- `mv_resource_create`
- `mv_resource_data`
- `mv_resource_release`
- `mv_resource_retain`
- `mv_resource_transfer`
- `mv_version_string`

## [platform.h](../../src/include/malvado/platform.h)

- `mv_audio_close`
- `mv_audio_open`
- `mv_draw_state_background`
- `mv_draw_state_color`
- `mv_draw_state_init`
- `mv_fade_active`
- `mv_fade_queue_active`
- `mv_fade_queue_push`
- `mv_fade_queue_step`
- `mv_fade_start`
- `mv_fade_step`
- `mv_font_destroy`
- `mv_font_draw`
- `mv_font_load`
- `mv_font_load_codepoints`
- `mv_font_text_width`
- `mv_image_destroy`
- `mv_image_draw`
- `mv_image_draw_region`
- `mv_image_from_rgba`
- `mv_image_height`
- `mv_image_load`
- `mv_image_region_valid`
- `mv_image_width`
- `mv_input_tick`
- `mv_key_down`
- `mv_key_from_name`
- `mv_key_pressed`
- `mv_key_released`
- `mv_monotonic_time`
- `mv_mouse_button_from_name`
- `mv_mouse_down`
- `mv_mouse_position`
- `mv_music_destroy`
- `mv_music_load`
- `mv_music_pause`
- `mv_music_play`
- `mv_music_playing`
- `mv_music_resume`
- `mv_music_set_volume`
- `mv_music_stop`
- `mv_music_update`
- `mv_pad_axis`
- `mv_pad_axis_from_name`
- `mv_pad_button_down`
- `mv_pad_button_from_name`
- `mv_pad_connected`
- `mv_platform_time`
- `mv_platform_wait`
- `mv_sound_clone`
- `mv_sound_destroy`
- `mv_sound_load`
- `mv_sound_pause`
- `mv_sound_play`
- `mv_sound_playing`
- `mv_sound_resume`
- `mv_sound_set_volume`
- `mv_sound_stop`
- `mv_text_input_next`
- `mv_utf8_decode`
- `mv_utf8_encode`
- `mv_window_begin`
- `mv_window_clip_begin`
- `mv_window_clip_end`
- `mv_window_close`
- `mv_window_draw_box`
- `mv_window_draw_circle`
- `mv_window_draw_fade`
- `mv_window_draw_filled_circle`
- `mv_window_draw_graphic`
- `mv_window_draw_process_graphics`
- `mv_window_draw_text`
- `mv_window_draw_transformed_rect`
- `mv_window_end`
- `mv_window_escape_down`
- `mv_window_flush`
- `mv_window_open`
- `mv_window_scroll_begin`
- `mv_window_scroll_end`
- `mv_window_set_title`
- `mv_window_should_close`
- `mv_window_size`
- `mv_window_space_down`
- `mv_window_text_width`

