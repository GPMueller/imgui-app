function( git_get_latest_tag INOUT_GIT_TAG INOUT_GIT_BRANCH INOUT_GIT_REV INOUT_GIT_DIFF INOUT_MAJOR INOUT_MINOR INOUT_PATCH )

    #----- Get the most recent ancestor that is shared with the main branch,
    # then get the next child on the ancestry-path to main (so latest develop
    # and derived branches get the correct version number):
    # First get the shared ancestor
    execute_process(
        COMMAND           git merge-base HEAD main
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        RESULT_VARIABLE   GIT_RESULT
        OUTPUT_VARIABLE   GIT_TAG_REV
        ERROR_QUIET )
    if( NOT "${GIT_RESULT}" STREQUAL "0")
        message( ERROR " \"git merge-base HEAD main\" -> ${GIT_RESULT}" )
    endif()
    string( STRIP "${GIT_TAG_REV}" GIT_TAG_REV )
    # Then get the list of children on the path to main
    if( GIT_TAG_REV )
        execute_process(
            COMMAND           git rev-list --reverse --ancestry-path ${GIT_TAG_REV}..main
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            RESULT_VARIABLE   GIT_RESULT
            OUTPUT_VARIABLE   GIT_TAG_REV
            ERROR_QUIET )
        if( NOT "${GIT_RESULT}" STREQUAL "0")
            message( ERROR " \"git rev-list --reverse --ancestry-path ${GIT_TAG_REV}..main\" -> ${GIT_RESULT}" )
        endif()
        string( STRIP "${GIT_TAG_REV}" GIT_TAG_REV )
        # Get the first element of the list, which determines the revision from which to get the tag
        string( REGEX REPLACE "\n" ";" GIT_TAG_REV "${GIT_TAG_REV}" )
        if( GIT_TAG_REV )
            list( GET GIT_TAG_REV 0 GIT_TAG_REV )
        endif()
    endif()

    # Then get the corresponding tag
    execute_process(
        COMMAND           git describe --tags --abbrev=0 ${GIT_TAG_REV}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        RESULT_VARIABLE   GIT_RESULT
        OUTPUT_VARIABLE   GIT_TAG )
    if( NOT "${GIT_RESULT}" STREQUAL "0")
        message( ERROR " \"git describe --tags --abbrev=0\" -> ${GIT_RESULT}" )
    endif()
    string( STRIP "${GIT_TAG}" GIT_TAG )

    #----- Get the branch description
    execute_process(
        COMMAND           git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        RESULT_VARIABLE   GIT_RESULT
        OUTPUT_VARIABLE   GIT_BRANCH )
    if( NOT "${GIT_RESULT}" STREQUAL "0")
        message( ERROR " \"git rev-parse --abbrev-ref HEAD\" -> ${GIT_RESULT}" )
    endif()
    string( STRIP "${GIT_BRANCH}" GIT_BRANCH )

    #----- Get the first few characters of the commit hash
    execute_process(
        COMMAND           git log --pretty=format:%h -n 1
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE   GIT_REV
        ERROR_QUIET )
    if( NOT "${GIT_RESULT}" STREQUAL "0")
        message( ERROR " \"git log --pretty=format:%h -n 1\" -> ${GIT_RESULT}" )
    endif()
    string( STRIP "${GIT_REV}" GIT_REV )

    #----- GIT_DIFF is a "+" if there are uncommitted changes
    execute_process(
        COMMAND           git diff --quiet --exit-code
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        RESULT_VARIABLE   GIT_RESULT
        OUTPUT_VARIABLE   GIT_DIFF )
    if( NOT "${GIT_RESULT}" STREQUAL "0")
        set( GIT_DIFF "+" )
    endif()
    string( STRIP "${GIT_DIFF}" GIT_DIFF )

    #----- Parse the version numbers from "vMajor.Minor.Patch"
    string( REGEX REPLACE "^v([0-9]+)\\..*" "\\1"                VERSION_MAJOR "${GIT_TAG}" )
    string( REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1"          VERSION_MINOR "${GIT_TAG}" )
    string( REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${GIT_TAG}" )
    if( "${VERSION_MAJOR}" STREQUAL "" )
        set( VERSION_MAJOR 0 )
    endif()
    if( "${VERSION_MINOR}" STREQUAL "" )
        set( VERSION_MINOR 0 )
    endif()
    if( "${VERSION_PATCH}" STREQUAL "" )
        set( VERSION_PATCH 0 )
    endif()

    #----- Branch tag
    message( "GOT GIT BRANCH         \"${GIT_BRANCH}\"" )
    if( "${GIT_BRANCH}" STREQUAL "main" )
        set( GIT_BRANCH "" )
    elseif( "${GIT_BRANCH}" STREQUAL "develop" )
        set( GIT_BRANCH "dev" )
    else()
        set( GIT_BRANCH "feature" )
    endif()

    message( "THEREFORE GIT_BRANCH = \"${GIT_BRANCH}\"" )
    message( "GOT GIT TAG REVISION   \"${GIT_TAG_REV}\"" )
    message( "GOT GIT TAG            \"${GIT_TAG}\"" )
    message( "GOT GIT REV            \"${GIT_REV}\"" )
    message( "GOT GIT DIFF           \"${GIT_DIFF}\"" )
    message( "GOT VERSION_MAJOR      \"${VERSION_MAJOR}\"" )
    message( "GOT VERSION_MINOR      \"${VERSION_MINOR}\"" )
    message( "GOT VERSION_PATCH      \"${VERSION_PATCH}\"" )

    #----- Output from the function
    set( ${INOUT_GIT_TAG}    ${GIT_TAG}       PARENT_SCOPE )
    set( ${INOUT_GIT_BRANCH} ${GIT_BRANCH}    PARENT_SCOPE )
    set( ${INOUT_GIT_REV}    ${GIT_REV}       PARENT_SCOPE )
    set( ${INOUT_GIT_DIFF}   ${GIT_DIFF}      PARENT_SCOPE )
    set( ${INOUT_MAJOR}      ${VERSION_MAJOR} PARENT_SCOPE )
    set( ${INOUT_MINOR}      ${VERSION_MINOR} PARENT_SCOPE )
    set( ${INOUT_PATCH}      ${VERSION_PATCH} PARENT_SCOPE )

endfunction()