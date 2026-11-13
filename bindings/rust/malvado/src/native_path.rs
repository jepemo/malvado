// Malvado 2; SPDX-License-Identifier: Apache-2.0
use crate::Error;
#[cfg(unix)]
use std::os::unix::ffi::OsStrExt;
use std::{ffi::CString, path::Path};

pub(crate) fn native_path(path: &Path) -> Result<CString, Error> {
    #[cfg(unix)]
    let bytes = path.as_os_str().as_bytes();
    #[cfg(windows)]
    let bytes = path.to_str().ok_or(Error::InvalidArgument)?.as_bytes();
    CString::new(bytes).map_err(|_| Error::InvalidArgument)
}
