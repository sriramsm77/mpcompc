# MPCoMPC lab on AWS (OpenTofu)

Builds the test network for the MPC Outlook add-in: one VPC with six small Ubuntu 24.04 servers
that set themselves up on first boot. MPC works in **both directions** between `green.mpc` (alice)
and `blue.mpc` (bob). Each chunk passes through **two linked relays**: relay-a with relay-c, and
relay-b with relay-d.

```
                         chunk 1:  relay-a ◄────► relay-c
alice@green.mpc                   (provider A)   (provider C)                      bob@blue.mpc
  mail.green.mpc  ◄──────►                                         ◄──────►  mx.blue.mpc
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
| alice → bob (`blue.mpc`) | relay-a → relay-c → `mx.blue.mpc` | relay-b → relay-d → `mx.blue.mpc` |
| bob → alice (`green.mpc`) | relay-c → relay-a → `mail.green.mpc` | relay-d → relay-b → `mail.green.mpc` |

| Server | Name | Address | Role |
|---|---|---|---|
| `mta-sender` | `mail.green.mpc` | 10.20.1.10 | green.mpc mail server (alice) + lab DNS |
| `mx-recipient` | `mx.blue.mpc` | 10.20.4.10 | blue.mpc mail server (bob) |
| `relay-a` | `relay.provider-a.test` | 10.20.2.10 | entry relay for blue.mpc; linked to relay-c |
| `relay-b` | `relay.provider-b.test` | 10.20.3.10 | entry relay for blue.mpc; linked to relay-d |
| `relay-c` | `relay.provider-c.test` | 10.20.5.10 | entry relay for green.mpc; linked to relay-a |
| `relay-d` | `relay.provider-d.test` | 10.20.6.10 | entry relay for green.mpc; linked to relay-b |

How the pieces fit:

- **Lab DNS** (dnsmasq on `mta-sender`): A, MX and the SRV records above. To change which relays
  a domain uses, edit `mpc_srv` in `main.tf`; to change which relays are linked, edit
  `relay_links`.
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
exist only in the lab DNS. dnsmasq answers for them itself and never forwards those names, and
the servers send every lookup for them to the lab DNS. To use other names, change the two
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
passwords stay the same.

## Check it works

From your laptop, both domains' SRV records should resolve:

```powershell
tofu output -raw check_srv_records | Invoke-Expression
```

`tofu output mpc_routes` shows the path each chunk takes.

## Set up Outlook (2013)

1. Add the line from `tofu output -raw addin_settings_ini` to
   `%APPDATA%\MpcOutlookAddIn\settings.ini`, then restart Outlook.
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

## Verify the path a chunk took

Open a chunk in Outlook, go to **File → Properties**, and read the `Received:` lines in the
**Internet headers** from the bottom up. For chunk 1 from alice to bob there are four hops:

```
by mx.blue.mpc             from relay.provider-c.test [10.20.5.10]
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
| partner relay | relay-c / relay-d → `relay=mx.blue.mpc[10.20.4.10]` | relay-a / relay-b → `relay=mail.green.mpc[10.20.1.10]` |
| receiving mail server | `to=<bob@blue.mpc>, relay=local … delivered to maildir` | `to=<alice@green.mpc>, relay=local …` |

## Troubleshooting

| Symptom | Check |
|---|---|
| `explicit deny in a service control policy` | `region` must be the one your account allows EC2 in |
| Message goes out unsplit | `DnsServers=` line in `settings.ini` and Outlook restarted; the SRV check above; `%LOCALAPPDATA%\MpcOutlookAddIn\mpc.log` |
| Outlook can't connect | Your public IP address still matches `admin_cidr`; setup finished (`/var/lib/mpc-lab/ready`) |
| Mail stuck on a server | `postqueue -p` and `/var/log/mail.log` on that server |

## Cost and clean-up

Six `t3.micro` servers with "standard" CPU credits, plus their public IPv4 addresses, cost
roughly US$2.50–3 a day in Sydney while the lab exists, which the free-plan credits cover. Run
`tofu destroy` to remove everything.

## Notes

- The mail passwords are in the OpenTofu state file and in the servers' user data. Treat both
  as secrets; this lab is for testing only.
- These `.tf` files also work with Terraform 1.6 or later.
