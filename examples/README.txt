- Building on Linux with CMake

  cmake .
  make

  export KIARA_MODULE_PATH=$PWD/../lib
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/../lib
  export PATH=$PATH:$PWD/../bin

- Building on Linux with SCons

  scons

  export KIARA_MODULE_PATH=$PWD/../lib
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/../lib
  export PATH=$PATH:$PWD/../bin

- Running examples on Linux

  Run:

  > ./structtest_server &

  Wait until message " Starting server..." appears then run:

  ./structtest

  Following text should be output:

  StructTest.pack: result = { ival : 21, sval : "test21" }
  StructTest.getInteger: result = 25
  StructTest.getString: result = test72
  Server exception raised: 1234 error

  Stop server:

  > fg
  Ctrl-C

- Building on Windows with CMake

  Depending on the version of SDK (32-bit vs 64-bit) run
  respective Visual Studio command prompt. Also depending on Visual Studio version
  use appropriate generator-name option for CMake:

  "Visual Studio 9 2008"
  "Visual Studio 9 2008 Win64"
  "Visual Studio 10"
  "Visual Studio 10 Win64"

  Run following commands:

  cd c:\Path\To\kiara_sdk
  cd examples
  "c:\Path\To\CMake\bin\cmake.exe" -G generator-name .
  start KIARA_SDK_Example.sln

  Visual Studio should start and load SDK example solution.
  Select build mode used in SDK (Debug or Release) and build solution.

  Built executables should be in Release or Debug directory depending
  on build mode.

  PATH=%PATH%;%CD%\..\bin;%CD%\..\lib;
  set KIARA_MODULE_PATH=%CD%\..\lib
  cd Release

- Running examples on Windows

  start structtest_server

  New terminal window will be opened. Wait until message " Starting server..." appears then run in
  the initial terminal window:

  structtest

  Following text should be output:

  StructTest.pack: result = { ival : 21, sval : "test21" }
  StructTest.getInteger: result = 25
  StructTest.getString: result = test72
  Server exception raised: 1234 error

  Stop server by switching to the terminal window with running server and pressing Ctrl-C.
