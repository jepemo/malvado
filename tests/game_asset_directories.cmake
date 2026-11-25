# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

file(MAKE_DIRECTORY "${WORK}/game ñ" "${WORK}/caller")
file(COPY "${SHEET}" DESTINATION "${WORK}/game ñ")
file(WRITE "${WORK}/game ñ/main.lua" [=[
local mv = require("malvado")
local atlas = mv.load_asset("sheet.json")
assert(mv.sprite_id(atlas, "one") == 1)
]=])

function(run_asset_case name directory executable)
  execute_process(COMMAND "${executable}" ${ARGN}
    WORKING_DIRECTORY "${directory}" RESULT_VARIABLE status ERROR_VARIABLE diagnostic)
  if(NOT status EQUAL 0)
    message(FATAL_ERROR "${name}: ${diagnostic}")
  endif()
endfunction()

run_asset_case(project "${WORK}" "${CLI}" run --headless "game ñ")
run_asset_case(default "${WORK}/game ñ" "${CLI}" run --headless)
run_asset_case(script "${WORK}" "${CLI}" run --headless "game ñ/main.lua")
run_asset_case(absolute "${WORK}/caller" "${CLI}" run --headless "${WORK}/game ñ/main.lua")
run_asset_case(direct "${WORK}/caller" "${RUNNER}" --headless "${WORK}/game ñ/main.lua")
run_asset_case(direct_relative "${WORK}" "${RUNNER}" --headless "game ñ/main.lua")

file(MAKE_DIRECTORY "${WORK}/game ñ/assets é/characters" "${WORK}/game ñ/unused")
file(COPY "${SHEET}" DESTINATION "${WORK}/game ñ/assets é/characters")
configure_file("${SHEET}" "${WORK}/caller/caller-only.json" COPYONLY)
file(WRITE "${WORK}/caller/cwd-marker.txt" "caller")
file(WRITE "${WORK}/game ñ/sidecar.lua" "return 'entry'")
file(WRITE "${WORK}/game ñ/assets é/sidecar.lua" "error('wrong module directory')")
file(WRITE "${WORK}/game ñ/settings.lua" [=[
local mv = require("malvado")
mv.import_globals()
assert(set_assets_dir == mv.set_assets_dir and get_assets_dir == mv.get_assets_dir)
local root = get_assets_dir()
assert(root:gsub("\\", "/"):match("/game ñ$"), root)
assert(not pcall(load_asset, "caller-only.json"))
assert(require("sidecar") == "entry")
set_assets_dir("unused")
assert(get_assets_dir():match("/unused$"))
set_assets_dir("assets é")
local selected = get_assets_dir()
assert(selected == root .. "/assets é")
for _, invalid in ipairs({"", "assets\0bad", 1, false, {}}) do
  assert(not pcall(set_assets_dir, invalid))
  assert(get_assets_dir() == selected)
end
local ok, err = pcall(load_asset, "missing.json")
assert(not ok and err:find(selected .. "/missing.json", 1, true), err)
local retained = load_asset("characters/sheet.json")
set_assets_dir(".")
assert(get_assets_dir() == root)
assert(sprite_id(retained, "one") == 1)
set_assets_dir(selected)
assert(get_assets_dir() == selected)
set_assets_dir()
assert(get_assets_dir() == root)
assert(sprite_id(load_asset(selected .. "/characters/sheet.json"), "one") == 1)
set_assets_dir("unused")
return {
  assets_dir = "assets é",
  init = function()
    assert(get_assets_dir() == selected)
    assert(sprite_id(load_asset("characters/sheet.json"), "one") == 1)
    package.loaded.sidecar = nil
    assert(require("sidecar") == "entry")
    local marker = assert(io.open("cwd-marker.txt"))
    assert(marker:read("a") == "caller")
    marker:close()
  end,
}
]=])
run_asset_case(settings "${WORK}/caller" "${RUNNER}" --headless "${WORK}/game ñ/settings.lua")

if(NOT CMAKE_HOST_WIN32)
  file(MAKE_DIRECTORY "${WORK}/real/sub")
  configure_file("${SHEET}" "${WORK}/real/linked.json" COPYONLY)
  file(CREATE_LINK "${WORK}/real/sub" "${WORK}/game ñ/link" SYMBOLIC)
  file(WRITE "${WORK}/game ñ/symlink.lua" [=[
local mv = require("malvado")
assert(mv.sprite_id(mv.load_asset("link/../linked.json"), "one") == 1)
]=])
  run_asset_case(symlink "${WORK}/caller" "${RUNNER}" --headless "${WORK}/game ñ/symlink.lua")
endif()
