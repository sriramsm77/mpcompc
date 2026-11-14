locals {
  sender_public_ip    = aws_eip.lab["mta-sender"].public_ip
  recipient_public_ip = aws_eip.lab["mx-recipient"].public_ip
}

output "servers" {
  description = "Instance IDs and addresses of the lab servers."
  value = {
    for k, i in aws_instance.lab : k => {
      instance_id = i.id
      fqdn        = local.hosts[k].fqdn
      private_ip  = i.private_ip
      public_ip   = contains(keys(aws_eip.lab), k) ? aws_eip.lab[k].public_ip : i.public_ip
    }
  }
}

output "outlook_sender_account" {
  description = "Outlook account (manual IMAP setup) for the sender. Password: tofu output -raw alice_password"
  value = {
    email_address          = "alice@${local.hosts["mta-sender"].domain}"
    user_name              = "alice"
    incoming_imap_server   = local.sender_public_ip
    incoming_imap_port     = "993 (SSL/TLS)"
    outgoing_smtp_server   = local.sender_public_ip
    outgoing_smtp_port     = "587 (STARTTLS)"
    outgoing_requires_auth = "yes, same user name and password"
  }
}

output "outlook_recipient_account" {
  description = "Outlook account (manual IMAP setup) for the recipient. Password: tofu output -raw bob_password"
  value = {
    email_address          = "bob@${local.hosts["mx-recipient"].domain}"
    user_name              = "bob"
    incoming_imap_server   = local.recipient_public_ip
    incoming_imap_port     = "993 (SSL/TLS)"
    outgoing_smtp_server   = local.recipient_public_ip
    outgoing_smtp_port     = "587 (STARTTLS)"
    outgoing_requires_auth = "yes, same user name and password"
  }
}

output "addin_settings_ini" {
  description = "Put this line in %APPDATA%\\MpcOutlookAddIn\\settings.ini on the sending laptop, then restart Outlook."
  value       = "DnsServers=${local.sender_public_ip}"
}

output "check_srv_records" {
  description = "Run on your laptop to confirm it can see the MPC SRV records for both mail domains."
  value = join("; ", [
    for d in sort(keys(local.mail_domains)) : "nslookup -type=SRV _mpc1._tcp.${d} ${local.sender_public_ip}"
  ])
}

output "mpc_routes" {
  description = "Path each chunk takes to each mail domain."
  value = {
    for pair in flatten([
      for d in sort(keys(local.mpc_srv)) : [
        for i, r in local.mpc_srv[d] : {
          key   = "to ${d}, chunk ${i + 1}"
          route = "${local.hosts[r].fqdn} -> ${local.hosts[local.relay_partner[r]].fqdn} -> ${local.mail_domains[d]}"
        }
      ]
    ]) : pair.key => pair.route
  }
}

output "alice_password" {
  description = "Password for alice (green.mpc)."
  value       = random_password.mail["alice"].result
  sensitive   = true
}

output "bob_password" {
  description = "Password for bob (blue.mpc)."
  value       = random_password.mail["bob"].result
  sensitive   = true
}
