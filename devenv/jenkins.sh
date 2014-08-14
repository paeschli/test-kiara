#!/bin/bash

# Jenkins build script
#
# This script is designed to be used with Jenkins CI server
# (http://jenkins-ci.org/).
#
# Setup slaves to have git in PATH and use bash for 'Execute shell' step.
# Setup job to use 'Execute shell' step with the following contents:
#
#  [ -z "$KIARA_BRANCH" ] && KIARA_BRANCH=master
#  set -e
#  KIARA_URL=https://git.cg.uni-saarland.de/projects/kiara.git
#  rm -rf tmp_kiara
#  git clone --no-checkout --depth 1 -b "$KIARA_BRANCH" "$KIARA_URL" tmp_kiara
#  rm -f jenkins.sh
#  (cd tmp_kiara; git show HEAD:devenv/jenkins.sh) > jenkins.sh
#  chmod +x jenkins.sh
#  rm -rf tmp_kiara
#
#  ./jenkins.sh



chmod -w "$0"

# Fail if any command fails
set -e

# Setup defaults
[ -z "$OUTPUT_VERBOSITY" ] && OUTPUT_VERBOSITY=none
[ -z "$BOOST_VERSION" ] && BOOST_VERSION=1.52.0
[ -z "$KIARA_BRANCH" ] && KIARA_BRANCH=master
[ -z "$DEVENV_URL" ] && DEVENV_URL=https://git.cg.uni-saarland.de/projects/devenv.git
[ -z "$DEVENV_BRANCH" ] && DEVENV_BRANCH=master
[ -z "$DEVENV_MODULE_PACKAGE" ] && DEVENV_MODULE_PACKAGE=https://git.cg.uni-saarland.de/projects/kiara-devenv-modules.git
[ -z "$DEVENV_MODULE_PACKAGE_BRANCH" ] && DEVENV_MODULE_PACKAGE_BRANCH=master
[ -z "$LLVM_BRANCH" ] && LLVM_BRANCH=release_33

[ -d devenv ] && \
    echo "* Devenv directory exists" || \
    echo "* Devenv directory does not exist"

[ -e ./run_devenv ] && \
    echo "* run_devenv script exists" || \
    echo "* run_devenv script does not exist"

[ "$CLEAN_WORKSPACE" = "true" ] && \
    echo "* CLEAN_WORKSPACE is true" || \
    echo "* CLEAN_WORKSPACE is false"

# Set up or update devenv
if [ ! -d devenv -o ! -e ./run_devenv -o "$CLEAN_WORKSPACE" = "true" ]; then
  echo "* Cleanup Workspace ..."

  for f in $(find . -maxdepth 1 \( ! -name "." \)); do
      if [ ! "$f" -ef "$0" ]; then
          echo "* Delete $f"
          rm -rf "$f"
      fi
  done

  #rm * -rf && rm .??* -rf

  #find . -maxdepth 1 -type f | xargs rm -f
  #rm -rf resources devenv dfc

  echo "* Clone devenv from $DEVENV_URL with branch $DEVENV_BRANCH ..."

  git clone -b "$DEVENV_BRANCH" "$DEVENV_URL"

  echo "* Setup devenv"
  echo "* Devenv module package: $DEVENV_MODULE_PACKAGE"
  echo "* Devenv module package branch: $DEVENV_MODULE_PACKAGE_BRANCH"

  devenv/setup_devenv -a \
      --x-module-package="$DEVENV_MODULE_PACKAGE" \
      --x-module-package-branch="$DEVENV_MODULE_PACKAGE_BRANCH"
else

  #rm -rf devenv
  #git clone https://git.cg.uni-saarland.de/projects/devenv.git
  #cd devenv && git checkout $DEVENV_BRANCH && cd ..

  #devenv/setup_devenv -a \
  #  --x-module-package="https://git.cg.uni-saarland.de/projects/kiara-devenv-modules.git" \
  #  --x-module-package-branch="master"

  echo "* Checkout devenv branch $DEVENV_BRANCH"

  cd devenv
  git fetch --all
  git checkout "$DEVENV_BRANCH"
  git pull --force || exit 1
  cd ..
fi

echo "* Devenv Revision $(cd devenv; git rev-parse --short=10 HEAD)"
echo "* LLVM Branch: $LLVM_BRANCH"

export LLVM_BRANCH
export MSVC_VERSIONS
export TARGET_ARCHITECTURE
export OPTIMIZATION

# Create build script
cat > build.sh <<EOF
  if [ "$PRINT_COMMANDS" = "true" ]; then
    set -x
  fi

  # Disable 'de save_config' checks.
  export DEVENV_FORCE_SAVE_CONFIG=true

  echo
  echo "** Updating self"
  boost_ignore_cmds=\$(de get_var boost.ignore_cmds "")
  de unset_all_vars
  de update self
  echo
  echo "** Configuring modules"

  [ -z "\$TARGET_ARCHITECTURE" ] && TARGET_ARCHITECTURE=\$SYS_ARCH
  if [ "\$SYS_NAME" = "windows" ]; then
    [ -z "\$MSVC_VERSIONS" ] && MSVC_VERSIONS=\$(devenvSelectBestMSVCVersion "\$TARGET_ARCHITECTURE")
    echo "** Will use Visual Studio \$MSVC_VERSIONS"
  fi

  set -e
  de configure_modules --none
  de configure_modules kiara llvm3
  de set_var ALLMODULES.target_arch "\$TARGET_ARCHITECTURE"
  [ -n "\$OPTIMIZATION" ] && de set_var ALLMODULES.optimization "\$OPTIMIZATION"

  if [ "\$SYS_NAME" = "windows" ]; then
    de set_var ALLMODULES.msvc_versions "\${MSVC_VERSIONS:?}"
  fi

  de set_var ALLMODULES.output_verbosity "${OUTPUT_VERBOSITY:?}"

  de set_var boost.version "${BOOST_VERSION:?}"
  de set_var kiara.git_branch "${KIARA_BRANCH:?}"
  [ -n "\$LLVM_BRANCH" ] && de set_var llvm3.git_branch "\${LLVM_BRANCH:?}"
  de set_var boost.ignore_cmds "\$boost_ignore_cmds"
  de save_config

  de info
  set -e

  #set -x
  de update --pull
  de configure_modules \$(devenvGetActiveModules)
  de update --pull
  #set +x

  # Print information
  de info

  # Workaround. Currently we need to get scons-builder manually
  # it is not a part of kiara Repo and can't be cloned automatically
  echo "Current directory: \$PWD"
  pwd

  cd \$PROJ_DIR
  if [ ! -d kiara/builder ]; then
    (
      cd kiara
      git clone https://git.cg.uni-saarland.de/projects/scons-builder.git builder
    )
  else
    (
      cd kiara/builder
      git pull --rebase
    )
  fi

  de build boost -j
  de set_var boost.ignore_cmds build

  de build -j

  set -e
  cd "\$PROJ_DIR"
  echo "* Current directory \$PWD"
  echo "* Running tests"
  source "kiara/devenv/test.sh"

  echo "* Building SDK"
  make_kiara_sdk
EOF
chmod +x build.sh

# Start devenv and run build script
./run_devenv --no-clear-screen run "source ./build.sh"
