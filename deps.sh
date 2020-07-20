# git@branch or git@commit or tarball@root
DEPS=(
https://github.com/yeolar/folly@v2018.08.06.00.fix
https://github.com/Yeolar/wangle@v2018.08.06.00.fix:"cmake ../wangle"
https://github.com/Yeolar/proxygen@v2018.08.06.00.fix:"cd proxygen && ./build.sh"
https://github.com/Yeolar/accelerator@v2
https://github.com/Yeolar/crystal@master
)

mkdir -p _deps && cd _deps
if [ ! -f dep-builder.py ]; then
    curl -L https://github.com/Yeolar/dep-builder/tarball/master | tar xz --strip 2 -C .
fi

python dep-builder.py - "${DEPS[@]}"
