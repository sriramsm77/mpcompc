#!/bin/sh

## mangen.sh
## Copyright (C) 2004 Free Software Foundation, Inc.
##
## GNU Mailutils is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc. 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##
## All mail utilities must be already installed!
## Requires `help2man'. Assume standard `/usr/local' by default.

set -e

if test -z "$1"; then
  prefix="/usr/local"
else
  prefix=$1
fi

export LD_LIBRARY_PATH="$prefix/lib/$DEB_HOST_MULTIARCH"
export GUILE_LOAD_PATH="$prefix/share/guile/site"

help2man="help2man"

bin1="decodemail dotlock.mailutils frm.mailutils from.mailutils mailutils mail.mailutils mailutils-config messages.mailutils mimeview movemail.mailutils popauth putmail readmsg.mailutils sieve"
#bin1scm="guimb"
sbin8="comsatd imap4d lmtpd mda pop3d"
libexec8=""

for program in $bin1; do
  echo "Creating $program.1..."
  if [ "$program" = "mail.mailutils" ]; then
    desc="process mail messages"
  elif [ "$program" = "mailutils-config" ]; then
    desc="deprecated mailutils build configuration tool"
  elif [ "$program" = "mailutils" ]; then
    desc="Mailutils multi-purpose tool"
  else
    desc=$($prefix/bin/$program --help | sed -n '2s/.*-- //p')
  fi
  $help2man -N -i debian/mangen.inc -s 1 -S FSF -n "$desc" \
    $prefix/bin/$program >$program.1
done

for program in $bin1scm; do
  # We need to temporarily change the lib-path so guile
  # can find the Mailutils shared libs.
  cp -a $prefix/share/guile/site/mailutils/mailutils.scm \
    $prefix/share/guile/site/mailutils/mailutils.scm.orig
  sed -i -e"s!/usr/lib!$prefix/lib/$DEB_HOST_MULTIARCH!g" \
    $prefix/share/guile/site/mailutils/mailutils.scm
  desc=$($prefix/bin/$program --help | sed -n '2s/.*-- //p')
  echo "Creating $program.1..."
  $help2man -N -i debian/mangen.inc -s 1 -S FSF -n "$desc" \
    $prefix/bin/$program >$program.1
  mv $prefix/share/guile/site/mailutils/mailutils.scm.orig \
    $prefix/share/guile/site/mailutils/mailutils.scm
done

for program in $sbin8; do
  if [ "$program" = "comsatd" ]; then
    desc="the Comsat daemon"
  else
    desc=$($prefix/sbin/$program --help | sed -n '2s/.*-- //p')
  fi
  echo "Creating $program.8..."
  $help2man -N -i debian/mangen.inc -s 8 -S FSF -n "$desc" \
    $prefix/sbin/$program >$program.8
done

for program in $libexec8; do
  desc=$($prefix/lib/$DEB_HOST_MULTIARCH/mailutils/$program --help | sed -n '2s/.*-- //p')
  echo "Creating $program.8..."
  $help2man -N -i debian/mangen.inc -s 8mailutils -S FSF -n "$desc" \
    $prefix/lib/$DEB_HOST_MULTIARCH/mailutils/$program >$program.8
done

exit 0
