# MPC Outlook Add-in

A VSTO add-in for **classic Outlook on Windows** that adds the X-MPC logic from this repository's
mailutils changes to Outlook:

- **Sending** (port of `mpc_mail_compose_send__` in `mail/send.c`): looks up `_mpc1._tcp.<domain>` and
  `_mpc2._tcp.<domain>` for the first To recipient, splits the message into one chunk per MPC
  server, and sends each chunk with `X-MPC-Via`, `X-MPC-ID`, `X-MPC-Chunk` and `X-MPC-Result`.
- **Receiving** (port of `maildir_mpc_coaelesce_mails_unlocked` in `mail/mail.c`): finds messages
  with `X-MPC-*` headers, groups them by `X-MPC-ID`, and once every chunk has arrived, joins them
  into one message marked `X-MPC-Chunk: [000/000]` with the combined `X-MPC-Via`.

Messages sent by the add-in can be rejoined by the mailutils `mail` client, and the other way round.

## How the C code maps to the add-in

| mailutils (C) | Add-in (C#) |
|---|---|
| `MU_HEADER_MPC_*` in `include/mailutils/header.h`, `MAX_MPC_SERVERS` in `mpc.h` | `src/Mpc.Core/MpcConstants.cs` |
| `resolveSRV()` in `lib/srvquery.c` | `src/Mpc.Core/DnsSrvResolver.cs`, `DnsMessage.cs` (DNS over UDP, TCP fallback) |
| `getSRVRecords()`, `getDomainFromEmailAddress()` in `send.c` | `src/Mpc.Core/MpcServerDiscovery.cs`, `EmailAddress.cs` |
| `chunkifyEMailBody()` in `send.c` | `src/Mpc.Core/BodyChunker.cs` |
| Header setting loop in `mpc_mail_compose_send__()` | `src/Mpc.Core/MpcSendPlanner.cs` + `src/MpcOutlookAddIn/OutlookMpcSender.cs` |
| `_mpc_get_chunk_info()` in `mail.c` | `MpcCoalescer.TryReadChunk` / `Group` in `src/Mpc.Core/MpcCoalescer.cs` |
| `maildir_mpc_coaelesce_mails_unlocked()` in `mail.c` | `src/MpcOutlookAddIn/OutlookMpcReceiver.cs` |

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
3. Run the core tests with `dotnet run --project tests/Mpc.Core.Tests` (they need only the .NET SDK).

The ClickOnce manifests are signed with `MpcOutlookAddIn_TemporaryKey.pfx`, a self-signed test
certificate with no password, like the one Visual Studio creates for a new VSTO project. Before
giving the add-in to other people, replace it with your own code-signing certificate
(**Project → Properties → Signing**).

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

- **Rejoin MPC Chunks**: scans the current folder and joins every complete set of chunks. It reports
  any sets that are still missing chunks.
- **Split on Send** (on by default): when the first To recipient's domain publishes MPC SRV records,
  every message to it is split automatically. If the domain has no records, the message goes out
  unchanged, as with `mail`.
- **Rejoin New Mail** (on by default): checks the Inbox at startup and a few seconds after new mail
  arrives. `mail` does this when it opens the mailbox.
- **MPC Settings**: opens the settings file.

**New message window: Message tab → Send via MPC**: splits and sends this message even when
**Split on Send** is off, or explains why it can't (for example, no SRV records).

What happens when a message is split:

1. The body is sent as **plain text**, because the chunks are byte ranges of the text, which is what
   mailutils sends and rejoins. Formatting is dropped.
2. The add-in saves your message, then sends each chunk as a **copy** of it, with the same
   recipients, subject and sending account. Only the last chunk keeps the attachments; `send.c`
   also attaches files only to the last chunk. (The message open in the editor isn't changed
   itself, because Outlook would overwrite those changes with the editor's contents when sending.)
3. Your original message isn't sent. Its window closes and its draft moves to **Deleted Items**.
   **Sent Items** shows the chunks.
4. If other recipients are at a different domain, the add-in asks first. Every chunk goes to every
   recipient, routed by the first To recipient's MPC servers, as in `send.c`.

What happens when chunks are rejoined: chunk 1 is copied, and the copy gets the joined body, the
other chunks' attachments, `X-MPC-Chunk: [000/000]`, `X-MPC-Via: <via1>, <via2>`, and is unread if
any chunk was. The chunks are then moved to **Deleted Items**. `mail.c` deletes them as well, and
you can turn this off in the settings.

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
```

The log is at `%LOCALAPPDATA%\MpcOutlookAddIn\mpc.log`.

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

## Testing status

- `tests/Mpc.Core.Tests`: 26 tests covering header formats, chunk sizes against the C formula,
  UTF-8 boundaries, header parsing and rewriting, grouping (out of order, incomplete, duplicates),
  a full send-to-rejoin round trip, and SRV lookups against an in-process DNS server over UDP and
  TCP (including NXDOMAIN, SERVFAIL and timeouts).
- The add-in code compiles against the .NET Framework 4.8 reference assemblies at C# 7.3.

It **has not yet been run inside Outlook**. On first use, check these:

1. Chunks arrive with all four `X-MPC-*` headers (**File → Properties → Internet headers**),
   especially when sending from an Exchange account.
2. Rejoining a two-chunk message from `mail` gives exactly the original text, with no extra line
   break where the chunks meet.
3. The rejoined message appears in the same folder, with the original sender and date.
