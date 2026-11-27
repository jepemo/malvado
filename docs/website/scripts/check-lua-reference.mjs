// Validate the authored reference against the existing public inventories.
const word = '[a-z_][a-z_0-9]*';
const callPattern = new RegExp(`^(${word}(?:[.:]${word})*)\\(([^()]*)\\)$`);
const nonempty = (value) => typeof value === 'string' && value.trim().length > 0;
const sortedSignatures = (variants) => variants.map((args) => args.join(',')).sort().join('|');
const optionality = (args) => args.map((arg) => arg.endsWith('?')).join(',');

function signature(line, key) {
  const call = typeof line === 'string' && callPattern.exec(line);
  if (!call) throw new Error(`Invalid Lua signature for ${key}: ${line}`);
  return [call[1], call[2] ? call[2].split(',').map((part) => part.trim()) : []];
}

export function checkLuaReference(entries, content, publicForms, methods) {
  const pages = new Map();
  const documentedMethods = new Map();
  const keys = new Set();
  for (const { key, forms } of entries) {
    if (!forms.length || keys.has(key)) throw new Error(`Invalid Lua reference page: ${key}`);
    keys.add(key);
    const detail = content[key];
    if (!detail) throw new Error(`Missing Lua reference entry: ${key}`);
    if (!['description', 'returns', 'exampleId', 'exampleSetup', 'exampleResult'].every((field) =>
      nonempty(detail[field])) || !Array.isArray(detail.signatures) || !detail.signatures.length ||
      !Array.isArray(detail.params) || detail.params.some((row) =>
        !Array.isArray(row) || row.length !== 2 || !row.every(nonempty))) {
      throw new Error(`Incomplete Lua reference entry: ${key}`);
    }
    const signatures = new Map();
    const parameters = new Set(detail.params.map(([parameter]) => parameter.split('.')[0]));
    const returnsConstructor = forms.includes('mv.process') || forms.includes('mv.process.define');
    for (const line of detail.signatures) {
      const [call, args] = signature(line, key);
      for (const arg of args) {
        if (call.includes(':') || /^"[^"]+"$/.test(arg) || parameters.has(arg.replace(/\?$/, '')))
          continue;
        throw new Error(`Undocumented Lua parameter in ${key}: ${arg}`);
      }
      if (call.includes(':')) {
        if (documentedMethods.has(call)) throw new Error(`Duplicate Lua method signature: ${call}`);
        documentedMethods.set(call, { key, args });
      } else if (!forms.includes(call) && !(returnsConstructor && call === 'constructor')) {
        throw new Error(`Unexpected Lua call in ${key}: ${call}`);
      }
      const overloads = signatures.get(call) ?? [];
      overloads.push(args);
      signatures.set(call, overloads);
    }
    for (const call of forms) {
      if (!publicForms.has(call)) throw new Error(`Stale Lua reference call: ${call}`);
      if (pages.has(call)) throw new Error(`Duplicate Lua reference call: ${call}`);
      if (!signatures.has(call)) throw new Error(`Missing Lua signature in ${key}: ${call}`);
      const overloads = signatures.get(call);
      if (sortedSignatures(signatures.get(forms[0])) !== sortedSignatures(overloads))
        throw new Error(`Lua alias signatures differ in ${key}: ${call}`);
      pages.set(call, { key, overloads });
    }
  }
  for (const key of Object.keys(content)) {
    if (!keys.has(key)) throw new Error(`Stale Lua reference entry: ${key}`);
  }
  for (const call of publicForms) {
    if (!pages.has(call)) throw new Error(`Public Lua call has no reference page: ${call}`);
  }
  for (const [methodLine, procedureLine] of methods) {
    const [method, args] = signature(methodLine, 'instance methods');
    const [procedure] = signature(procedureLine, 'instance methods');
    const actual = documentedMethods.get(method);
    const page = pages.get(procedure);
    if (!actual || actual.key !== page?.key) throw new Error(`Missing Lua method reference: ${method}`);
    // Match the explicit procedural overload after removing its receiver.
    const shape = optionality(actual.args);
    if (optionality(args) !== shape ||
        !page.overloads.some((variant) => variant.length && optionality(variant.slice(1)) === shape))
      throw new Error(`Lua method arity or optionality differs: ${method}`);
    documentedMethods.delete(method);
  }
  if (documentedMethods.size)
    throw new Error(`Stale Lua method reference: ${[...documentedMethods.keys()].join(', ')}`);
}
