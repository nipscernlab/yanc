<!--
Thanks for the PR! A few quick reminders:

- Read CONTRIBUTING.md if you haven't yet, especially the bilingual
  MSG_* convention and the m4 apostrophe trap in .l/.y files.
- CI runs build + full-pipeline smoke (cmmcomp -> appcomp -> asmcomp)
  on every push. Make sure it's green before requesting review.
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
- [ ] Scripts/aurora.bat succeeds locally
- [ ] CI smoke (`.github/workflows/ci.yml`) passes
- [ ] Manually exercised on examples: <which ones>
- [ ] (For language/codegen changes) inspected the generated .asm / .v
-->

## Anything reviewers should look at carefully

<!--
Tricky bits, intentional trade-offs, follow-ups that should land in a
separate PR, etc.
-->
