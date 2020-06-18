# git@branch or git@commit
DEPS=(
https://github.com/facebook/folly@v2018.08.06.00
https://github.com/Yeolar/wangle@v2018.08.06.00.fix:"cmake ../wangle"
https://github.com/facebookresearch/faiss@v1.6.3:"./configure --without-cuda"
https://github.com/Yeolar/accelerator@v2
https://github.com/Yeolar/crystal
)

mkdir -p _deps && cd _deps
if [ ! -f dep-builder.py ]; then
    curl -L https://github.com/Yeolar/dep-builder/tarball/master | tar xz --strip 2 -C .
fi

python dep-builder.py - "${DEPS[@]}"
