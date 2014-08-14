# $1        :  array variable where to set flags
# $2        :  module ID
_computeAllDependencies() {
    local avar=$1
    local mid=$2
    local deps_ids=$(getRealModuleDirectDepsIDs $mid MODULE_ACTIVE_FLAGS)
    setArrayElem "$avar" "$mid" 1

    for mid in $deps_ids; do
        if [ "$(getArrayElem "$avar" "$mid")" = 0 ]; then
            _computeAllDependencies "$avar" "$mid"
        fi
    done
}

# $1        :  array variable where to clear flag
# $2        :  module ID
# $3        :  value (0 | 1) (optional, default is 0)
_clearModuleFlag() {
    [ "$2" != -1 ] && {
        local v
        if [ -z "$3" ]; then
            v=0
        else
            v=$3
        fi

        setArrayElem "$1" $2 $v
    }
}

make_kiara_sdk() {
    local verbose_opt=
    [ "$1" = -v -o "$1" = --verbose ] && verbose_opt=-v

    declare -a sdk_modules
    clearModulesFlags sdk_modules
    _computeAllDependencies sdk_modules $(getModuleID "kiara")

    # Disable unneeded modules
    _clearModuleFlag sdk_modules $(getModuleID strawberry_perl)

    local dest=$PROJ_DIR/kiara_sdk

    echo "Delete old package in $dest"
    rm -rf "$dest" || return 1
    echo "Creating package in $dest"
    mkdir -p "$dest/bin" "$dest/lib" "$dest/include" "$dest/tests" || return 1

    local module_name
    local n=${#sdk_modules[@]} i
    for ((i=0; $i<$n; i++)); do
        [ "$(getArrayElem sdk_modules $i)" = 0 ] && continue
        module_name=$(firstItem $(getModuleNames $i))
        echo "Processing $module_name"

        build_dir=$(devenvGetModuleBuildDir "$module_name")

        [ -z "$build_dir" ] && continue

        local cpppath=$(devenvGetModuleCPPPATH "$module_name")

        [ -d "$cpppath" ] && \
            cp $verbose_opt -af "$cpppath"/* "$dest/include"
        #[ -d "$build_dir/include" ] && \
        #    cp $verbose_opt -af "$build_dir/include" "$dest"
        [ -d "$build_dir/bin" ] && \
            cp $verbose_opt -af "$build_dir/bin"/* "$dest/bin"
        [ -d "$build_dir/lib" ] && \
            cp $verbose_opt -af "$build_dir/lib"/* "$dest/lib"
    done

    local root_dir=$(devenv_get_var kiara.root_dir)
    cp $verbose_opt -af "$root_dir"/examples "$dest"
    cp $verbose_opt -af "$root_dir"/sdk/* "$dest"
    cp $verbose_opt -af "$root_dir"/devenv/test_examples.py \
                        "$root_dir"/devenv/winpexpect \
                        "$dest"/tests

    local build_mode=$(devenvNormalizeBuildMode "$(devenv_get_var kiara.optimization Release)")
    local target_arch="$(devenv_get_var kiara.target_arch x86)"
    local sdk_arch=$(devenvNormalizeTargetArchV2 $target_arch)
    local sdk_version=$(kiara-version | cut -d' ' -f3)
    cat > "$dest/sdk_info.txt" <<EOF
$(kiara-version)
SDK Architecture: $sdk_arch
SDK Build Mode: $build_mode

Build Date: $(date -R)
Build System Name: $SYS_NAME
Build System ID: $SYS_ID
EOF

    if [ "$SYS_NAME" = "windows" ]; then
        sed $SED_NOCR_OPT 's|$|\r\n|g' "$dest/sdk_info.txt" > "$dest/sdk_info.tmp" && \
        mv "$dest/sdk_info.tmp" "$dest/sdk_info.txt"
    fi

    if [ "$SYS_NAME" != "windows" ]; then
        python -c "import platform; d=platform.dist(); print 'Build Distribution: '+d[0]+' '+d[1]" >> "$dest/sdk_info.txt"
    fi

    local suffix
    if devenv_has_var "kiara.msvc_versions"; then
        local msvc_versions=$(devenv_get_var kiara.msvc_versions)
        echo "SDK built with Visual Studio version: $msvc_versions" >> "$dest/sdk_info.txt"
        suffix=msvc$msvc_versions-$build_mode
    else
        suffix=$build_mode
    fi

    (
        local dist
        cd "$PROJ_DIR"
        if [ "$SYS_NAME" = "windows" ]; then
            dist="$SYS_NAME-$sdk_arch"
        else
            dist=$("$PYTHON" -c "import platform; d=platform.dist(); print d[0]+'-'+d[1]")-$sdk_arch
        fi
        archive_name="kiara-sdk-$sdk_version-$dist-$suffix-$(date -I).tar.bz2"
        echo "Deleting old SDK archives..."
        rm -f kiara-sdk-*
        echo "Creating SDK archive: $archive_name"
        tar cjf "$archive_name" kiara_sdk
    )

    # Hack to copy contrib on Windows
    # [ "$SYS_NAME" != "windows" ] && return 0

    # local module_name DEVENV_THIS_MODULE old_IFS IFS p
    # eval "$LOCAL_ENV_GEN_VARS"
    # for module_name in contrib; do
    #     DEVENV_THIS_MODULE=$module_name
    #     eval "_${module_name}_setup_own_env"
    #     old_IFS=$IFS
    #     IFS=":"
    #     for p in $_PATH; do
    #         [ -d "$p" ] && \
    #             cp $verbose_opt -af "$p"/* "$dest/bin"
    #     done
    #     for p in $_CPPPATH; do
    #         [ -d "$p" ] && \
    #             cp $verbose_opt -af "$p"/* "$dest/include"
    #     done
    #     for p in $_LIBPATH; do
    #         [ -d "$p" ] && \
    #             cp $verbose_opt -af "$p"/* "$dest/lib"
    #     done
    #     IFS=$old_IFS
    #     unset _PATH _CPPPATH _LIBPATH
    # done
}
# vim: set expandtab tabstop=4 shiftwidth=4:
