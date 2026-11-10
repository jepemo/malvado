// Malvado 2; SPDX-License-Identifier: Apache-2.0
use crate::Error;
use std::future::Future;
use std::panic::{AssertUnwindSafe, catch_unwind};
use std::pin::Pin;
use std::sync::{
    Arc,
    atomic::{AtomicBool, Ordering},
};
use std::task::{Context, Poll, Wake, Waker};

struct Readiness {
    ready: AtomicBool,
    alive: AtomicBool,
}
impl Wake for Readiness {
    fn wake(self: Arc<Self>) {
        self.wake_by_ref();
    }
    fn wake_by_ref(self: &Arc<Self>) {
        if self.alive.load(Ordering::Acquire) {
            self.ready.store(true, Ordering::Release);
        }
    }
}

/// Suspend until the next eligible native activation. This primitive does not
/// advance time, poll recursively, or run a separate executor.
pub struct Frame {
    yielded: bool,
}
impl Frame {
    pub(crate) fn new() -> Self {
        Self { yielded: false }
    }
}
impl Future for Frame {
    type Output = ();
    fn poll(mut self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<()> {
        if self.yielded {
            return Poll::Ready(());
        }
        self.yielded = true;
        cx.waker().wake_by_ref();
        Poll::Pending
    }
}

pub(crate) enum Step {
    Yield,
    Done,
    Error,
}

type ProcessFuture = Pin<Box<dyn Future<Output = Result<(), Error>>>>;

pub(crate) struct Task {
    future: Option<ProcessFuture>,
    readiness: Arc<Readiness>,
}
impl Task {
    pub(crate) fn empty() -> Self {
        Self {
            future: None,
            readiness: Arc::new(Readiness {
                ready: AtomicBool::new(true),
                alive: AtomicBool::new(true),
            }),
        }
    }
    pub(crate) fn install(&mut self, future: impl Future<Output = Result<(), Error>> + 'static) {
        self.future = Some(Box::pin(future));
    }
    pub(crate) fn poll(&mut self) -> Step {
        if !self.readiness.ready.swap(false, Ordering::AcqRel) {
            return Step::Yield;
        }
        let waker = Waker::from(self.readiness.clone());
        let mut context = Context::from_waker(&waker);
        let result = contain(|| {
            self.future
                .as_mut()
                .expect("installed process future")
                .as_mut()
                .poll(&mut context)
        });
        match result {
            Ok(Poll::Pending) => Step::Yield,
            Ok(Poll::Ready(Ok(()))) => {
                self.readiness.alive.store(false, Ordering::Release);
                Step::Done
            }
            _ => {
                self.readiness.alive.store(false, Ordering::Release);
                Step::Error
            }
        }
    }
}
impl Drop for Task {
    fn drop(&mut self) {
        self.readiness.alive.store(false, Ordering::Release);
    }
}

// A panic payload can itself panic in Drop. Release normal payloads inside a
// second boundary; forget only the exceptional secondary panic payload.
pub(crate) fn contain<T>(operation: impl FnOnce() -> T) -> Result<T, ()> {
    catch_unwind(AssertUnwindSafe(operation)).map_err(|payload| {
        if let Err(secondary) = catch_unwind(AssertUnwindSafe(|| drop(payload))) {
            std::mem::forget(secondary);
        }
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::cell::{Cell, RefCell};
    use std::future::poll_fn;
    use std::rc::Rc;

    struct Guard(Rc<Cell<u32>>);
    impl Drop for Guard {
        fn drop(&mut self) {
            self.0.set(self.0.get() + 1);
        }
    }

    #[test]
    fn pinned_locals_and_borrows_survive_frame_waits() {
        let observed = Rc::new(Cell::new(0));
        let result = observed.clone();
        let mut task = Task::empty();
        task.install(async move {
            let mut local = 41;
            let borrowed = &mut local;
            Frame::new().await;
            *borrowed += 1;
            Frame::new().await;
            result.set(*borrowed);
            Ok(())
        });
        assert!(matches!(task.poll(), Step::Yield));
        // Move the adapter; the future allocation itself must remain pinned.
        let mut moved = Box::new(task);
        assert!(matches!(moved.poll(), Step::Yield));
        assert_eq!(observed.get(), 0);
        assert!(matches!(moved.poll(), Step::Done));
        assert_eq!(observed.get(), 42);
    }

    #[test]
    fn pending_without_wake_does_not_poll_again_and_cross_thread_wake_is_local() {
        let polls = Rc::new(Cell::new(0));
        let count = polls.clone();
        let saved = Rc::new(RefCell::new(None));
        let output = saved.clone();
        let mut task = Task::empty();
        task.install(poll_fn(move |cx| {
            count.set(count.get() + 1);
            *output.borrow_mut() = Some(cx.waker().clone());
            if count.get() == 1 {
                Poll::Pending
            } else {
                Poll::Ready(Ok(()))
            }
        }));
        for _ in 0..100 {
            assert!(matches!(task.poll(), Step::Yield));
        }
        assert_eq!(polls.get(), 1);
        let waker = saved.borrow().as_ref().unwrap().clone();
        std::thread::spawn(move || {
            waker.wake_by_ref();
            waker.wake();
        })
        .join()
        .unwrap();
        assert_eq!(polls.get(), 1);
        assert!(matches!(task.poll(), Step::Done));
        assert_eq!(polls.get(), 2);
        let waker = saved.borrow().as_ref().unwrap().clone();
        drop(task);
        waker.wake();
    }

    #[test]
    fn wake_during_poll_is_deferred_and_coalesced() {
        let polls = Rc::new(Cell::new(0));
        let count = polls.clone();
        let mut task = Task::empty();
        task.install(poll_fn(move |cx| {
            count.set(count.get() + 1);
            cx.waker().wake_by_ref();
            cx.waker().wake_by_ref();
            if count.get() == 3 {
                Poll::Ready(Ok(()))
            } else {
                Poll::Pending
            }
        }));
        assert!(matches!(task.poll(), Step::Yield));
        assert_eq!(polls.get(), 1);
        assert!(matches!(task.poll(), Step::Yield));
        assert_eq!(polls.get(), 2);
        assert!(matches!(task.poll(), Step::Done));
    }

    #[test]
    fn cancellation_drops_owned_state_once_before_or_after_poll() {
        for activated in [false, true] {
            let drops = Rc::new(Cell::new(0));
            let guard = Guard(drops.clone());
            let mut task = Task::empty();
            task.install(async move {
                let _guard = guard;
                std::future::pending::<()>().await;
                Ok(())
            });
            if activated {
                assert!(matches!(task.poll(), Step::Yield));
            }
            drop(task);
            assert_eq!(drops.get(), 1);
        }
    }

    #[test]
    fn panic_and_returned_error_become_process_failure() {
        let mut task = Task::empty();
        task.install(async { panic!("poll failure") });
        assert!(matches!(task.poll(), Step::Error));
        let mut task = Task::empty();
        task.install(async { Err(Error::Closed) });
        assert!(matches!(task.poll(), Step::Error));
    }

    #[test]
    fn drop_panic_does_not_escape_containment() {
        struct BadDrop;
        impl Drop for BadDrop {
            fn drop(&mut self) {
                panic!("drop failure");
            }
        }
        let mut task = Task::empty();
        let value = BadDrop;
        task.install(async move {
            let _value = value;
            Ok(())
        });
        assert!(contain(|| drop(task)).is_err());
    }
}
