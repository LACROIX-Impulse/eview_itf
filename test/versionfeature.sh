#!/bin/bash

function getVersion ()
{
    # First check if there are local changes
    GITSTATUS=$(git status --porcelain --untracked-files=no)
    GITREV=$(git rev-parse --short HEAD)
    if [ -z "${GITSTATUS}" ]
    then
        # Then check if there is a tag on current commit
        GITTAG=$(git describe --tags --exact-match 2> /dev/null)
        if [ -z "${GITTAG}" ]
        then
            # If no tag get branch name and short commit ID
            GITBRANCH=$(git rev-parse --abbrev-ref HEAD | sed 's/[a-zA-Z0-9]*\/\([a-zA-Z0-9-]*\)/\1/' | sed 's/\([a-zA-Z0-9]*\-[a-zA-Z0-9/]*\)-.*/\1/')
            if [ "HEAD" == "${GITBRANCH}" ]
            then
                # No branch name in detached mode, use only short commit ID
                VERSION=unknown-${GITREV}
            else #GITBRANCH
                # Use branch name and short commit ID
                VERSION=${GITBRANCH}-${GITREV}
                if [ "release-EMIRROR" == "${GITBRANCH}" ]
                then
                    GITBRANCHTESTRELEASE=$(git branch --list | grep "*" | cut -c-27 | cut -c-15 --complement)
                    VERSION=${GITBRANCHTESTRELEASE}-${GITREV}
                fi

            fi
        else
            VERSION=${GITTAG}
        fi
    else
        # Always use dirty name when there are local changes
        VERSION=dirty-${GITREV}
    fi
    echo ${VERSION}
}

echo $(getVersion)
