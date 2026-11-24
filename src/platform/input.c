/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */

#include <string.h>
#include "malvado/platform.h"
#include "input_internal.h"

/* The platform owns one window. Events accumulate between logical ticks;
 * queries never mutate or consume the committed snapshot. */
typedef struct {
    unsigned pressed, released;
} ButtonEdges;

typedef struct {
    ButtonEdges committed, pending;
    unsigned held;
} ButtonSnapshot;

static int input_open;
static struct {
    unsigned char pressed[MV_KEY_BACKSPACE + 1], released[MV_KEY_BACKSPACE + 1];
    unsigned char pending_pressed[MV_KEY_BACKSPACE + 1], pending_released[MV_KEY_BACKSPACE + 1];
    unsigned char held[MV_KEY_BACKSPACE + 1];
} keyboard;
static ButtonSnapshot mouse;
static struct {
    int connected;
    ButtonSnapshot buttons;
} pads[MV_INPUT_PAD_SLOTS];

static int
supported_key(MvKey key)
{
    return (key >= MV_KEY_0 && key <= MV_KEY_9) ||
           (key >= MV_KEY_A && key <= MV_KEY_Z) ||
           (key >= MV_KEY_ESCAPE && key <= MV_KEY_BACKSPACE);
}

void
mv_input_reset(void)
{
    input_open = 0;
    memset(&keyboard, 0, sizeof(keyboard));
    memset(&mouse, 0, sizeof(mouse));
    memset(pads, 0, sizeof(pads));
}

void
mv_input_open(void)
{
    mv_input_reset();
    input_open = 1;
}

void
mv_keyboard_sample(MvKey key, int pressed, int released)
{
    if (!input_open || !supported_key(key))
        return;
    keyboard.pending_pressed[key] |= pressed != 0;
    keyboard.pending_released[key] |= released != 0;
}

void
mv_keyboard_sample_held(MvKey key, int down)
{
    if (!input_open || !supported_key(key))
        return;
    down = down != 0;
    mv_keyboard_sample(key, down && !keyboard.held[key], !down && keyboard.held[key]);
    keyboard.held[key] = (unsigned char)down;
}

static void
sample_held(ButtonSnapshot *buttons, unsigned held)
{
    buttons->pending.pressed |= held & ~buttons->held;
    buttons->pending.released |= buttons->held & ~held;
    buttons->held = held;
}

static void
commit_buttons(ButtonSnapshot *buttons)
{
    buttons->committed = buttons->pending;
    buttons->pending = (ButtonEdges){0};
}

static unsigned
mouse_bit(MvMouseButton button)
{
    return button >= MV_MOUSE_LEFT && button <= MV_MOUSE_RIGHT ? 1u << button : 0;
}

void
mv_mouse_sample(MvMouseButton button, int pressed, int released)
{
    if (!input_open)
        return;
    unsigned bit = mouse_bit(button);
    if (pressed)
        mouse.pending.pressed |= bit;
    if (released)
        mouse.pending.released |= bit;
}

void
mv_mouse_sample_held(unsigned held)
{
    if (input_open)
        sample_held(&mouse, held);
}

static int
supported_slot(int slot)
{
    return slot >= 0 && slot < MV_INPUT_PAD_SLOTS;
}

void
mv_pad_sample(int slot, int connected, unsigned held)
{
    if (!input_open || !supported_slot(slot))
        return;
    if (!connected) {
        pads[slot].connected = 0;
        pads[slot].buttons = (ButtonSnapshot){0};
    } else if (!pads[slot].connected) {
        pads[slot].connected = 1;
        pads[slot].buttons.held = held;
    } else {
        sample_held(&pads[slot].buttons, held);
    }
}

void
mv_input_tick(void)
{
    memcpy(keyboard.pressed, keyboard.pending_pressed, sizeof(keyboard.pressed));
    memcpy(keyboard.released, keyboard.pending_released, sizeof(keyboard.released));
    memset(keyboard.pending_pressed, 0, sizeof(keyboard.pending_pressed));
    memset(keyboard.pending_released, 0, sizeof(keyboard.pending_released));
    commit_buttons(&mouse);
    for (int slot = 0; slot < MV_INPUT_PAD_SLOTS; ++slot)
        commit_buttons(&pads[slot].buttons);
}

int
mv_key_pressed(MvKey key)
{
    return input_open && supported_key(key) && keyboard.pressed[key];
}

int
mv_key_released(MvKey key)
{
    return input_open && supported_key(key) && keyboard.released[key];
}

int
mv_mouse_pressed(MvMouseButton button)
{
    return (mouse.committed.pressed & mouse_bit(button)) != 0;
}

int
mv_mouse_released(MvMouseButton button)
{
    return (mouse.committed.released & mouse_bit(button)) != 0;
}

static unsigned
pad_bit(MvPadButton button)
{
    return button >= MV_PAD_A && button <= MV_PAD_START ? 1u << button : 0;
}

int
mv_pad_button_pressed(int slot, MvPadButton button)
{
    return supported_slot(slot) && (pads[slot].buttons.committed.pressed & pad_bit(button)) != 0;
}

int
mv_pad_button_released(int slot, MvPadButton button)
{
    return supported_slot(slot) && (pads[slot].buttons.committed.released & pad_bit(button)) != 0;
}

int
mv_pad_button_from_name(const char *name, MvPadButton *button)
{
    static const char *const names[] = {
        "a", "b", "x", "y", "up", "down", "left", "right", "lb", "rb", "back", "start"
    };
    if (!name || !button)
        return 0;
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        if (strcmp(name, names[i]) == 0) {
            *button = (MvPadButton)i;
            return 1;
        }
    }
    return 0;
}

int
mv_pad_axis_from_name(const char *name, MvPadAxis *axis)
{
    static const char *const names[] = {
        "left_x", "left_y", "right_x", "right_y", "left_trigger", "right_trigger"
    };
    if (!name || !axis)
        return 0;
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        if (strcmp(name, names[i]) == 0) {
            *axis = (MvPadAxis)i;
            return 1;
        }
    }
    return 0;
}

int
mv_key_from_name(const char *name, MvKey *key)
{
    static const struct { const char *name; MvKey key; } named[] = {
        {"escape", MV_KEY_ESCAPE}, {"space", MV_KEY_SPACE},
        {"left", MV_KEY_LEFT}, {"right", MV_KEY_RIGHT},
        {"up", MV_KEY_UP}, {"down", MV_KEY_DOWN},
        {"enter", MV_KEY_ENTER}, {"tab", MV_KEY_TAB}, {"backspace", MV_KEY_BACKSPACE}
    };
    if (!name || !key || !name[0])
        return 0;
    if (!name[1]) {
        if (name[0] >= 'a' && name[0] <= 'z') {
            *key = (MvKey)(MV_KEY_A + name[0] - 'a');
            return 1;
        }
        if (name[0] >= '0' && name[0] <= '9') {
            *key = (MvKey)(MV_KEY_0 + name[0] - '0');
            return 1;
        }
    }
    for (size_t i = 0; i < sizeof(named) / sizeof(named[0]); ++i) {
        if (strcmp(name, named[i].name) == 0) {
            *key = named[i].key;
            return 1;
        }
    }
    return 0;
}

int
mv_mouse_button_from_name(const char *name, MvMouseButton *button)
{
    if (!name || !button)
        return 0;
    if (strcmp(name, "left") == 0)
        *button = MV_MOUSE_LEFT;
    else if (strcmp(name, "middle") == 0)
        *button = MV_MOUSE_MIDDLE;
    else if (strcmp(name, "right") == 0)
        *button = MV_MOUSE_RIGHT;
    else
        return 0;
    return 1;
}

size_t
mv_utf8_decode(const char *input, size_t length, uint32_t *codepoint)
{
    if (!input || !codepoint || length == 0)
        return 0;
    const unsigned char *bytes = (const unsigned char *)input;
    uint32_t value;
    size_t count;
    if (bytes[0] <= 0x7f) {
        value = bytes[0];
        count = 1;
    } else if (bytes[0] >= 0xc2 && bytes[0] <= 0xdf) {
        value = bytes[0] & 0x1f;
        count = 2;
    } else if (bytes[0] >= 0xe0 && bytes[0] <= 0xef) {
        value = bytes[0] & 0x0f;
        count = 3;
    } else if (bytes[0] >= 0xf0 && bytes[0] <= 0xf4) {
        value = bytes[0] & 0x07;
        count = 4;
    } else {
        return 0;
    }
    if (length < count)
        return 0;
    for (size_t i = 1; i < count; ++i) {
        if ((bytes[i] & 0xc0) != 0x80)
            return 0;
        value = (value << 6) | (bytes[i] & 0x3f);
    }
    if ((count == 2 && value < 0x80) || (count == 3 && value < 0x800) ||
        (count == 4 && value < 0x10000) || value > 0x10ffff ||
        (value >= 0xd800 && value <= 0xdfff))
        return 0;
    *codepoint = value;
    return count;
}

size_t
mv_utf8_encode(uint32_t codepoint, char output[4])
{
    if (!output || codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff))
        return 0;
    if (codepoint <= 0x7f) {
        output[0] = (char)codepoint;
        return 1;
    }
    if (codepoint <= 0x7ff) {
        output[0] = (char)(0xc0 | (codepoint >> 6));
        output[1] = (char)(0x80 | (codepoint & 0x3f));
        return 2;
    }
    if (codepoint <= 0xffff) {
        output[0] = (char)(0xe0 | (codepoint >> 12));
        output[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        output[2] = (char)(0x80 | (codepoint & 0x3f));
        return 3;
    }
    output[0] = (char)(0xf0 | (codepoint >> 18));
    output[1] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
    output[2] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
    output[3] = (char)(0x80 | (codepoint & 0x3f));
    return 4;
}
