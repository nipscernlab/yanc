#!/usr/bin/env python3
"""Hold the link list at the foot of CHANGELOG.md to its version headings.

    python3 Scripts/check_changelog.py [repo root]

Each heading `## [vX] - date` is a link that resolves only when the foot of
the file has a line `[vX]: .../releases/tag/vX`, and `[Unreleased]` links to
the comparison from the newest version to HEAD. Nothing in a release adds
those lines, and three releases went without them (GitHub issue #51). This
checks, from the file alone, so that a checkout without tags checks too:

  1. every heading has a link line and every link line a heading,
  2. each version links to its own release tag,
  3. [Unreleased] compares the newest version heading with HEAD,
  4. the link lines are in the order of the headings.
"""
import os
import re
import sys

REPO = 'https://github.com/nipscernlab/yanc'


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else \
        os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    text = open(os.path.join(root, 'CHANGELOG.md'), encoding='utf-8').read()
    heads = re.findall(r'^## \[([^\]]+)\]', text, re.M)
    links = re.findall(r'^\[([^\]]+)\]: *(\S+)', text, re.M)
    urls = dict(links)
    bad = []

    # 1
    for h in heads:
        if h not in urls:
            bad.append(f'## [{h}] has no link line at the foot')
    for name, _ in links:
        if name not in heads:
            bad.append(f'[{name}] links a version with no heading')

    # 2 and 3
    versions = [h for h in heads if h != 'Unreleased']
    for v in versions:
        want = f'{REPO}/releases/tag/{v}'
        if v in urls and urls[v] != want:
            bad.append(f'[{v}] links {urls[v]}, not {want}')
    if 'Unreleased' in urls and versions:
        want = f'{REPO}/compare/{versions[0]}...HEAD'
        if urls['Unreleased'] != want:
            bad.append(f'[Unreleased] links {urls["Unreleased"]}, not {want} '
                       f'({versions[0]} is the newest heading)')

    # 4
    order = [name for name, _ in links if name in heads]
    if order != [h for h in heads if h in urls]:
        bad.append('the link lines are not in the order of the headings')

    if bad:
        print('check_changelog: the links at the foot of CHANGELOG.md do not '
              'match its headings', file=sys.stderr)
        for b in bad:
            print(f'  {b}', file=sys.stderr)
        return 1
    print(f'check_changelog: {len(heads)} headings, each with its link; '
          f'[Unreleased] compares from {versions[0] if versions else "nothing"}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
