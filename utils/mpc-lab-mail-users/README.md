# Mail users for the MPC lab

Adds mail accounts to the AWS lab's mail servers (`mail.green.mpc`, `mail.blue.mpc`), which
[`lab/aws-opentofu`](../../lab/aws-opentofu) creates.

| File | Runs on | What it does |
| --- | --- | --- |
| `New-LabMailUser.ps1` | Your Windows PC (needs the AWS CLI) | Finds the domain's mail server and runs `add-mail-user.sh` there through AWS Systems Manager, then prints the Outlook settings. |
| `add-mail-user.sh` | The mail server, as root | Creates the account (a Linux user with no login shell and a Maildir, like alice and bob) or resets its password. |

```powershell
cd utils\mpc-lab-mail-users
.\New-LabMailUser.ps1 -Domain blue.mpc  -UserName carol -Region ap-southeast-2
.\New-LabMailUser.ps1 -Domain green.mpc -UserName dave  -Password 'Lab-Pass-2026' -Region ap-southeast-2
.\New-LabMailUser.ps1 -Domain blue.mpc  -UserName carol -ResetPassword -Region ap-southeast-2
```

Keep both files in the same folder. See "Add more mail users" in the
[lab README](../../lab/aws-opentofu/README.md#add-more-mail-users) for details.
