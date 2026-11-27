# MPC Outlook Add-in

A VSTO add-in for **classic Outlook on Windows** that adds the X-MPC logic from this repository's
mailutils changes to Outlook:

- **Sending** (based on `mpc_mail_compose_send__` in `mail/send.c`): looks up `_mpc1._tcp.<domain>`
  and `_mpc2._tcp.<domain>` for the first To recipient, splits the body **and every attachment** into
  **additive secret shares modulo the prime p = 2^61 − 1**, one share per MPC server, and sends each
  share with `X-MPC-Via`, `X-MPC-ID`, `X-MPC-Chunk`, `X-MPC-Result` and `X-MPC-Scheme`. Each MPC
  server relays only its own share, which on its own reveals nothing about the text or attachments.
- **Receiving** (based on `maildir_mpc_coaelesce_mails_unlocked` in `mail/mail.c`): finds messages
  with `X-MPC-*` headers, groups them by `X-MPC-ID`, and once every share has arrived, adds the
  shares back together into one message marked `X-MPC-Chunk: [000/000]` with the combined
  `X-MPC-Via`.

The add-in still rejoins messages from the mailutils `mail` client, whose chunks are plain slices of
the body. `mail` can't read the add-in's shares: they need the sharing code in this add-in (or an
equivalent) on the receiving side.

## How the C code maps to the add-in

| mailutils (C) | Add-in (C#) |
|---|---|
| `MU_HEADER_MPC_*` in `include/mailutils/header.h`, `MAX_MPC_SERVERS` in `mpc.h` | `src/Mpc.Core/MpcConstants.cs` |
| `resolveSRV()` in `lib/srvquery.c` | `src/Mpc.Core/DnsSrvResolver.cs`, `DnsMessage.cs` (DNS over UDP, TCP fallback) |
| `getSRVRecords()`, `getDomainFromEmailAddress()` in `send.c` | `src/Mpc.Core/MpcServerDiscovery.cs`, `EmailAddress.cs` |
| `chunkifyEMailBody()` in `send.c` | Replaced by secret sharing: `src/Mpc.Core/PrimeField.cs`, `AdditiveSharing.cs`, `SharePayload.cs`, `SharedMessage.cs`. `BodyChunker.cs` is kept for mailutils compatibility tests. |
| Header setting loop in `mpc_mail_compose_send__()` | `src/Mpc.Core/MpcSendPlanner.cs` (`PlanShares`) + `src/MpcOutlookAddIn/OutlookMpcSender.cs` |
| `_mpc_get_chunk_info()` in `mail.c` | `MpcCoalescer.TryReadChunk` / `Group` in `src/Mpc.Core/MpcCoalescer.cs` |
| `maildir_mpc_coaelesce_mails_unlocked()` in `mail.c` | `src/MpcOutlookAddIn/OutlookMpcReceiver.cs` (+ `SharedMessage.Combine` for shares) |

`src/Mpc.Core` has no Outlook dependencies. The add-in compiles those files in directly, so it
ships as a single assembly.

## Requirements

- Windows with classic Outlook (2013, 2016, 2019, 2021, 2024 or Microsoft 365 desktop). The new
  Outlook for Windows doesn't run VSTO add-ins.
- Visual Studio 2022 with the **Office/SharePoint development** workload and the .NET Framework 4.8
  targeting pack.
- The Visual Studio 2010 Tools for Office Runtime, which Office installs and the ClickOnce installer
  adds if it's missing.

## Build and debug

1. Open `MpcOutlook.sln` in Visual Studio.
2. Set **MpcOutlookAddIn** as the startup project and press **F5**. Visual Studio registers the add-in
   and starts Outlook with it loaded.
3. Run the core tests with `dotnet run --project tests/Mpc.Core.Tests` (they need only the .NET SDK;
   with only .NET 9 or later installed, add `DOTNET_ROLL_FORWARD=Major` in front).

The ClickOnce manifests are signed with `MpcOutlookAddIn_TemporaryKey.pfx`, a self-signed test
certificate with no password, like the one Visual Studio creates for a new VSTO project. Before
giving the add-in to other people, replace it with your own code-signing certificate
(**Project → Properties → Signing**).

## See the secret sharing work, without Outlook (any OS, including macOS)

`tools/MpcDemo` runs the add-in's own sharing code on a real email and writes it out as `.eml`
files you can open in Apple Mail, Outlook or Thunderbird (or preview with Space in Finder):

```bash
dotnet run --project tools/MpcDemo                        # sample email: budget.csv, notes.txt, smiley.png
dotnet run --project tools/MpcDemo -- --servers 3 ~/Documents/report.pdf photo.jpg
dotnet run --project tools/MpcDemo -- --help
```

It writes `demo-output/`:

| Folder | Contents |
|---|---|
| `1-original/` | `original.eml`, the email as you'd write it, and its attachments |
| `2-shares/` | `share-1.eml`, `share-2.eml`, ...: what each MPC server relays, with the `X-MPC-*` headers, a placeholder body and the `.bin` share files; the raw `.bin` files are also in `share-N-files/` |
| `3-rebuilt/` | `rebuilt.eml` and its attachments, rebuilt by reading the share emails back from disk (in reverse order) and combining them with `MpcCoalescer` and `SharedMessage.Combine` |

The console shows each step: the share headers, the first bytes of every share file, a check that no
share contains readable text from the original, the arithmetic for one number (share 1 + share 2 =
secret, mod p), SHA-256 checks that every rebuilt file is identical, and what happens if one server
tries to rebuild the message from its share alone. It exits with code 0 only if everything matches.

## Install on other machines

[`utils/outlook-mpc-addin-installer/`](../utils/outlook-mpc-addin-installer) builds a single setup
program, `MpcOutlookAddIn-Setup-<version>.exe`, that you can copy
to any Windows PC with classic Outlook. It uses [Inno Setup](https://jrsoftware.org/isinfo.php).

### Build the installer

On the machine where you build the add-in:

1. Install Inno Setup 6 once: `winget install JRSoftware.InnoSetup`.
2. In PowerShell, from `utils\outlook-mpc-addin-installer`, run:

   ```powershell
   powershell -ExecutionPolicy Bypass -File .\build-installer.ps1
   ```

   It builds the add-in in Release with Visual Studio's MSBuild, then writes
   `utils\outlook-mpc-addin-installer\Output\MpcOutlookAddIn-Setup-<version>.exe`. The version comes from
   `AssemblyFileVersion` in `Properties\AssemblyInfo.cs`; raise it for each release so upgrades
   are easy to tell apart. `-SkipBuild` packages the existing `bin\Release` output, and
   `-Iscc <path>` points at `ISCC.exe` if Inno Setup is somewhere unusual.

### What the setup does

- Asks whether to install **just for me** (the default; no admin rights) or **for all users**
  (admin). From the command line, `/CURRENTUSER` or `/ALLUSERS` choose, and `/VERYSILENT` installs
  without any windows.
- Checks for the .NET Framework 4.8, and for the Visual Studio 2010 Tools for Office Runtime. If the
  runtime is missing, it offers to open its
  [download page](https://www.microsoft.com/download/details.aspx?id=105522).
- Copies the add-in, its two signed manifests and the VSTO utility DLLs to
  `%LOCALAPPDATA%\Programs\MpcOutlookAddIn` (just me) or `Program Files\MpcOutlookAddIn` (all users).
- Registers the add-in under `Software\Microsoft\Office\Outlook\Addins\MpcOutlookAddIn` with
  `LoadBehavior=3`, in HKCU (just me) or in both the 32-bit and 64-bit HKLM views (all users), so it
  works with either bitness of Office.
- Adds the add-in to Outlook 2013/2016+'s `DoNotDisableAddinList`, so Outlook doesn't turn it off
  for starting slowly.
- Trust: the manifests are signed with the test certificate, which Windows doesn't know. For a
  just-me install, `vsto-trust.ps1` adds an entry to the user's VSTO inclusion list so Outlook
  doesn't ask whether to install the add-in. Add-ins installed to Program Files are trusted
  already. If Outlook still asks once, answer **Install**. The entry is written to
  `%LOCALAPPDATA%\MpcOutlookAddIn\install.log`.
- If Outlook has the add-in open during an upgrade, setup offers to close Outlook. Start Outlook
  again after installing to load the add-in.

The uninstaller (**Settings → Apps → MPC Outlook Add-in**) removes the files, the registration and
the inclusion list entry. It leaves `settings.ini` and `mpc.log` in place.

Notes:

- The setup .exe itself isn't code-signed, so Windows SmartScreen may say "Windows protected your
  PC". Choose **More info → Run anyway**.
- On the machine where you debug with Visual Studio, F5 registers the add-in from `bin\Debug` under
  the same HKCU key, and a VS **Clean** removes that key. A just-me install and VS debugging
  overwrite each other's registration. Rerun whichever one you want Outlook to use.
- Outlook 2013 works with the installer as-is. For F5 debugging against Outlook 2013, change
  `DebugInfoExeName` in the .csproj from `Office\16.0` to `Office\15.0`.

**Build → Publish MpcOutlookAddIn** in Visual Studio still works too. It produces a ClickOnce
`setup.exe` plus a folder of files, not a single installer.

## Using it

**Main window: Home tab → MPC group**

- **Rejoin MPC Chunks**: scans the current folder and rebuilds every complete set of shares (or
  mailutils chunks). It reports any sets that are still missing pieces.
- **Split on Send** (on by default): when the first To recipient's domain publishes MPC SRV records,
  every message to it is secret-shared automatically. If the domain has no records, the message goes
  out unchanged, as with `mail`.
- **Rejoin New Mail** (on by default): checks the Inbox of **every account** in the profile at
  startup and a few seconds after new mail arrives in any of them (each Inbox is watched, since
  Outlook's new-mail event isn't raised reliably for IMAP accounts). `mail` does this when it
  opens the mailbox. Accounts added while Outlook is running are watched after a restart.
- **MPC Settings**: opens the settings file.

**New message window: Message tab → Send via MPC**: secret-shares and sends this message even when
**Split on Send** is off, or explains why it can't (for example, no SRV records).

What happens when a message is sent via MPC:

1. The add-in saves your message (Outlook only copies the editor's contents into the message when
   it is saved), then takes the body as **plain text** (formatting is dropped) and reads every
   attachment.
2. The body and each attachment are split into N additive shares, one per MPC server (see
   [Secret sharing](#secret-sharing)).
3. Each share is sent as a **copy** of your saved message, with the same recipients, subject and
   sending account. (The message open in the editor isn't changed itself, because Outlook would
   overwrite those changes with the editor's contents when sending.)
4. Every email-share carries **its share of the body and of every attachment** as `.bin` files, and
   a short placeholder body that says what the message is. The original attachments are removed;
   none of the text or attachments goes out in the clear. The **subject is not shared** and is
   sent as it is on every share.
5. Your original message isn't sent. Its window closes and its draft moves to **Deleted Items**.
   **Sent Items** shows the shares.
6. If other recipients are at a different domain, the add-in asks first. Every share goes to every
   recipient, routed by the first To recipient's MPC servers, as in `send.c`.

What happens when shares are combined: the `.bin` files of every share are read and added up modulo
p. If they don't add up to valid data (shares of different messages, or a damaged file), nothing is
changed, the pieces stay where they are and the error goes to the log. Otherwise share 1 is copied,
and the copy gets the rebuilt body and the original attachments with their names,
`X-MPC-Chunk: [000/000]`, `X-MPC-Via: <via1>, <via2>`, and is unread if any share was. The shares
are then moved to **Deleted Items** (you can turn this off in the settings).

Chunks from mailutils (no `X-MPC-Scheme` header) are rejoined as before: the body slices are joined
in order and the attachments copied from the other chunks.

**Sender checks (stage 1 of malware detection).** Before rejoining, the add-in checks that every
chunk passed SPF, DKIM and DMARC at the MPC relay it entered through: the relay named in its
`X-MPC-Via`, which records its results in `Authentication-Results` headers named after itself (see
the lab's [Sender checks](../lab/aws-opentofu/README.md#sender-checks-stage-1)). For every chunk,
the results from that relay must say `spf=pass`, `dkim=pass` for the From: domain and
`dmarc=pass`, with no other SPF, DKIM or DMARC result (a `pass` next to a `fail` fails); its
From: address must be its envelope sender (`Return-Path`), since the sender's server ties the
envelope sender to the logged-in user but signs whatever From: they write; and all chunks must
have the same From: address. Results from any other server are ignored, and a chunk
without results from its relay fails. A message that fails is not rejoined: its chunks get the
category **"MPC: failed sender checks"** and are moved to the account's **Junk Email** folder (kept,
not deleted, like a mail server's quarantine; if the account has no Junk folder they stay where
they are), and **Rejoin MPC Chunks** lists it once under "failed the sender checks" with the reason
(the full list is in the log). Chunks with that category are skipped from then on, so the same
message isn't reported on every rejoin. To check a message again (for example after turning
`RequireSenderChecks` off), move its chunks back to the Inbox and remove the category. Outlook may
re-save a moved IMAP message without some of its original headers, so the reason is best read from
the report or the log.

Every copy of a chunk is checked, since two emails can carry the same X-MPC-ID and chunk number (a
genuine chunk resent from another server, or a forged one). If one copy passes, it is used, so a
forged copy can't stop the message. A duplicate that fails, or comes from a different From:
address, is **not** deleted with the rest: it is moved to Junk Email with the category
"MPC: failed sender checks", and the report lists it with the reason. Identical duplicates that
pass are removed with the other chunks, as before. Turn this off with `RequireSenderChecks=false`, for mail servers
that don't add these results (and remove the category from chunks that already failed).

### Settings

`%APPDATA%\MpcOutlookAddIn\settings.ini` (restart Outlook after editing it):

```ini
AutoSplitOnSend=true
AutoRejoinNewMail=true
DeleteChunksAfterRejoin=true
# Only scan mail received in the last N days (0 = whole folder)
RejoinLookbackDays=30
# Comma-separated DNS servers for the SRV lookups; empty uses this computer's DNS
DnsServers=
DnsTimeoutMs=2000
# Rejoin only messages whose chunks all passed the sender checks (SPF, DKIM, DMARC at their MPC relay)
RequireSenderChecks=true
```

The log is at `%LOCALAPPDATA%\MpcOutlookAddIn\mpc.log`.

## Secret sharing

**Field.** All arithmetic is in Z_p with p = 2^61 − 1 (a Mersenne prime), in `PrimeField.cs`.

**Packing.** Data is cut into 7-byte groups, each read as a little-endian number below 2^56 (and so
below p); the last group is padded with zeros. A body or attachment of L bytes becomes ⌈L/7⌉
field elements.

**Sharing.** For each element x and N servers, shares 1 to N−1 are uniformly random field elements
from the system's cryptographic random number generator, and share N is x minus their sum, mod p.
Any N−1 shares are uniformly random whatever x is; adding all N gives x back. Random values are
taken as the low 61 bits of 8 random bytes, and the one value that equals p is drawn again, so every
share is exactly uniform. With a single SRV record there is only one share, which equals the data,
so the server sees everything; publish at least two records.

**What is shared.** The body as UTF-8 text, and each attachment as
`[2-byte name length][file name, UTF-8][file content]`, so attachment names are hidden too. Each is
shared separately, with fresh randomness.

**Share files.** Each email-share has `smilies-body.bin` and one `smilies-att-<k>.bin` per attachment
(k = 1, 2, …). Each file (all integers little-endian):

| Offset | Size | Field |
|---|---|---|
| 0 | 4 | `SMLS` |
| 4 | 1 | Format version, `1` |
| 5 | 1 | Kind: `1` body, `2` attachment |
| 6 | 1 | Field: `1` = Z_p, p = 2^61 − 1 |
| 7 | 1 | Form: `0` = full share |
| 8 | 2 | Attachment number k (0 for the body) |
| 10 | 8 | Length L of the shared data in bytes |
| 18 | 8 × ⌈L/7⌉ | The share: one 8-byte element per 7 bytes of data |

The first 18 bytes follow the SMILIES Thunderbird spec, so both clients can share one format.

**What the servers still see.** Sender, recipients, subject, date, the number of attachments and the
size of each shared item (to within 7 bytes). The protection is against each MPC relay on its own:
every share is still addressed to the recipient, so the server that finally stores the recipient's
mailbox receives all N shares, as do relays that collude. Each email-share is about 8/7 of the original size,
so N servers carry about N times the original traffic.

**Checks when combining.** Every share must hold the body and the same set of attachments, with
matching kinds and lengths; every element must be below p; and the rebuilt numbers must fit in 7
bytes with zero padding. Shares of a different message almost never pass that last check, because a
wrong total is a random field element. Received attachment names are made safe before being saved
(no path separators, no `..`).

## DNS and mail routing

A recipient domain enables MPC by publishing SRV records:

```
_mpc1._tcp.example.com. 3600 IN SRV 10 0 25 relay1.example.net.
_mpc2._tcp.example.com. 3600 IN SRV 10 0 25 relay2.example.org.
```

`X-MPC-Via` is only a header. A chunk actually travels through that relay because of the Postfix
rule in `postfix-3.6.4/etc/postfix/header_checks` (`/^X-MPC-Via: .../ FILTER smtp:$2`). For that to
apply to mail from Outlook, **the sending account must submit mail through that Postfix server**:

- **POP/IMAP account in Outlook**: set its outgoing (SMTP) server to your Postfix host.
- **Exchange or Microsoft 365 account**: Exchange routes by MX and ignores `X-MPC-Via`. Add an
  outbound connector, plus a mail flow rule matching messages with an `X-MPC-Via` header, that relays
  them through the Postfix host.

## Where the add-in differs from the C code

These are deliberate. Most avoid failure cases in the current C code:

- **Out-of-order chunks.** In `_mpc_get_chunk_info()`, if a group's head is chunk 2 when chunk 1
  arrives, the insertion loop exits with `mpc_msg_prev == NULL` and dereferences it. The add-in sorts
  chunks instead.
- **Incomplete sets.** `mail.c` stores `max_chunks` but never checks it, so it joins whatever chunks
  are present and marks the result `[000/000]`. Chunks that arrive later then can't be added. The
  add-in waits until every chunk has arrived.
- **No SRV records.** When `srv_rec_cnt` is 0, `mpc_chunks_count` stays 0, but the `do … while` send
  loop still runs once. `env->mpc_chunks[0]` is NULL (because `compose_init` zeroes `env`), and
  `mu_stream_seek(NULL, …)` dereferences it, so `mail` crashes instead of sending normally. The
  add-in sends the message unchanged.
- **Empty body.** For a zero-byte body, `chunkifyEMailBody()` returns 0 chunks, which leads to the
  same NULL stream. The add-in always sends one chunk per server, some possibly empty, because the
  receiver needs all of them.
- **Recipient domain.** `getDomainFromEmailAddress()` takes everything after the first `@` of the raw
  To header, so `Name <a@b.com>` gives `b.com>`, and `a@x.com, c@y.com` gives `x.com, c@y.com`. The
  add-in uses the first To recipient's SMTP address.
- **Multi-byte characters.** The C code splits at exact byte offsets, which can cut a UTF-8 character
  in two. The add-in moves the boundary back to the start of the character. Chunk sizes are identical
  for ASCII text, and the tests check this against the C formula.
- **Duplicates.** A chunk delivered twice is rejoined once, and the extra copy is deleted with the
  other chunks.
- **Secret sharing instead of splitting.** The C code sends each server a readable slice of the body
  and sends attachments in the clear on the last chunk. The add-in sends each server an additive
  share of the whole body and of every attachment, marked with `X-MPC-Scheme: additive-p61`.

## Testing status

- `tests/Mpc.Core.Tests`: 67 tests covering header formats, chunk sizes against the C formula,
  UTF-8 boundaries, header parsing and rewriting, grouping (out of order, incomplete, duplicates),
  SRV lookups against an in-process DNS server over UDP and TCP (including NXDOMAIN, SERVFAIL and
  timeouts), and secret sharing: field arithmetic, packing, sharing and reconstruction for 1 to 5
  servers, redrawing random values equal to p, the share-file format and its validation, rejecting
  shares that don't belong together, safe file names, and a full send-to-combine round trip with
  attachments; and the sender checks: parsing `Authentication-Results` (using the exact headers
  OpenDKIM and OpenDMARC add in the lab's configuration), and passing or failing a message
  (forged chunk, a forged pass next to the relay's real fail, a replayed chunk, results from the
  wrong relay, SPF without DKIM, a DKIM signature or DMARC result for another domain, different
  senders, missing results, a From: that isn't the envelope sender), and duplicates of a chunk (a replayed copy is kept and reported, a
  passing copy is used even when a forged one comes first, identical copies are extras).
- The add-in code compiles against the .NET Framework 4.8 reference assemblies at C# 7.3.

It **has not yet been run inside Outlook**. On first use, check these:

1. Shares arrive with all five `X-MPC-*` headers (**File → Properties → Internet headers**),
   especially when sending from an Exchange account.
2. Each share has `smilies-body.bin` and one `smilies-att-<k>.bin` per attachment, and no original
   attachment.
3. Combining a message with several attachments, including a large one, gives back the exact text
   and identical files.
4. Rejoining a two-chunk message from `mail` gives exactly the original text, with no extra line
   break where the chunks meet.
5. The rebuilt message appears in the same folder, with the original sender and date.
6. With the lab's sender checks: every chunk shows three `Authentication-Results` lines from its
   entry relay, and the message is rejoined; with `RequireSenderChecks=false` on a lab built
   before them, messages still rejoin.
