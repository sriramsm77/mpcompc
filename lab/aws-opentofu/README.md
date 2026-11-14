# MPCoMPC lab on AWS (OpenTofu)

Builds the test network for the MPC Outlook add-in: one VPC with four small Ubuntu 24.04 servers
that set themselves up on first boot.

```
Your laptop: Outlook + MPC add-in
  alice@sender.test ──587/993──► mta-sender   (10.20.1.10)  lab DNS + Postfix with the X-MPC-Via rule
                                    │ FILTER smtp:relay-a            │ FILTER smtp:relay-b
                                    ▼                                ▼
                     relay-a (10.20.2.10)              relay-b (10.20.3.10)
                     "service provider A"              "service provider B"
                                    └──────────────┬─────────────────┘  normal MX delivery
                                                   ▼
  bob@recipient.test ◄──993/587── mx-recipient (10.20.4.10)  Postfix MX + Dovecot IMAP
```

| Server | Role |
|---|---|
| `mta-sender` (`mail.sender.test`) | dnsmasq DNS for the `.test` domains (A, MX and the `_mpc1`/`_mpc2._tcp` SRV records), Postfix with your `header_checks` rule, SMTP submission and IMAP for **alice** |
| `relay-a` (`relay-a.provider-a.test`) | Postfix relay; no `header_checks` (with the rule, mail would loop) |
| `relay-b` (`relay-b.provider-b.test`) | Same, in a separate subnet |
| `mx-recipient` (`mx.recipient.test`) | MX for `recipient.test`, SMTP submission and IMAP for **bob** |

Mail between the servers uses private addresses inside the VPC. Your laptop reaches only ports
587, 993 and 53 (and 22 if you add an SSH key), and only from the address in `admin_cidr`.

## Before you start

- [OpenTofu](https://opentofu.org/docs/intro/install/) 1.6 or later (`tofu version`).
- An AWS account, and the AWS CLI signed in to it (`aws sts get-caller-identity` works).
- Optional: the [Session Manager plugin](https://docs.aws.amazon.com/systems-manager/latest/userguide/session-manager-working-with-install-plugin.html)
  for the AWS CLI, to open a shell on the servers without SSH.

## Create the lab

```bash
cp terraform.tfvars.example terraform.tfvars
# Edit terraform.tfvars: set admin_cidr to your laptop's public IP address plus /32
#   curl https://checkip.amazonaws.com
tofu init
tofu plan
tofu apply
```

The servers take about 3–5 minutes after `apply` finishes to install and configure everything.
Each one writes `/var/lib/mpc-lab/ready` when it's done. The setup log is in
`/var/log/cloud-init-output.log`.

If your laptop's public IP address changes, update `admin_cidr` and run `tofu apply` again. Only
the security groups change.

## Check it works (before Outlook)

**From your laptop**, the add-in's SRV lookup should find both relays:

```bash
tofu output -raw check_srv_records | sh
# _mpc1._tcp.recipient.test  service = 10 0 25 relay-a.provider-a.test.
```

**On `mta-sender`** (`tofu output shell_access` gives the command), send two fake chunks and one
normal message, as the add-in would:

```bash
sudo -i
PW='<alice password>'   # tofu output -raw alice_password
for via in relay-a.provider-a.test relay-b.provider-b.test; do
  swaks -s 127.0.0.1:587 -tls -a PLAIN -au alice -ap "$PW" -f alice@sender.test -t bob@recipient.test \
        --header "X-MPC-Via: $via" --header "Subject: route test via $via"
done
grep -E 'relay=' /var/log/mail.log | tail -3
```

You should see `relay=relay-a.provider-a.test[10.20.2.10]:25` and
`relay=relay-b.provider-b.test[10.20.3.10]:25`, each with `status=sent`. On `mx-recipient`, both
messages arrive in `/home/bob/Maildir/new/`, and their `Received:` headers show which relay each
one passed through.

## Set up Outlook

You need classic Outlook for Windows with the MPC add-in installed (see
`outlook-mpc-addin/README.md` in the repository).

1. **Point the add-in at the lab DNS.** On the sending laptop, add the line from
   `tofu output -raw addin_settings_ini` to `%APPDATA%\MpcOutlookAddIn\settings.ini` (or use **MPC
   Settings** on the Home tab), then restart Outlook.
2. **Add the accounts** with **File → Add Account → Advanced options → Let me set up my account
   manually → IMAP**. `tofu output outlook_sender_account` and `tofu output
   outlook_recipient_account` give the servers and ports, and `tofu output -raw alice_password` /
   `bob_password` give the passwords.
   - User name is just `alice` or `bob`, not the full email address.
   - Outgoing server requires authentication, using the same user name and password.
   - The servers use self-signed certificates, so Outlook warns about them. Choose to continue (or
     **View certificate → Install** to stop the warnings).
3. **One laptop or two.** You can add both accounts to one Outlook profile. Automatic rejoining
   only watches the **default** account's Inbox, so either make bob's account the default or click
   **Rejoin MPC Chunks** in bob's Inbox.

## Run the test

1. As alice, write to `bob@recipient.test` and click **Send** (or **Send via MPC**).
2. The add-in finds the two SRV records and sends two chunks to `mta-sender`, tagged
   `X-MPC-Via: relay-a…` and `X-MPC-Via: relay-b…`.
3. `mta-sender` sends chunk 1 through relay-a and chunk 2 through relay-b. Both reach
   `mx-recipient`.
4. In bob's Inbox, once both chunks are there, the add-in rejoins them into one message with
   `X-MPC-Chunk: [000/000]` and `X-MPC-Via: relay-a.provider-a.test, relay-b.provider-b.test`, and
   moves the chunks to Deleted Items.

To see the route, run `grep relay= /var/log/mail.log` on `mta-sender`, and look at the chunks'
**File → Properties → Internet headers** in Outlook.

## Troubleshooting

| Symptom | Check |
|---|---|
| Message goes out unsplit | `DnsServers=` line in `settings.ini` and Outlook restarted; `nslookup` check above; `%LOCALAPPDATA%\MpcOutlookAddIn\mpc.log` |
| Outlook can't connect | `admin_cidr` matches your current public IP address; server finished setup (`/var/lib/mpc-lab/ready`) |
| Mail stuck on a server | `postqueue -p` and `/var/log/mail.log` on that server |
| A relay is unreachable | `nc -vz relay-a.provider-a.test 25` on `mta-sender` |

## Cost and clean-up

With the default `t3.micro`, each server costs roughly US$7–8 a month plus about US$3.65 a month
for its public IPv4 address, so about US$45 a month for the lab while it runs. New AWS accounts'
free-plan credits cover that. To pause, stop the instances in the EC2 console (the two Elastic IP
addresses keep their numbers and are still charged). To remove everything:

```bash
tofu destroy
```

## Notes

- The mail passwords are in the OpenTofu state file and in the servers' user data. Treat both
  as secrets and use this lab only for testing.
- `mta-sender` applies the X-MPC-Via rule to every message it handles, including incoming mail
  for alice. That doesn't matter here, because nobody sends MPC chunks to `sender.test`.
- To change the instance size, region or address range, edit `terraform.tfvars`. The DNS records
  follow `vpc_cidr` automatically.
- These `.tf` files also work with Terraform 1.6 or later (`terraform init/apply`).
