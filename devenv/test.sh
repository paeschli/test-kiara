# Source this file

case $SYS_NAME in
    windows)
            getDllName() {
                echo "${1}.dll"
            }
            ;;
    *)
            getDllName() {
                echo "lib${1}.so"
            }
            ;;
esac

# Value Test

kiara_valuetest || return 1

echo "VALUE TEST: OK"

kiara-version || return 1

# JSON Test

cat > tmp.json <<EOF
{
  "kiara" : {
      "description" : "KIARA Library",
      "default" : false,
      "type" : "library",
      "dependsOn" : ["contrib", "boost", "dfc"],
      "optionallyDependsOn" : ["llvm3", "anydsl"],
      "repository" : {
          "type" : "git",
          "url" : "ssh://contact.cg.uni-saarland.de/projects/private/rubinste/repos/kiara.git"
      },
      "builder" : "SCons",
      "variables@activate" : {
          "boost.build_libraries+unique" : [
              "--with-thread", "--with-regex", "--with-test"
          ]
      },
      "shell@setup_own_env" : {
      }
  }
}
EOF

kiara-lli $(devenvGetModuleBuildDir kiara)/lib/json_process_tool.bc --env < tmp.json > result1.txt || return 1
json_process --env < tmp.json > result2.txt || return 1
diff result1.txt result2.txt || { echo "Test failed"; return 1; }

rm -f tmp.json result1.txt result2.txt

echo "JSON: OK"
echo

kiara_buffertest || return 1

echo "Buffer Test: OK"
echo

kiara_apitest_c || return 1

echo "API Test C: OK"
echo

kiara_encrypttest || return 1

echo "Encryption/Decryption: OK"
echo

# Curl Test

#kiara-lli -load=$(devenvGetModuleBuildDir kiara)/lib/$(getDllName warnless) $(devenvGetModuleBuildDir kiara)/lib/curl_tool.bc http://graphics.cs.uni-sb.de -o result1.txt || return 1
#curl http://graphics.cs.uni-sb.de -o result2.txt || return 1
#diff result1.txt result2.txt || { echo "Test failed"; return 1; }

#rm -f tmp.json result1.txt result2.txt

#echo "CURL: OK"

# Compiler Test

KIARA_DIR=$(de get_var kiara.root_dir)

for engine in JIT MCJIT; do

    export KIARA_JIT_ENGINE=$engine

    echo "TESTING COMPILER WITH $engine ENGINE..."

    "$PYTHON" "$KIARA_DIR/test/tools/runtests.py" "$KIARA_DIR/test/simple" \
        || { echo "Test failed"; return 1; }

    echo "COMPILER WITH $engine ENGINE: OK"
    echo

    # Compiler with Valgrind Test

    if true; then # [ "$engine" != "MCJIT" ]; then # Currently MCJIT tests are failing with valgrind

    if [ "$1" = "--no-valgrind" ]; then
        echo "Skipping compiler test for $engine engine with valgrind"
    else
        if [ -n "$(type -t valgrind)" ]; then
            echo "TESTING COMPILER WITH VALGRIND WITH $engine ENGINE..."

            "$KIARA_DIR/test/tools/runtests_with_valgrind.sh" "$KIARA_DIR/test/simple" \
                || { echo "Test failed"; return 1; }

            echo "COMPILER WITH VALGRIND WITH $engine ENGINE: OK"
            echo
        fi
    fi

    fi

done

# Test examples

if "$PYTHON" -c "
import sys, os
if sys.platform == 'win32':
    dirname = os.path.join(os.path.abspath(sys.argv[1]), 'winpexpect')
    sys.path = [dirname] + sys.path
    try:
      import winpexpect
    except ImportError:
      sys.exit(1)
else:
  try:
    import pexpect
  except ImportError:
    sys.exit(1)
sys.exit(0)
" "$KIARA_DIR/devenv"; then

    for engine in JIT MCJIT; do

        export KIARA_JIT_ENGINE=$engine

        echo "TESTING EXAMPLES WITH $engine ENGINE..."

        "$PYTHON" "$KIARA_DIR/devenv/test_examples.py" \
            || { echo "Test failed"; return 1; }

        echo "EXAMPLES WITH $engine ENGINE: OK"
        echo
    done

else
    echo "Could not find pexpect python module, examples cannot be tested !!!"
fi
unset KIARA_JIT_ENGINE
unset engine
