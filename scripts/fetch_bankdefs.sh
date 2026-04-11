#!/usr/bin/env bash
# Refresh data/bankdefs/hipo4/*.json from coatjava's development branch.
#
# Run this when the upstream bank definitions change, then re-run
#     python3 scripts/gen_bankdefs.py
# to regenerate the C++ wrappers under clas12/include/clas12/bankdefs/.

set -euo pipefail

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
DEST="$REPO_ROOT/data/bankdefs/hipo4"
BRANCH=${1:-development}
API="https://api.github.com/repos/JeffersonLab/coatjava/contents/etc/bankdefs/hipo4?ref=$BRANCH"

mkdir -p "$DEST"
cd "$DEST"

echo "Fetching bank defs from coatjava@$BRANCH ..."
curl -sL "$API" | python3 -c "
import json, sys, urllib.request
for f in json.load(sys.stdin):
    url = f['download_url']
    name = f['name']
    print(f'  {name}', file=sys.stderr)
    urllib.request.urlretrieve(url, name)
"
echo "Wrote $(ls "$DEST" | wc -l | tr -d ' ') files to $DEST"
echo
echo "Now run: python3 scripts/gen_bankdefs.py"
