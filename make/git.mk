# First check if there are local changes
GITSTATUS:=$(shell git status --porcelain --untracked-files=no)
GITREV:=$(shell git rev-parse --short HEAD)
ifeq ($(GITSTATUS),)
# Then check if there is a tag on current commit
GITTAG:=$(shell git describe --tags --exact-match 2> /dev/null)
ifeq ($(GITTAG),)
# If no tag get branch name and short commit ID
GITBRANCH:=$(shell git rev-parse --abbrev-ref HEAD | sed 's/[a-zA-Z0-9]*\/\([a-zA-Z0-9-]*\)/\1/' | sed 's/\([a-zA-Z0-9]*\-[a-zA-Z0-9/]*\)-.*/\1/')
ifeq ($(GITBRANCH),HEAD)
# No branch name in detached mode, use only short commit ID
VERSION:=unknown-$(GITREV)
else #GITBRANCH
# Use branch name and short commit ID
VERSION:=$(GITBRANCH)-$(GITREV)
endif #GITBRANCH
else #GITTAG
VERSION:=$(GITTAG)
endif #GITTAG
else #GITSTATUS
# Always use dirty name when there are local changes
VERSION:=dirty-$(GITREV)
endif #GITSTATUS
# Display VERSION
$(info VERSION is $(VERSION))