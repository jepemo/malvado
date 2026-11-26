# Documentation templates

Copy the outline for the page you need. Replace the prompts with content.
Combine short sections when it makes the page easier to scan. See the
[writing guide](api-documentation.md) for example and verification rules.

## Concept chapter

```markdown
# [Concept the reader wants to understand]

[Open with a question or a result the reader can observe.]

## See it work

[Show or link to the smallest complete program. State what readers will see.]

## How it works

[Explain the mental model in the order events happen. Use a diagram if it
clarifies a relationship. Address a likely source of confusion.]

## Try a change

[Change one thing. Ask the reader to predict the result, then tell them
what to check.]

## Learn more

[Link to the exact API reference and a natural next lesson or example.]
```

## Tutorial

```markdown
# [What the reader will build]

[Show the finished result and say what the reader will learn.]

## Before you start

[Give the prerequisites, starting files, and exact setup commands.]

## [First action and visible result]

[Give one small step. Explain why it works. Provide the run command, the
expected result, and the complete runnable checkpoint.]

## [Next action and visible result]

[Repeat for each step. Make it possible to resume from its checkpoint.]

## Run the finished game

[Link the complete code and assets. Give the command, expected behavior,
controls, and how to stop. Link to relevant concepts and references.]
```

## Pattern or recipe

```markdown
# [Recurring problem the reader wants to solve]

[Describe a concrete game situation and when this solution fits.]

## Use the pattern

[Show the smallest complete runnable example. Provide setup, run and exit
instructions, and the expected result.]

## Why it works

[Explain the Malvado behavior the solution relies on. Describe the
trade-offs, common failure points, and when a simpler approach is enough.]

## Learn more

[Link to the supporting concepts, API entries, and runnable examples.]
```

## Gallery entry

```markdown
# [Example name]

[State the topic, learning goal, and observable result. Add a preview
when it helps readers choose.]

## Run the example

[Link to the complete source and required files. Give requirements,
working directory, commands, expected behavior, controls, and how to
stop. Credit and link to asset licenses where applicable.]

## Explore the code

[Point out the part that teaches the goal. Give entry points and
instructions for each verified language variant you present. Link to the
concept and API entries.]
```

## Function reference

```markdown
# [Public function name]

[Summarize observable behavior in one sentence. Give a game use case
when it helps the reader decide when to call the function.]

## Signatures

[Show exact module, imported global, namespace, and method forms where
supported. Show each supported overload and any options table.]

## Parameters

[For each parameter and options field, give the name, type, whether it
is required, default if optional, and applicable units, constants,
accepted values, and bounds.]

## Returns

[Give each result's type and meaning in tuple order. Describe when the
function returns nil, false, a handle, or no values, and how failure is
reported. Include rejected inputs and raised errors where applicable.]

## Run the example

[Provide or directly link to a complete runnable program. Include setup,
required files, entry point, working directory, command, expected result,
and how the program or main loop exits.]

## Things to know

[Cover applicable side effects, scope, timing, coordinate space,
ownership, lifetime, invalid handles, and common mistakes. Use a diagram
only when it clarifies behavior.]

## Related

[Link to nearby operations, the concept guide, or another useful example.]
```

Keep simple entries short. Omit sections that have no relevant contract.
Use a sentence for a simple return value and its error behavior.
See [`rand`](https://malvado.dev/docs/reference/lua/platform/rand/) for a
short function entry.
