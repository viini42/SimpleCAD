## Naming conventions

- Filenames: snake_case
- Classes and structs: PascalCase
- Enums: PascalCase
- Parameters: camelCase
- Local variables: snake_case
- Global variables: g_snake_case
- Lambdas: snake_case
- Global functions: PascalCase
- Class and struct methods: PascalCase
- Class fields: m_snake_case
- Struct fields: snake_case
- Enumerators: SCREAMING_CASE
- Other constants: SCREAMING_CASE
- Global constants: SCREAMING_CASE
- Namespaces: snake_case
- Macros: SCREAMING_CASE
- Typedefs: PascalCase

## Format style

From .clang-format file in root project

Always run clang-format on changed files before committing.

## Versioning rules

### Commit messages

- fix: for bug fixes, regressions, leaks, warnings and behavior corrections
- dev: for feature work, behavior changes and new capabilities
- infra: for build system, dependency, tooling and project structure changes
- test: for test-only changes
- ref: for refactors that primarily reorganize code without changing the intended behavior
- ui: for user interface-focused changes
- doc: for documentation updates
- wip: only for temporary local progress, not for stable history on shared branches

### Recommended format:

<prefix>: short imperative summary

Any additional note should be written here
In multiline style
Can use markdown syntax

### Guidelines:

    Keep the summary short and specific. Prefer the changed behavior over generic wording.
    Use one commit for one concern. Do not mix infra: and fix: changes in the same commit unless one is required for the other.
    Prefer lowercase summaries, matching the existing project history.
    Write subjects as statements of the change, not as ticket notes or long explanations.
    If a change fixes a bug caused by an earlier refactor or feature, use fix: instead of preserving the older prefix.

Practical rule of thumb:

    choose fix: when correcting behavior
    choose dev: when adding behavior
    choose infra: when changing how the project is built, organized or tooled
    choose ref: when reshaping code without intending a behavior change

### Commit level

Always atomical commits that can be build when needed to checkout.
Each commit does one small things.

For big changes, create branches and commit them gradually.

## Code styling

- Name header files with hpp extension.
- Use anonymous namespaces in cpp files for utility/auxiliary functions.
- In cpp files, do not create [named] namespaces blocks. Prefer the syntax `my_namespace::MyClass::MyFunction`.
- Four maximum levels of indentation. For example, 1st level: function body, 2nd level: if body, 3rd level: for body,
  4th level: statements. If a deeper level is needed, refactor to another function.
- Maximum of 4 function parameters, more than that should use structs.
- No `new` or `delete` call (neither `malloc` or `free`), use smart pointers for dynamic memory allocation.
- Call object constructor with braces {}