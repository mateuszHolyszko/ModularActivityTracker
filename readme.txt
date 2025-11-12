cd /c/dev/ModularActivityTracker

git fetch origin
git reset --hard origin/$(git rev-parse --abbrev-ref HEAD)