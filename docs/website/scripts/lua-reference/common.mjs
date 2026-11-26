// Shared example setup and parameter descriptions for Lua reference families.
export const referenceExample = (name, result, graphical = false) => ({
  exampleId: `reference-${name}`,
  exampleSetup: graphical
    ? `With the Malvado SDK installed and a desktop display, save \`${name}.lua\` in an empty directory. Open a terminal there and run \`malvado run ${name}.lua\`.`
    : `With the Malvado SDK installed, save \`${name}.lua\` in an empty directory. Open a terminal there and run \`malvado run --headless ${name}.lua\`.`,
  exampleResult: result,
});
export const luaColor = '[Lua color](../../../../concepts/drawing/#lua-colors)';
export const colorParameter = `Required ${luaColor}. Use a palette constant, \`{r, g, b, a?}\` or \`{r = r, g = g, b = b, a = a}\`. The RGB channels are required. Lua integer channels range from 0 to 255 and alpha defaults to 255.`;
export const collisionExample = referenceExample('collision',
  'The program checks rectangle overlap, point containment, shape overlap and a process collision. It reports each successful check, then exits.');
