# Empty WAV fixture

`empty.wav` is a generated 44-byte PCM header with no samples. It verifies
cleanup when a decoder opens a file but the stream has no playable frames.
It contains no third-party content and uses the repository license.
Generate it with Python's `wave` module: mono, 16-bit, 22050 Hz, zero frames.
