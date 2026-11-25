// Malvado 2; SPDX-License-Identifier: Apache-2.0
use crate::{
    Error, Game,
    audio::Audio,
    graphic::BLACK,
    window::{Key, Window},
};
use std::{rc::Rc, time::Instant};

impl Game {
    /// Create a game and its window on the application main thread.
    /// Dimensions must be positive and fit the native signed size.
    pub fn windowed(width: u32, height: u32, title: &str) -> Result<Self, Error> {
        let width = i32::try_from(width).map_err(|_| Error::InvalidArgument)?;
        let height = i32::try_from(height).map_err(|_| Error::InvalidArgument)?;
        let mut game = Self::new()?;
        game.attach_window(Window::open(width, height, title)?);
        Ok(game)
    }

    /// Open the managed audio device on demand. The runner services this
    /// device every render iteration. Call before moving the game into run.
    pub fn audio(&mut self) -> Result<Rc<Audio>, Error> {
        if self.window.is_none() {
            return Err(Error::Platform("no managed window"));
        }
        if let Some(audio) = &self.audio {
            return Ok(audio.clone());
        }
        let audio = Rc::new(Audio::open()?);
        self.audio = Some(audio.clone());
        Ok(audio)
    }

    /// Run a windowed game until close, Escape, a quit request, or the end of
    /// its last process. Errors propagate after native safe-point cleanup.
    /// The game is consumed so all remaining processes are cancelled on exit.
    pub fn run(mut self) -> Result<(), Error> {
        let window = self
            .window
            .clone()
            .ok_or(Error::Platform("no managed window"))?;
        let mut previous = Instant::now();
        while !self.quit_requested() && self.process_count() != 0 && !window.should_close() {
            let now = Instant::now();
            for _ in 0..self.advance_time((now - previous).as_secs_f64())? {
                window.input_tick();
                if window.key_down(Key::Escape) || window.key_pressed(Key::Escape) {
                    return Ok(());
                }
                self.tick()?;
                if self.quit_requested() || self.process_count() == 0 {
                    return Ok(());
                }
            }
            previous = now;
            if let Some(audio) = &self.audio {
                audio.update()?;
            }
            let frame = window.frame(BLACK)?;
            frame.process_graphics(&self)?;
            frame.texts(&self)?;
        }
        Ok(())
    }
}
