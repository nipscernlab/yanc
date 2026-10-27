<!--
Thanks for the PR! A few quick reminders:

- Read CONTRIBUTING.md if you haven't yet, especially the bilingual
  MSG_* convention.
- CI builds the tools, runs the pipeline (cmmcomp -> asmcomp) on every
  C± test, the README quick start and the manual. It does not compare
  goldens: run Scripts/regress.sh before requesting review.
- Source comments in English. User-facing strings stay bilingual
  through the M("pt", "en") macro.
-->

## Summary

<!-- What does this PR do, in 1–3 sentences? -->

## Why

<!--
What problem does this solve? If it's a bug fix, link the issue. If
it's a refactor or feature, explain what motivated it.
-->

## How it was tested

<!--
- [ ] make CFLAGS="-O2 -Wall -Werror" builds
- [ ] Scripts/regress.sh passes (every golden, every simulation)
- [ ] CI (`.github/workflows/ci.yml`) passes
- [ ] Manually exercised on examples: <which ones>
- [ ] (For language/codegen changes) inspected the generated .asm / .v
-->

## Anything reviewers should look at carefully

<!--
Tricky bits, intentional trade-offs, follow-ups that should land in a
separate PR, etc.
-->
