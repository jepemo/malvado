# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

set(MALVADO_BASE_VERSION "2.1.0")
set(MALVADO_PRERELEASE "")
if(MALVADO_PRERELEASE)
  set(MALVADO_VERSION_STRING "${MALVADO_BASE_VERSION}-${MALVADO_PRERELEASE}")
else()
  set(MALVADO_VERSION_STRING "${MALVADO_BASE_VERSION}")
endif()
