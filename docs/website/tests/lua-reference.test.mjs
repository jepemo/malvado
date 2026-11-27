import assert from 'node:assert/strict';
import { test } from 'node:test';
import { checkLuaReference } from '../scripts/check-lua-reference.mjs';
import { publishExample } from '../scripts/examples.mjs';

const key = 'cameras/camera_move';
const forms = ['camera_move', 'mv.camera_move'];
const entries = [{ key, forms }];
const expected = new Set(forms);
const methods = [['camera:move(dx, dy)', 'camera_move(camera, dx, dy)']];
const entry = {
  description: 'Move a camera.',
  signatures: ['camera_move(camera, dx, dy)', 'mv.camera_move(camera, dx, dy)', 'camera:move(dx, dy)'],
  params: [['camera', 'Handle.'], ['dx', 'Horizontal distance.'], ['dy', 'Vertical distance.']],
  returns: 'No values.',
  exampleId: 'reference-cameras',
  exampleSetup: 'Run cameras.lua.',
  exampleResult: 'The program prints the camera position.',
};

test('valid reference covers aliases and methods', () => {
  checkLuaReference(entries, { [key]: entry }, expected, methods);
});

test('missing and stale references fail', () => {
  assert.throws(() => checkLuaReference(entries, {}, expected, methods), /Missing Lua reference entry/);
  assert.throws(() => checkLuaReference(entries, { [key]: entry, obsolete: entry }, expected, methods),
    /Stale Lua reference entry/);
  assert.throws(() => checkLuaReference(entries, { [key]: entry }, new Set([...forms, 'mv.new_call']), methods),
    /Public Lua call has no reference page/);
});

test('missing or inconsistent signature forms fail', () => {
  const withoutMethod = { ...entry, signatures: entry.signatures.slice(0, 2) };
  assert.throws(() => checkLuaReference(entries, { [key]: withoutMethod }, expected, methods),
    /Missing Lua method reference/);
  const wrongArity = { ...withoutMethod, signatures: [...withoutMethod.signatures, 'camera:move(dx)'] };
  assert.throws(() => checkLuaReference(entries, { [key]: wrongArity }, expected, methods),
    /Lua method arity or optionality differs/);
  const wrongAlias = { ...entry, signatures: ['camera_move(camera)', ...entry.signatures.slice(1)] };
  assert.throws(() => checkLuaReference(entries, { [key]: wrongAlias }, expected, methods),
    /Lua alias signatures differ/);
});

test('missing, unknown or non-Lua examples fail', async () => {
  assert.throws(() => checkLuaReference(entries, { [key]: { ...entry, exampleId: '' } }, expected, methods),
    /Incomplete Lua reference entry/);
  await assert.rejects(publishExample('missing-example-fixture', 'docs/reference'), /Unknown documentation example/);
  await assert.rejects(publishExample('catch-c', 'docs/reference'), /does not have a lua entry point/);
});
