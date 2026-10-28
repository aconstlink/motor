# msl mode for Emacs

A small major mode for motor shading language, derived from Emacs' built-in
`c-mode`. No additional packages or engine build are required.

## Setup

Add the following to your Emacs initialization file, adjusting the path:

```elisp
(add-to-list 'load-path "E:/repos_aconstlink/motor_demos/motor/tools/emacs")
(require 'msl-mode)
```

For Emacs inside WSL, use the corresponding `/mnt/e/.../tools/emacs` path.
Files ending in `.msl`, including `.lib.msl`, then select the mode automatically.
In an already open buffer, run `M-x msl-mode`.

## Highlighting

- All public type names and built-in functions registered in `msl/enums.hpp`.
- Binding names and aliases after `:`, such as `projection` and `light_color`.
- Shader stages, configuration and library names, and msl structural keywords.
- C control flow, comments and strings through `c-mode`.
- Common native shader functions such as `normalize`, `sin` and `reflect`.

The native function list is separate from the registered msl built-ins;
highlighting does not guarantee backend support or validate function arguments.
Internal sentinels and the parser-only `__make_array__` are intentionally omitted.
Standard Emacs faces follow your current theme.

Indentation uses C rules with a four-space offset. Shader-specific blocks may
need manual adjustment; this mode is not an msl parser or language server.
For example, to additionally disable indentation tabs:

```elisp
(add-hook 'msl-mode-hook
          (lambda () (setq-local indent-tabs-mode nil)))
```

## Maintenance and Tests

When changing the language, update `msl-types`, `msl-builtins` and `msl-bindings`
from `msl/enums.hpp`. Use the public string names, not the internal enum names
(for example, `smoothpulse`, not `spulse`).

From this directory, run:

```sh
emacs -Q --batch -L . -l msl-mode-tests.el -f ert-run-tests-batch-and-exit
```

The tests compare the complete public vocabulary against the source tables and
check highlighting, comments, strings and automatic mode selection. They require
the repository layout, but the mode itself works as a standalone `.el` file.
