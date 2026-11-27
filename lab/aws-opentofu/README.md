# MPCoMPC lab on AWS (OpenTofu)

Builds the test network for the MPC Outlook add-in: one VPC with six small Ubuntu 24.04 servers
that set themselves up on first boot. MPC works in **both directions** between `green.mpc` (alice)
and `blue.mpc` (bob). Each chunk passes through **two linked relays**: relay-a with relay-c, and
relay-b with relay-d.

```
                         chunk 1:  relay-a ◄────► relay-c
alice@green.mpc                   (provider A)   (provider C)                      bob@blue.mpc
  mail.green.mpc  ◄──────►                                         ◄──────►  mail.blue.mpc
  (mta-sender)               chunk 2:  relay-b ◄────► relay-d                  (mx-recipient)
                                  (provider B)   (provider D)
```

Each mail domain publishes its **own** entry relays as SRV records. Chunk n of a message enters
at the destination's n-th relay, which hands it to its linked partner, which delivers it:

| SRV record | Target | Priority |
|---|---|---|
| `_mpc1._tcp.blue.mpc` | `relay.provider-a.test` | 10 |
| `_mpc2._tcp.blue.mpc` | `relay.provider-b.test` | 11 |
| `_mpc1._tcp.green.mpc` | `relay.provider-c.test` | 10 |
| `_mpc2._tcp.green.mpc` | `relay.provider-d.test` | 11 |

| Direction | Chunk 1 | Chunk 2 |
|---|---|---|
| alice → bob (`blue.mpc`) | relay-a → relay-c → `mail.blue.mpc` | relay-b → relay-d → `mail.blue.mpc` |
| bob → alice (`green.mpc`) | relay-c → relay-a → `mail.green.mpc` | relay-d → relay-b → `mail.green.mpc` |

| Server | Name | Address | Role |
|---|---|---|---|
| `mta-sender` | `mail.green.mpc` | 10.20.1.10 | green.mpc mail server (alice) + DNS for green.mpc |
| `mx-recipient` | `mail.blue.mpc` | 10.20.4.10 | blue.mpc mail server (bob) + DNS for blue.mpc |
| `relay-a` | `relay.provider-a.test` | 10.20.2.10 | entry relay for blue.mpc; linked to relay-c |
| `relay-b` | `relay.provider-b.test` | 10.20.3.10 | entry relay for blue.mpc; linked to relay-d |
| `relay-c` | `relay.provider-c.test` | 10.20.5.10 | entry relay for green.mpc; linked to relay-a |
| `relay-d` | `relay.provider-d.test` | 10.20.6.10 | entry relay for green.mpc; linked to relay-b |

How the pieces fit:

- **DNS** (dnsmasq on **both** mail servers): each mail server is the DNS server for its own
  domain. `mail.green.mpc` holds green.mpc's A, MX and SRV records; `mail.blue.mpc` holds
  blue.mpc's. Each also carries the relay names (`relay.provider-*.test`) and forwards questions
  about the other mail domain to that domain's DNS server, as a resolver follows a delegation. So
  either server can answer for both domains, but each domain's records live only on its own
  server. All six servers list both DNS servers. To change which relays a domain uses, edit
  `mpc_srv` in `main.tf`; to change which relays are linked, edit `relay_links`.
- **Mail servers** (Postfix + Dovecot): both apply your X-MPC-Via rule from
  `postfix-3.6.4/etc/postfix/header_checks` to mail their user **submits** from Outlook (port 587).
  This uses a separate Postfix cleanup service, `mpccleanup`, which only the submission service
  uses. Chunks **arriving** from the relays on port 25 skip the rule and are delivered to the
  mailbox. If they didn't, they would be sent straight back out in a loop.
- **Relays** (Postfix only): each has a transport table (`/etc/postfix/transport`) that sends mail
  for the domains it's an entry relay of to its linked partner. Mail for any other domain goes to
  that domain's MX. No relay applies the X-MPC-Via rule.

Your laptop can reach only ports 587, 993 and 53 on the two mail servers, and only from
`admin_cidr`. Mail between the servers stays on private addresses inside the VPC.

**About the domain names:** `.mpc` isn't a real top-level domain, so `green.mpc` and `blue.mpc`
exist only in the lab's two DNS servers. Neither ever forwards a lab name to the internet, and the
servers send every lookup for lab names to the lab DNS servers. To use other names, change the two
`domain` / `fqdn` pairs in `main.tf`; the DNS records and outputs follow automatically.

## Before you start

- [OpenTofu](https://opentofu.org/docs/intro/install/) 1.6 or later.
- The AWS CLI signed in to your account (`aws sts get-caller-identity` works).
- In `terraform.tfvars`, set `region` to the region where your account allows EC2
  (`ap-southeast-2` for yours) and set `admin_cidr` to your laptop's public IP address plus `/32`.

## Create or update the lab

```powershell
tofu plan
tofu apply
```

The servers need **3–5 minutes** after `apply` to finish setting themselves up. Each writes
`/var/lib/mpc-lab/ready` when done; the log is in `/var/log/cloud-init-output.log`.

When you change the lab design, `apply` **replaces** every server whose setup changed, and their
mailboxes are lost. The two mail servers keep their public IP addresses (Elastic IPs) and the
passwords stay the same. (Adding the sender checks below changes every server's setup, so the
first `apply` after it replaces all six. Users added with `New-LabMailUser.ps1` must be added
again.)

## Check it works

From your laptop, each domain's own DNS server should return both of its SRV records:

```powershell
tofu output -raw check_srv_records | Invoke-Expression
```

green.mpc is asked at `mail.green.mpc` (→ `relay.provider-c.test` priority 10,
`relay.provider-d.test` priority 11), and blue.mpc at `mail.blue.mpc` (→ `relay.provider-a.test`
10, `relay.provider-b.test` 11).

`tofu output mpc_routes` shows the path each chunk takes.

## Set up Outlook (2013)

1. Add the line from `tofu output -raw addin_settings_ini` (`DnsServers=<green IP>, <blue IP>`) to
   `%APPDATA%\MpcOutlookAddIn\settings.ini`, then restart Outlook. The add-in asks the first
   server, and the second only if the first doesn't answer; either one answers for both domains.
2. Add both accounts with **File → Add Account → Manual setup or additional server types → POP or
   IMAP**. `tofu output outlook_sender_account` / `outlook_recipient_account` give the details
   (`alice@green.mpc`, `bob@blue.mpc`), and `tofu output -raw alice_password` / `bob_password`
   give the passwords.
   - User name is just `alice` or `bob`.
   - **More Settings → Outgoing Server**: tick *My outgoing server requires authentication*.
   - **More Settings → Advanced**: IMAP **993 / SSL**, SMTP **587 / TLS**.
   - Accept the certificate warnings.
3. If the accounts already exist from an earlier version of the lab, just change their **Email
   Address** (**File → Account Settings → Change**). The servers and user names stay the same.
4. Automatic rejoining watches only the **default** account's Inbox. For the other account, click
   **Rejoin MPC Chunks** in its Inbox.

## Add more mail users

Besides alice and bob, you can add accounts on either mail server from your own computer.
[`utils\mpc-lab-mail-users\New-LabMailUser.ps1`](../../utils/mpc-lab-mail-users) runs
`add-mail-user.sh` from the same folder on the server through Systems Manager (no SSH), then prints
the Outlook settings:

```powershell
cd utils\mpc-lab-mail-users
.\New-LabMailUser.ps1 -Domain blue.mpc  -UserName carol -Region ap-southeast-2
.\New-LabMailUser.ps1 -Domain green.mpc -UserName dave  -Password 'Lab-Pass-2026' -Region ap-southeast-2
.\New-LabMailUser.ps1 -Domain blue.mpc  -UserName carol -ResetPassword -Region ap-southeast-2
```

- `-Region` is the `region` from your `terraform.tfvars`. Without `-Password`, a password is
  generated. `-NamePrefix` and `-AwsProfile` cover a non-default `name_prefix` or AWS CLI profile.
- If PowerShell refuses to run the script, run it with
  `powershell -ExecutionPolicy Bypass -File .\New-LabMailUser.ps1 ...` instead.
- Accounts are Linux users with no login shell, like alice and bob. Mail to `carol@blue.mpc` is
  delivered to `~carol/Maildir` straight away, including MPC chunks from the other domain.
- The password travels in the Run Command, so it shows in that command's history in the Systems
  Manager console. Don't use a real password.
- Accounts live on the instances, so `tofu destroy` (or any change that replaces the mail servers)
  removes them. Run the script again afterwards.
- Without the AWS CLI, connect to the server (EC2 console → instance → **Connect → Session
  Manager**), copy `add-mail-user.sh` there, and run `sudo sh add-mail-user.sh carol`.

## Verify the path a chunk took

Open a chunk in Outlook, go to **File → Properties**, and read the `Received:` lines in the
**Internet headers** from the bottom up. For chunk 1 from alice to bob there are four hops:

```
by mail.blue.mpc             from relay.provider-c.test [10.20.5.10]
by relay.provider-c.test from relay.provider-a.test [10.20.2.10]
by relay.provider-a.test from mail.green.mpc [10.20.1.10]
by mail.green.mpc          from your laptop (ESMTPSA = alice logged in)
```

From bob to alice, chunk 1 shows relay-c first, then relay-a. `tofu output mpc_routes` lists all
four routes. On any server (EC2 console → instance → **Connect → Session Manager**), run
`sudo -i`, then `grep -E 'relay=|status=' /var/log/mail.log | tail`.

| Server | alice → bob | bob → alice |
|---|---|---|
| `mta-sender` / `mx-recipient` (sending side) | `relay=relay.provider-a.test[10.20.2.10]` and `relay=relay.provider-b.test[10.20.3.10]` | `relay=relay.provider-c.test[10.20.5.10]` and `relay=relay.provider-d.test[10.20.6.10]` |
| entry relay | relay-a → `relay=relay.provider-c.test`, relay-b → `relay=relay.provider-d.test` | relay-c → `relay=relay.provider-a.test`, relay-d → `relay=relay.provider-b.test` |
| partner relay | relay-c / relay-d → `relay=mail.blue.mpc[10.20.4.10]` | relay-a / relay-b → `relay=mail.green.mpc[10.20.1.10]` |
| receiving mail server | `to=<bob@blue.mpc>, relay=local … delivered to maildir` | `to=<alice@green.mpc>, relay=local …` |

## Sender checks (stage 1)

Stage 1 of malware detection: checks on *who sent* a chunk, not on its contents, so no MPC is
needed. Each chunk is checked where it enters the recipient's MPC providers (relay-a/b for mail to
blue.mpc, relay-c/d for mail to green.mpc), and the Outlook add-in rejoins a message only when
every chunk passed (its `RequireSenderChecks` setting).

| Server | Adds | Checks | Action |
|---|---|---|---|
| DNS (each mail domain) | SPF `v=spf1 ip4:<its mail server> -all`, DKIM key at `mpc._domainkey.<domain>`, DMARC `v=DMARC1; p=reject; adkim=s; aspf=r` | — | — |
| Sending mail server (port 587) | a DKIM signature (OpenDKIM), also over `X-MPC-Via/-ID/-Chunk/-Scheme` | the logged-in user owns the MAIL FROM address, and the From: header is that address (`mpc-fromcheck`) | refuses a MAIL FROM the user doesn't own (553), and a From: that isn't the MAIL FROM (553), before signing |
| Recipient's entry relays (port 25) | `Authentication-Results: relay.provider-x.test; spf=… dkim=… dmarc=…` | blocklist (`blocked_cidrs`), DKIM (OpenDKIM), SPF and DMARC (OpenDMARC) | refuses blocklisted clients. First **removes every `Authentication-Results` line in the chunk**, so the sender can't write its own results; then checks, **tags** and passes the chunk on. Holds mail if a checker is down |
| Partner relays (second hop, port 2525) | — | nothing: only the relays may use this port | passes the chunk on with the entry relay's results, to the recipient's mail server on port 2525 |
| Recipient's mail server | — | — | on **port 2525** (relays only): keeps only the `Authentication-Results` of its own domain's entry relays. On **port 25** (any other server): removes them all, so a server that sends chunks straight here, past the relays, can't bring "pass" results; then checks DKIM, SPF and DMARC itself and adds `Authentication-Results: mail.<domain>; …` (it tags, for ordinary mail; the add-in doesn't use these) |
| Outlook add-in | — | for each chunk, the results from the relay in its `X-MPC-Via`: `spf=pass`, `dkim=pass` from the From: domain and `dmarc=pass`, and no other SPF, DKIM or DMARC result; the From: address is the envelope sender (`Return-Path`); all chunks have the same From: | rejoins only if all pass; otherwise lists the message under "failed the sender checks" |

The relays tag rather than reject: if relay-a rejected chunk 1 and relay-b accepted chunk 2, bob
would be left with half a message. The add-in makes one decision for the whole message.

**Check the records** from your laptop:

```powershell
tofu output -raw check_sender_records | Invoke-Expression
```

**See the results** on a chunk: in Outlook, **File → Properties → Internet headers**. A chunk to
bob that passed carries three lines from its entry relay:

```
Authentication-Results: relay.provider-a.test; dmarc=pass (p=reject dis=none) header.from=green.mpc
Authentication-Results: relay.provider-a.test; dkim=pass (2048-bit key; unprotected) header.d=green.mpc ...
Authentication-Results: relay.provider-a.test; spf=pass smtp.mailfrom=green.mpc
```

**Send a forged message** from the wrong server: on `mx-recipient` (Session Manager, `sudo -i`),
send "alice's" mail straight to blue.mpc's entry relay, unsigned:

```bash
swaks --server relay.provider-a.test --from alice@green.mpc --to bob@blue.mpc \
      --header "From: Alice <alice@green.mpc>" --header "Subject: forged"
```

bob receives it with `spf=fail`, `dkim=none` and `dmarc=fail` from relay-a. (It isn't an MPC
chunk, so the add-in doesn't rejoin it; a forged chunk would be listed under "failed the sender
checks".)

**Block a server**: add `blocked_cidrs = ["10.20.4.10/32"]` to `terraform.tfvars` and run
`tofu apply` (this replaces the relays). Mail from mail.blue.mpc is then refused at the relays
with `554 5.7.1 … Listed on the MPC lab blocklist`, and bob gets a bounce for alice-bound mail.

**On a server**, the checks log to `/var/log/mail.log` (`grep -E 'opendkim|opendmarc|mpc-fromcheck'`);
`systemctl status opendkim opendmarc` shows the checkers, and on a mail server
`systemctl status mpc-fromcheck` the From: check.

## Troubleshooting

| Symptom | Check |
|---|---|
| `explicit deny in a service control policy` | `region` must be the one your account allows EC2 in |
| Message goes out unsplit | `DnsServers=` line in `settings.ini` and Outlook restarted; the SRV check above; `%LOCALAPPDATA%\MpcOutlookAddIn\mpc.log` |
| Outlook can't connect | Your public IP address still matches `admin_cidr`; setup finished (`/var/lib/mpc-lab/ready`) |
| Mail stuck on a server | `postqueue -p` and `/var/log/mail.log` on that server |
| Mail held on a relay or a mail server with `451 4.7.1 Service unavailable - try again later` | A sender-check service is down: `systemctl status opendkim opendmarc` on that server (and `mpc-fromcheck` on a mail server) |
| Outlook can't send: `553 5.7.1 The From: header must be your own address` | The account's From: address isn't its login@domain. Check the account's email address in Outlook |
| The add-in says a message "failed the sender checks" | The reason is in its report and in `mpc.log`. A lab built before the sender checks doesn't add `Authentication-Results`: run `tofu apply`, or set `RequireSenderChecks=false` in the add-in's settings |
| `dkim=temperror` or `dkim=permerror (no key)` | The DKIM record: `tofu output -raw check_sender_records`, and `systemctl status systemd-resolved` on the relay |

## Cost and clean-up

Six `t3.micro` servers with "standard" CPU credits, plus their public IPv4 addresses, cost
roughly US$2.50–3 a day in Sydney while the lab exists, which the free-plan credits cover. Run
`tofu destroy` to remove everything.

## Notes

- The mail passwords and the DKIM private keys are in the OpenTofu state file and in the servers'
  user data (gzip-compressed, since a script with a DKIM key is over the 16 KB EC2 allows). Treat
  both as secrets; this lab is for testing only. To read the user data:
  `aws ec2 describe-instance-attribute --instance-id <id> --attribute userData --query UserData.Value --output text`,
  then base64-decode and gunzip it.
- These `.tf` files also work with Terraform 1.6 or later.
