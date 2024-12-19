#!/bin/bash

if [ "$#" -eq 1 ]; then
    if [ "$1" = "-h" ]; then
        echo "USAGE:"
        echo "    '-b' to build new docker (optional)"
        echo "    '-c <command>' to run command (optional)"
        echo "    '-s' to execute docker in root (optional)"
        exit 0
    fi
fi

PWD=`pwd`
BASENAME=`basename "$PWD"`
preferred_name=$BASENAME":latest"

# Default values:
build_docker=false
is_sudo=false
command=""

# It's the : after d that signifies that it takes an option argument.

while getopts bsc: opt; do
    case $opt in
        b) build_docker=true ;;
        s) is_sudo=true ;;
        c) command=$OPTARG ;;
        *) echo 'error in command line parsing' >&2
           exit 1
    esac
done

shift "$(( OPTIND - 1 ))"

"$build_docker" && echo 'Got the -b option'
"$is_sudo" && echo 'Got the -s option'
printf 'Option -c: %s\n' "$command"

if [ "$build_docker" = "true" ]; then
    docker build -t $preferred_name .
fi

docker images | grep $BASENAME
if [ "$?" -ne 0 ]; then
    echo "it means there is no built docker, cannot proceed"
    exit 1
fi

# here, we probably have the docker, then run it

if [ "$command" != "" ]; then
    echo "command running"
    if [ "$is_sudo" = "true" ]; then
        docker run --rm -t --net=host -v $(pwd):/workspace --entrypoint=/bin/bash $preferred_name -c "$command"
    else
        docker run --rm -t --net=host -v $(pwd):/workspace -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro -u $(id -u ${USER}):$(id -g ${USER}) --entrypoint=/bin/bash $preferred_name -c "$command"
    fi
    exit 0
fi

# if no command run, lets start the docker for personal use
if [ "$is_sudo" = "true" ]; then
    docker run --rm -it --net=host -v $(pwd):/workspace --entrypoint=/bin/bash $preferred_name
else
    docker run --rm -it --net=host -v $(pwd):/workspace -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro -u $(id -u ${USER}):$(id -g ${USER}) --entrypoint=/bin/bash $preferred_name
fi
