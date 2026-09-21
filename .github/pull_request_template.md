<!-- A reviewer bot reviews every PR automatically (@coderabbitai).
     A full build + ctest run is attached to the PR as required checks. -->

## What does this PR change?

<!-- One or two sentences: what and why. Reference issues with #nr. -->

## Checklist

- [ ] The build is green locally (`cmake --build cmake-build-debug`)
- [ ] `ctest --test-dir cmake-build-debug --output-on-failure` passes
- [ ] New sources follow the 1:1 port convention (see README, vendor reference)
- [ ] No files outside the repository were produced by the change
- [ ] Every TODO left in the code is intentional and marked `// TODO:`

## Verification

<!-- How was this verified? paste the relevant log lines or test names -->

## Notes for the reviewer

<!-- Anything worth pointing out: risky paths, follow-ups, decisions. -->
