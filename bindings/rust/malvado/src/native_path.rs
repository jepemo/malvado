// Malvado 2; SPDX-License-Identifier: Apache-2.0
use crate::Error;
#[cfg(unix)]
use std::os::unix::ffi::{OsStrExt, OsStringExt};
use std::{
    ffi::CString,
    path::{Path, PathBuf},
};

pub(crate) fn native_path(path: &Path) -> Result<CString, Error> {
    #[cfg(unix)]
    let bytes = path.as_os_str().as_bytes();
    #[cfg(windows)]
    let bytes = path.to_str().ok_or(Error::InvalidArgument)?.as_bytes();
    CString::new(bytes).map_err(|_| Error::InvalidArgument)
}

pub(crate) fn path_from_bytes(bytes: &[u8]) -> Result<PathBuf, Error> {
    #[cfg(unix)]
    let path = std::ffi::OsString::from_vec(bytes.to_vec());
    #[cfg(windows)]
    let path = std::str::from_utf8(bytes).map_err(|_| Error::InvalidArgument)?;
    Ok(PathBuf::from(path))
}
