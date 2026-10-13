# Sphinx configuration of the YANC manual (docs/). The pages are Markdown
# (MyST), readable as they are on GitHub; CI builds them into the HTML
# published at nipscern.com/library/yanc (.github/workflows/docs.yml).
#   python -m pip install -r docs/requirements.txt
#   python -m sphinx -W -b html docs docs/_build/html

import os, re

here = os.path.dirname(os.path.abspath(__file__))
with open(os.path.join(here, '..', 'Compilers', 'yanc_version.h'), encoding='utf-8') as f:
    m = re.search(r'#define\s+YANC_VERSION\s+"([^"]+)"', f.read())
release = m.group(1) if m else 'dev'
version = release

project = 'YANC'
author = 'NIPS-CERN'
copyright = 'NIPS-CERN, Universidade Federal de Juiz de Fora'

extensions = ['myst_parser']
source_suffix = {'.md': 'markdown'}
root_doc = 'index'
exclude_patterns = ['_build']

myst_heading_anchors = 3          # [text](page.md#a-heading) links work as on GitHub

html_theme = 'furo'
html_title = f'YANC {release}'
html_static_path = []
