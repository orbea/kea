#!/bin/sh

# Copyright (C) 2014-2020 Internet Systems Consortium, Inc. ("ISC")
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This is an utility script that is being included by other scripts.

# Exit with error if commands exit with non-zero and if undefined variables are
# used.
set -eu

# These are the default parameters. They will likely not work in any
# specific deployment. Also used in unit tests.
db_host='localhost'
db_user='keatest'
db_password='keatest'
db_name='keatest'

# Runs all the given arguments as a single command. Maintains quoting. Places
# output in ${OUTPUT} and exit code in ${EXIT_CODE}. Does not support pipes and
# redirections. Support for them could be added through eval and single
# parameter assignment, but eval is not recommended.
# shellcheck disable=SC2034
# SC2034: ... appears unused. Verify use (or export if used externally).
run_and_return_output_and_exit_code() {
    if test -n "${DEBUG+x}"; then
        printf '%s\n' "${*}" >&2
    fi
    set +e
    OUTPUT=$("${@}")
    EXIT_CODE=${?}
    set -e
}

# There are two ways of calling this method.
# mysql_execute SQL_QUERY - This call is simpler, but requires db_user,
#     db_password and db_name variables to be set.
# mysql_execute SQL_QUERY PARAM1 PARAM2 .. PARAMN - Additional parameters
#     may be specified. They are passed directly to mysql. This one is
#     more convenient to use if the script didn't parse db_user db_password
#     and db_name.
#
# It returns the mysql command exit status to the caller as $?
mysql_execute() {
    QUERY=$1
    shift
    if [ $# -gt 1 ]; then
        mysql -N -B "$@" -e "${QUERY}"
    else
        # Shellcheck complains about variables not being set. They're set in the script that calls this script.
        # shellcheck disable=SC2154
        mysql -N -B --host="${db_host}" --database="${db_name}" --user="${db_user}" --password="${db_password}" -e "${QUERY}"
    fi
}

mysql_execute_script() {
    file=$1
    shift
    if [ $# -ge 1 ]; then
        mysql -N -B "$@" < "${file}"
    else
        mysql -N -B --host="${db_host}" --database="${db_name}" --user="${db_user}" --password="${db_password}" < "${file}"
    fi
}

mysql_version() {
    mysql_execute "SELECT CONCAT_WS('.', version, minor) FROM schema_version" "$@"
}

checked_mysql_version() {
    run_and_return_output_and_exit_code \
        mysql_execute "SELECT CONCAT_WS('.', version, minor) FROM schema_version" "$@"

    if [ "${EXIT_CODE}" -ne 0 ]
    then
        printf "Failed to get schema version, mysql status  %s\n" "${EXIT_CODE}"
    fi

    printf '%s\n' "${OUTPUT}"
    return "${EXIT_CODE}"
}

# Submits given SQL text to PostgreSQL
# There are two ways of calling this method.
# pgsql_execute SQL_QUERY - This call is simpler, but requires db_user,
#     db_password and db_name variables to be set.
# pgsql_execute SQL_QUERY PARAM1 PARAM2 .. PARAMN - Additional parameters
#     may be specified. They are passed directly to pgsql. This one is
#     more convenient to use if the script didn't parse db_user db_password
#     and db_name.
#
# It returns the pgsql command exit status to the caller as $?
pgsql_execute() {
    QUERY=$1
    shift
    if [ $# -gt 0 ]; then
        echo "${QUERY}" | psql --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q "$@"
    else
        export PGPASSWORD=$db_password
        echo "${QUERY}" | psql --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q -U "${db_user}" -d "${db_name}"
    fi
}

# Submits SQL in a given file to PostgreSQL
# There are two ways of calling this method.
# pgsql_execute SQL_FILE - This call is simpler, but requires db_user,
#     db_password and db_name variables to be set.
# pgsql_execute SQL_FILE PARAM1 PARAM2 .. PARAMN - Additional parameters
#     may be specified. They are passed directly to pgsql. This one is
#     more convenient to use if the script didn't parse db_user db_password
#     and db_name.
#
# It returns the pgsql command exit status to the caller as $?
pgsql_execute_script() {
    file=$1
    shift
    if [ $# -gt 0 ]; then
        psql --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q -f "${file}" "$@"
    else
        export PGPASSWORD=$db_password
        psql --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q -U "${db_user}" -d "${db_name}" -f "${file}"
    fi
}

pgsql_version() {
    pgsql_execute "SELECT version || '.' || minor FROM schema_version" "$@"
}

checked_pgsql_version() {
    run_and_return_output_and_exit_code \
        pgsql_execute "SELECT version || '.' || minor FROM schema_version" "$@"

    if [ "${EXIT_CODE}" -ne 0 ]
    then
        printf "Failed to get schema version, pgsql status %s\n" "${EXIT_CODE}"
    fi

    printf '%s\n' "${OUTPUT}"
    return "${EXIT_CODE}"
}

cql_execute() {
    query=$1
    shift
    if [ $# -gt 1 ]; then
        run_and_return_output_and_exit_code \
            cqlsh "$@" -e "$query"
    else
        run_and_return_output_and_exit_code \
            cqlsh -u "${db_user}" -p "${db_password}" -k "${db_name}" -e "${query}"
    fi

    if [ "${EXIT_CODE}" -ne 0 ]; then
        printf "cqlsh returned with exit status %s\n" "${EXIT_CODE}"
    fi

    printf '%s\n' "${OUTPUT}"
    return "${EXIT_CODE}"
}

cql_execute_script() {
    file=$1
    shift
    if [ $# -gt 1 ]; then
        run_and_return_output_and_exit_code \
            cqlsh "$@" -e "$file"
    else
        run_and_return_output_and_exit_code \
            cqlsh -u "${db_user}" -p "${db_password}" -k "${db_name}" -f "${file}"
    fi

    if [ "${EXIT_CODE}" -ne 0 ]; then
        printf "cqlsh returned with exit status %s\n" "${EXIT_CODE}"
    fi

    printf '%s\n' "${OUTPUT}"
    return "${EXIT_CODE}"
}

cql_version() {
    run_and_return_output_and_exit_code \
        cql_execute "SELECT version, minor FROM schema_version" "$@"
    version="${OUTPUT}"
    select_exit_code="${EXIT_CODE}"
    version=$(echo "$version" | grep -A 1 "+" | grep -v "+" | tr -d ' ' | cut -d "|" -f 1-2 | tr "|" ".")
    echo "$version"
    return "${select_exit_code}"
}

# recount IPv4 leases from scratch
_RECOUNT4_QUERY=\
"
START TRANSACTION; \
DELETE FROM lease4_stat; \
INSERT INTO lease4_stat (subnet_id, state, leases) \
    SELECT subnet_id, state, COUNT(*) \
    FROM lease4 WHERE state = 0 OR state = 1 \
    GROUP BY subnet_id, state; \
COMMIT;"
export _RECOUNT4_QUERY

# recount IPv6 leases from scratch
_RECOUNT6_QUERY=\
"
START TRANSACTION; \
DELETE FROM lease6_stat; \
INSERT INTO lease6_stat (subnet_id, lease_type, state, leases) \
    SELECT subnet_id, lease_type, state, COUNT(*) \
    FROM lease6 WHERE state = 0 OR state = 1 \
    GROUP BY subnet_id, lease_type, state; \
COMMIT;"
export _RECOUNT6_QUERY
