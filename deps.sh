# git@branch or git@commit
DEPS=(
https://github.com/facebook/folly@v2018.08.06.00
https://github.com/Yeolar/wangle@v2018.08.06.00.fix:"cmake ../wangle"
)

mkdir -p _deps && cd _deps
if [ ! -f dep-builder.py ]; then
    curl -L https://github.com/Yeolar/dep-builder/tarball/master | tar xz --strip 2 -C .
fi

python dep-builder.py - "${DEPS[@]}"
