// Malvado 2; SPDX-License-Identifier: Apache-2.0
use crate::engine::{Runtime, result};
use crate::task::contain;
use crate::{Error, Handle};
use malvado_sys as sys;
use std::ffi::c_void;
use std::marker::PhantomData;
use std::ops::Deref;
use std::ptr::NonNull;
use std::rc::Rc;

/// An owned native resource reference, separate from its context/process owner.
/// Shared access remains valid after the owner or context ends. T must own its
/// data; platform services are not extended by this generic reference.
pub struct Resource<T: 'static> {
    native: NonNull<sys::MvResource>,
    runtime: Rc<Runtime>,
    value: PhantomData<T>,
}
impl<T: 'static> Resource<T> {
    pub(crate) fn create(
        runtime: &Rc<Runtime>,
        owner: Option<&Handle>,
        value: T,
    ) -> Result<Self, Error> {
        let ctx = runtime.live()?;
        if let Some(owner) = owner
            && !Rc::ptr_eq(runtime, &owner.inner.runtime)
        {
            return Err(Error::ForeignContext);
        }
        let data = Box::into_raw(Box::new(value));
        let mut native = std::ptr::null_mut();
        let owner = owner
            .map(|h| &h.inner.reference as *const _)
            .unwrap_or(std::ptr::null());
        // SAFETY: value is stable and transferred only on native success.
        let status = unsafe {
            sys::mv_resource_create(ctx, owner, data.cast(), Some(destroy::<T>), &mut native)
        };
        if let Err(error) = result(status) {
            // SAFETY: failed create does not own data; ordinary Rust drops it.
            unsafe {
                drop(Box::from_raw(data));
            }
            return Err(error);
        }
        // SAFETY: a new native resource has zero independent references, so
        // retaining the first one cannot overflow or invoke a callback.
        let status = unsafe { sys::mv_resource_retain(native) };
        result(status)?;
        Ok(Self {
            native: NonNull::new(native).expect("successful resource creation"),
            runtime: runtime.clone(),
            value: PhantomData,
        })
    }
    pub fn try_clone(&self) -> Result<Self, Error> {
        // SAFETY: this retained reference keeps the native resource live.
        result(unsafe { sys::mv_resource_retain(self.native.as_ptr()) })?;
        Ok(Self {
            native: self.native,
            runtime: self.runtime.clone(),
            value: PhantomData,
        })
    }
    /// Move the native owner reference. None selects this resource's context.
    /// Retained references do not change. Failure preserves ownership.
    pub fn transfer_to(&self, owner: Option<&Handle>) -> Result<(), Error> {
        self.runtime.live()?;
        if let Some(owner) = owner
            && !Rc::ptr_eq(&self.runtime, &owner.inner.runtime)
        {
            return Err(Error::ForeignContext);
        }
        let owner = owner
            .map(|h| &h.inner.reference as *const _)
            .unwrap_or(std::ptr::null());
        // SAFETY: both retained resource and optional context-bound owner live.
        result(unsafe { sys::mv_resource_transfer(self.native.as_ptr(), owner) })
    }
}
impl<T: 'static> Deref for Resource<T> {
    type Target = T;
    fn deref(&self) -> &T {
        // SAFETY: this reference retains the Box<T>; no mutable T access is
        // exposed and native ownership changes cannot release our reference.
        unsafe { &*sys::mv_resource_data(self.native.as_ptr()).cast::<T>() }
    }
}
impl<T: 'static> Drop for Resource<T> {
    fn drop(&mut self) {
        // SAFETY: pair this owned reference with exactly one native release.
        unsafe {
            sys::mv_resource_release(self.native.as_ptr());
        }
    }
}
unsafe extern "C" fn destroy<T>(data: *mut c_void) {
    // SAFETY: native final release calls exactly once for the transferred Box.
    let _ = contain(|| unsafe {
        drop(Box::from_raw(data.cast::<T>()));
    });
}

/// Resource values own their data and have shared-only access.
/// ```compile_fail
/// let game = malvado::Game::new().unwrap();
/// let mut resource = game.resource(vec![1]).unwrap();
/// let retained = resource.try_clone().unwrap();
/// resource.push(2); // no DerefMut while retained references can exist
/// ```
/// Borrowed caller data cannot outlive its native resource registration.
/// ```compile_fail
/// let game = malvado::Game::new().unwrap();
/// let value = String::from("local");
/// let resource = game.resource(&value).unwrap();
/// ```
/// Native resource release stays on the owner thread.
/// ```compile_fail
/// let game = malvado::Game::new().unwrap();
/// let resource = game.resource(42).unwrap();
/// std::thread::spawn(move || drop(resource));
/// ```
mod ownership_contract {}
