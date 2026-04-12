PROJECT_ROOT="$HOME/dev/orderbook/"

cmake -S "$PROJECT_ROOT" -B "$PROJECT_ROOT/build/release" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++

cmake -S "$PROJECT_ROOT" -B "$PROJECT_ROOT/build/debug" \
    -DENABLE_TSAN=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++
