---
name: Bug report
about: Something compiles wrong, crashes, or produces unexpected output
title: ''
labels: bug
assignees: ''
---

## What happened

<!-- One or two sentences. What did the compiler do that it shouldn't have? -->

## What you expected

<!-- What should have happened instead? -->

## Reproducer

<!--
The smallest possible input that triggers the issue. A CMM snippet
inline in the issue is ideal. If a file is needed, paste its content
in a code block rather than attaching it.
-->

```c
// minimal .cmm here
```

## Command and output

```
$ cmmcomp.exe my_program.cmm proc_name path/to/proc path/to/Macros path/to/tmp 0
<paste full compiler output here>
```

## Environment

- YANC version / tag: <!-- e.g. v2, or the commit SHA from `git rev-parse HEAD` -->
- OS: <!-- Windows 10/11, Linux distro, ... -->
- MSYS2 / MinGW / GCC version: <!-- output of `gcc --version` -->

## Anything else

<!-- Workarounds you tried, related issues, screenshots if it's a GTKWave thing, etc. -->
