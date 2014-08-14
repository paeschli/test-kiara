#!/bin/bash

abspath_cd() {
    if [ -d "$1" ]; then
        echo "$(cd "$1"; pwd)"
    else
        case "$1" in
            "" | ".") echo "$PWD";;
            /*) echo "$1";;
            *)  echo "$PWD/$1";;
        esac
    fi
}

THIS_DIR=$(abspath_cd $(dirname "$0"))

AUTHOR_FILTER=
DEFAULT_LICENSE=GPL
DEFAULT_COPYRIGHT_HOLDER="German Research Center for Artificial Intelligence \(DFKI\)"
declare -i DONT_ASK=0
declare -i END_OF_OPTIONS=0

while [ $# -ne 0 ]; do
    case "$1" in
	--)
	    END_OF_OPTIONS=1
	    ;;
	--gpl|--GPL)
            DEFAULT_LICENSE=GPL
            ;;
	--lgpl|--LGPL)
            DEFAULT_LICENSE=LGPL
            ;;
        --author-filter=*)
            AUTHOR_FILTER=${1:16}
	    if [ -z "$AUTHOR_FILTER" ]; then
                echo >&2 "No author specified in option: $1 (e.g. --author-filter=Name)"
	        exit 1
	    fi
            ;;
        --default-copyright-holder=*)
            DEFAULT_COPYRIGHT_HOLDER=${1:27}
	    if [ -z "$DEFAULT_COPYRIGHT_HOLDER" ]; then
                echo >&2 "No default copyright holder specified in option: $1 (e.g. --default-copyright-holder=Name)"
	        exit 1
	    fi
            ;;
        --dont-ask)
            DONT_ASK=1
            ;;
	-*)
	    echo >&2 "unknown option $1"
	    exit 1
	    ;;
    esac
    shift
    [ $END_OF_OPTIONS -eq 1 ] && break
done

echo "Author filter: $AUTHOR_FILTER"
echo "Default license: $DEFAULT_LICENSE"
echo "Default copyright holder: $DEFAULT_COPYRIGHT_HOLDER"
echo "Don't ask anything: $DONT_ASK"

# ask prompt default
ask() {
    local prompt=$1
    local default=$2

    echo "$1"
    [ -n "$default" ] && echo -n "[$default] "
    read -e

    [ -z "$REPLY" ] && REPLY=$default
}

EXCLUDES="
tools/json
tools/check.py
devenv/winpexpect
src/KIARA/LLVM/AsmParser
src/tools/lli.cpp
src/tools/RecordingMemoryManager.h
src/tools/RecordingMemoryManager.cpp
src/tools/RemoteTarget.cpp
src/tools/RemoteTarget.h
test/tools/easyprocess
test/tools/colorama
src/lemon.c
src/lempar.c
src/bin2c.c
"

EXCLUDE_PATTERN=
for i in $EXCLUDES; do
    EXCLUDE_PATTERN="$EXCLUDE_PATTERN$i\|"
done
EXCLUDE_PATTERN=${EXCLUDE_PATTERN:0:${#EXCLUDE_PATTERN}-2}

THIS_DIR=$(abspath_cd "$(dirname "$0")")
if [ -z "$1" ]; then
    # We operate in root directory
    cd "$THIS_DIR/.."
    FILES=$(find . -name "*.hpp" -or -name "*.cpp" -or -name "*.py" -or -name "*.h" -or -name "*.c" | xargs grep -L Copyright | grep -v "$EXCLUDE_PATTERN")
else
    echo "<$1>"
    FILES="$@"
fi

for f in $FILES; do
    # First author
    INFO_STR=$(git log --pretty="format:%an|%ae|%ai|%s" "$f" | tail -n1)

    if [ -z "$INFO_STR" ]; then
        continue
    fi

    oldIFS=$IFS
    IFS="|"
    INFOS=($INFO_STR)
    IFS=$oldIFS

    AUTHOR=${INFOS[0]}
    AUTHOR_EMAIL=${INFOS[1]}
    AUTHOR_YEAR=$(echo "${INFOS[2]}" | sed "s|\([^-]\+\)-.*|\1|g")

    YEARS=($(git log --pretty="format:%ai" "$f" | sed "s|\([^-]\+\)-.*|\1|g" | sort -u))

    YEARS_STR=""
    for ((i=0; i<${#YEARS[*]}; i++)); do
        if [ $i -ne $(( ${#YEARS[*]}-1 )) ]; then
            YEARS_STR="${YEARS_STR}${YEARS[$i]}, "
        else
            YEARS_STR="${YEARS_STR}${YEARS[$i]}"
        fi
    done

    if [ -n "$AUTHOR_FILTER" -a "$AUTHOR_FILTER" != "$AUTHOR" ]; then
        echo "--- Skipping $f (\"$AUTHOR_FILTER\" != \"$AUTHOR\") ---"
        continue
    fi

    echo "******* Processing $f *******"

    head "$f"
    echo "..."

    echo
    echo "First commit: ${INFOS[3]}"
    echo "First commit author: $AUTHOR <$AUTHOR_EMAIL>"
    echo "First commit year: $AUTHOR_YEAR"

    echo "All other authors (with git blame):"

    git blame -f "$f" | perl -w -e '
my %authorToNumLines = ();
my @files = ();
while (<>) {
  if (/(\S+)\s+\(([^[:digit:]]*[^[:digit:][:space:]]).*\)/) {
    $file = $1;
    $author = $2;
    push @files, $file;
    # Fix
    if ($author eq "Kristian") {
      $author = "Kristian Sons";
    }
    if ($author eq "Sergiy Byelozoyorov") {
      $author = "Sergiy Byelozyorov";
    }
    if (defined $authorToNumLines{$author}) {
      $authorToNumLines{$author} = $authorToNumLines{$author} + 1;
    }
    else {
      $authorToNumLines{$author} = 1;
    }
  }
}

while (($key, $value) = each %authorToNumLines) {
  $numLinesToAuthor{$value} = $key;
}

my @sorted_keys = sort { ($a <=> $b)*-1 } keys %numLinesToAuthor;

foreach my $key ( @sorted_keys ) {
  $value = $numLinesToAuthor{$key};
  print "$value : $key line(s) of code\n";
}'

    echo

    if [ -z "$DEFAULT_COPYRIGHT_HOLDER" ]; then
        DEFAULT_AUTHOR="$AUTHOR <$AUTHOR_EMAIL>"
    else
        DEFAULT_AUTHOR="$DEFAULT_COPYRIGHT_HOLDER"
    fi

    LIC_AUTHOR=$DEFAULT_AUTHOR
    LIC_YEAR=$YEARS_STR
    LIC_TYPE=$DEFAULT_LICENSE

    if [ "$DONT_ASK" -eq 0 ]; then

        ask "File author (or type skip for next) ?" "$LIC_AUTHOR"
        LIC_AUTHOR=$REPLY

        if [ "$LIC_AUTHOR" = "skip" ]; then
            continue
        fi

        ask "Year (or type skip for next) ?" "$YEARS_STR"
        LIC_YEAR=$REPLY

        if [ "$LIC_YEAR" = "skip" ]; then
            continue
        fi

        ask "File license (GPL/LGPL) (or type skip for next)" "$LIC_TYPE"
        LIC_TYPE=$REPLY

        if [ "$LIC_TYPE" = "skip" ]; then
            continue
        fi
    fi

    case "$LIC_TYPE" in
        gpl|GPL) LIC_TYPE=--gpl;;
        lgpl|LGPL) LIC_TYPE=--lgpl;;
    esac

    "$THIS_DIR/insert_copyright.sh" \
        $LIC_TYPE \
        --year="$LIC_YEAR" \
        --author="$LIC_AUTHOR" "$f"

    echo
    echo
done
