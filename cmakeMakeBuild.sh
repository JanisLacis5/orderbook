PROJECT_ROOT="~/dev/orderbook/"

cmake -B $PROJECT_ROOT/build/release -DCMAKE_CXX_COMPILER=/usr/bin/clang++
cmake -B $PROJECT_ROOT/build/debug -DENABLE_TSAN=ON -DCMAKE_CXX_COMPILER=/usr/bin/clang++

cmake --build $PROJECT_ROOT/build/release
cmake --build $PROJECT_ROOT/build/debug
