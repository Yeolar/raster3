# git@branch or git@commit or tarball@root
DEPS=(
https://github.com/yeolar/folly@v2018.08.06.00.fix
https://github.com/Yeolar/wangle@v2018.08.06.00.fix:"cmake ../wangle"
https://github.com/Yeolar/proxygen@v2018.08.06.00.fix:"cd proxygen && ./build.sh"
https://github.com/google/s2geometry@master:"cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_EXAMPLES=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON .."
https://github.com/facebookresearch/faiss/archive/v1.6.3.zip@faiss-1.6.3:"./configure --without-cuda"
https://github.com/Yeolar/accelerator@v2
https://github.com/Yeolar/crystal@master
https://github.com/Yeolar/inja@master
)

mkdir -p _deps && cd _deps
if [ ! -f dep-builder.py ]; then
    curl -L https://github.com/Yeolar/dep-builder/tarball/master | tar xz --strip 2 -C .
fi

python dep-builder.py - "${DEPS[@]}"
