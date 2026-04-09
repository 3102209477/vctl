import re
from pathlib import Path
root = Path(r'd:\Desktop\test\vctl')
for path in sorted(root.rglob('*.md')):
    print('FILE', path.relative_to(root))
    lines = path.read_text(encoding='utf-8').splitlines()
    for i, line in enumerate(lines):
        if line.rstrip() != line and line.rstrip() != '':
            print('  TRAIL', i+1, repr(line))
        if re.match(r'^(#+)\s+.*', line):
            if i > 0 and lines[i-1].strip() != '':
                print('  BADHEAD', i+1, line)
            if i+1 < len(lines) and lines[i+1].strip() != '':
                print('  BADHEADAF', i+1, line)
        if re.match(r'^\s*[-*+]\s+\S', line):
            if i > 0 and lines[i-1].strip() != '':
                print('  BADLIST', i+1, line)
        if line.strip() == '```':
            print('  FENCE_NO_LANG', i+1, line)
        if re.match(r'^\|[-:]+\|[-:]+\|?\s*$', line.strip()):
            print('  TABLESTYLE', i+1, line)
    print()
