# This file is part of GNU Mailutils.
# Copyright (C) 2010-2022 Free Software Foundation, Inc.
#
# GNU Mailutils is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.
#
# GNU Mailutils is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>.

# Provide leading quote
1i\
"\\

# Provide trailing quote
$a\
"

# Remove empty lines and comments
/ *#/d
/^ *$/d
# Escape quotes and backslashes
s/["\]/\\&/g
# Add newline and continuation character at the end of each line
s/$/\\n\\/
# End

