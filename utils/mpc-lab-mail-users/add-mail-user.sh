#!/bin/sh
# Adds a mail account on a lab mail server (mail.green.mpc or mail.blue.mpc).
#
# Run it on the mail server as root (EC2 console -> instance -> Connect -> Session Manager):
#   sudo sh add-mail-user.sh carol              # new account, prints a generated password
#   sudo sh add-mail-user.sh carol 'S3cret-pw'  # new account with this password
#   sudo sh add-mail-user.sh carol '' --reset   # new password for an existing mail account
#
# New-LabMailUser.ps1 runs this script for you from your own computer.
#
# Lab accounts are Linux users with no login shell, the same as alice and bob: Postfix delivers
# their mail to ~/Maildir, and Dovecot checks their password for IMAP and SMTP submission.
# It is POSIX sh, because AWS Systems Manager runs it with /bin/sh (dash on Ubuntu).
set -eu

user="${1:-}"
password="${2:-}"
mode="${3:-}"

# fail MESSAGE [EXIT_CODE]
fail() { echo "error: $1" >&2; exit "${2:-1}"; }

[ "$(id -u)" -eq 0 ] || fail "run this as root (sudo)."
command -v doveadm >/dev/null 2>&1 || fail "Dovecot is not installed here; run this on mail.green.mpc or mail.blue.mpc."

case "$user" in
  "" | *[!a-z0-9_-]* | [!a-z_]*)
    fail "user name must start with a lowercase letter or _, and use only a-z, 0-9, _ and - (got '$user')." 2 ;;
esac
[ "${#user}" -le 32 ] || fail "user name is longer than 32 characters." 2

case "$mode" in
  "" | --reset) ;;
  *) fail "unknown option '$mode' (the only option is --reset)." 2 ;;
esac

generated=no
if [ -z "$password" ]; then
  password=$(tr -dc 'A-Za-z0-9' </dev/urandom | head -c 16)
  generated=yes
fi
[ "${#password}" -ge 8 ] || fail "password must be at least 8 characters." 2
case "$password" in
  *"
"*) fail "password must not contain a line break." 2 ;;
esac

if id "$user" >/dev/null 2>&1; then
  [ "$mode" = "--reset" ] || fail "$user already exists. Add --reset to give it a new password." 3
  # Only touch mail accounts: never root, ubuntu or system users.
  uid=$(id -u "$user")
  shell=$(getent passwd "$user" | cut -d: -f7)
  [ "$uid" -ge 1000 ] && [ "$shell" = "/usr/sbin/nologin" ] ||
    fail "$user is not a mail account (uid $uid, shell $shell); not changing it." 3
  action="Reset the password of"
else
  useradd --create-home --shell /usr/sbin/nologin "$user"
  action="Created"
fi

printf '%s:%s\n' "$user" "$password" | chpasswd

# Create the Maildir now, so IMAP shows an empty Inbox before the first message arrives.
home=$(getent passwd "$user" | cut -d: -f6)
for dir in "$home/Maildir" "$home/Maildir/cur" "$home/Maildir/new" "$home/Maildir/tmp"; do
  install -d -m 700 -o "$user" -g "$user" "$dir"
done

domain=$(postconf -h mydomain)
echo "$action $user@$domain (mailbox $home/Maildir)."

if doveadm auth test "$user" "$password" >/dev/null 2>&1; then
  echo "Dovecot accepts the new password."
else
  echo "warning: Dovecot did not accept the password; check 'doveadm auth test $user'." >&2
fi

[ "$generated" = "no" ] || echo "Password: $password"
