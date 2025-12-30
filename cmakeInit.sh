PROJECT_ROOT="$HOME/dev/orderbook/"

cmake -B $PROJECT_ROOT/build/release -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=Release
cmake -B $PROJECT_ROOT/build/debug -DENABLE_TSAN=ON -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=Debug
