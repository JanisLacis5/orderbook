PROJECT_ROOT="$HOME/dev/orderbook/"
buildDir="build/release build/debug"

for dir in $buildDir; do
  echo "cmake --build $PROJECT_ROOT$dir"
  cmake --build "$PROJECT_ROOT$dir"
done