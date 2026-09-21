#!/usr/bin/env bash
# One-time repository setup (run after the first push to main):
#   .github/setup-repo.sh
#
# Creates: labels, the Port Roadmap project, branch protection with the
# required checks and the repository metadata. Idempotent.
set -euo pipefail

OWNER="${GITHUB_REPOSITORY%%/*}"
REPO="${GITHUB_REPOSITORY##*/}"
: "${OWNER:=mattiasploesch-cpu}"
: "${REPO:=LibMatti}"

echo "== Labels =="
gh label create new_mod --color 62d50f --description "A PR importing a new mod package (.matti)" 2>/dev/null || true
# All labels from .github/labels.yml (name/color/description columns).
python3 - <<'EOF' | while IFS='|' read -r name color desc; do
  gh label create "$name" --color "$color" --description "$desc" 2>/dev/null || \
    gh label edit "$name" --color "$color" --description "$desc" || true
done
import yaml, sys
for lbl in yaml.safe_load(open('.github/labels.yml')):
    print(f"{lbl['name']}|{lbl['color']}|{lbl.get('description','')}")
EOF

echo "== Port Roadmap project =="
PROJECT_ID="$(gh project create --owner "$OWNER" --title "Port Roadmap" --format json | python3 -c 'import json,sys;print(json.load(sys.stdin)["id"])' 2>/dev/null || true)"
if [ -z "$PROJECT_ID" ]; then
  echo "Project exists already (or could not be created) - skipping"
else
  gh project field-create "$PROJECT_ID" --owner "$OWNER" --name "Status" \
    --single_select_option --options "Backlog,In Progress,In Review,Done" || true
  gh project field-create "$PROJECT_ID" --owner "$OWNER" --name "Phase" \
    --single_select_option --options "P3 Render,P4 World,Mod Pipeline,Infra" || true
  echo "Project created (id: $PROJECT_ID) - add it to .github/workflows/projects.yml"
fi

echo "== Branch protection (main) =="
gh api -X PUT "repos/$OWNER/$REPO/branches/main/protection" --input - <<'EOF'
{
  "required_status_checks": {
    "strict": true,
    "contexts": ["Build (ubuntu-latest)", "Client smoke test", "Lint (clang-format)"]
  },
  "enforce_admins": false,
  "required_pull_request_reviews": {
    "required_approving_review_count": 1,
    "dismiss_stale_reviews": true,
    "require_code_owner_reviews": false
  },
  "restrictions": null,
  "allow_force_pushes": false,
  "allow_deletions": false,
  "required_linear_history": true
}
EOF
echo "Branch protection set."

echo "== Repo metadata =="
gh repo edit "$OWNER/$REPO" \
  --description "A from-scratch C port of the Minecraft 1.21.11 + NeoForge toolchain: bootstrap, FML, ModLauncher, mixins, registries, render - with native .so mods." \
  --homepage "" \
  --add-topic c --add-topic minecraft --add-topic neoforge --add-topic modding \
  --add-topic reverse-engineering --add-topic cmake --add-topic gamedev \
  2>/dev/null || true
gh repo edit "$OWNER/$REPO" --enable-issues --enable-wiki=false --enable-projects 2>/dev/null || true
gh repo edit "$OWNER/$REPO" --delete-branch-on-merge 2>/dev/null || true
gh repo edit "$OWNER/$REPO" --enable-automated-security-fixes 2>/dev/null || true
gh repo edit "$OWNER/$REPO" --enable-vulnerability-alerts 2>/dev/null || true

echo "Setup complete."
