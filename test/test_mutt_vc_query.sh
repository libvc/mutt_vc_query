#!/bin/bash

# Copyright © 2019 Rafael Laboissière <rafael@laboissiere.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

### Set the environment variable PREFIX to the directory where the
### mutt_vc_query command is located.  Defaults to the empty string, which
### means that the installed version of mutt_vc_query .  Make it is
### exercised.  N.B.: There *_must_* be a leading slash in its value.
: ${PREFIX-:=}
MUTT_VC_QUERY=${PREFIX}mutt_vc_query

### The following functions (`has', `has_dir', and `run') are taken from
### the Debian package autodep8.

has() {
    file="$1"
    shift
    mkdir -p $(dirname "$file")
    if [ $# -gt 0 ]; then
        printf "$@" > "$file"
        echo >> "$file"
    else
        touch "$file"
    fi
}

has_dir() {
    mkdir -p "$@"
}

### These are shunits2 functions that must be defined for our tests.

setUp() {
    tmpdir=$(mktemp -d)
    cd "$tmpdir"
    has contacts.vcf "
BEGIN:VCARD
version:3.0
fn:Joe First
email;type=internet:joe.first@mail.com
email;type=internet:joe.first@mail.net
note:b
END:VCARD

BEGIN:VCARD
version:3.0
fn:Joe Second
email;type=internet:joe.second@mail.com
note:a
END:VCARD
"
}

origdir=$(pwd)
tearDown() {
    cd "$origdir"
    rm -rf "$tmpdir"
}

### Helper function for calling actual queries
assert_query() {
    assertEquals "$2" "$($MUTT_VC_QUERY -f $tmpdir/contacts.vcf $1 | grep -v Searching)"
}

### Check simple call
test_mutt_vc_query_simple_call() {
    assertEquals "$($MUTT_VC_QUERY 2>&1 > /dev/null)" "Invalid number of arguments."
}

### Check call with -h option
test_mutt_vc_query_help_option() {
    assertTrue "$MUTT_VC_QUERY -h > /dev/null"
}

### Check call with -v option
test_mutt_vc_query_version_option() {
    assertEquals "mutt_vc_query 005" "$($MUTT_VC_QUERY -v)"
}

### Check call with -V option
test_mutt_vc_query_version_copyright() {
    assertTrue "$MUTT_VC_QUERY -V > /dev/null"
}

### Check simple
test_mutt_vc_query_simple_query() {
    assert_query First "joe.first@mail.com	Joe First	 "
}

### Check multiple results
test_mutt_vc_query_multiple_results() {
    assert_query Joe "joe.first@mail.com	Joe First	 
joe.second@mail.com	Joe Second	 "
}

### Check sorting by the note field
test_mutt_vc_query_sort_by_note_field() {
    assert_query "-m -t note Joe" "joe.second@mail.com	Joe Second	a
joe.first@mail.com	Joe First	b"
}

### Check chowing all email addresses
test_mutt_vc_query_sort_by_all_email_addresses() {
    assert_query "-a First" "joe.first@mail.com	Joe First	 
joe.first@mail.net	Joe First	 "
}

### Run the unit tests

# Enfore the C locale, since error messages are used in some test cases
export LANG=C

. shunit2
