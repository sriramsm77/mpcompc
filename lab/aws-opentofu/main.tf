# MPCoMPC test lab: one VPC, four Ubuntu 24.04 servers.
#
#   mta-sender   mail.sender.test         lab DNS (dnsmasq) + Postfix with the X-MPC-Via
#                                         header_checks rule + Dovecot IMAP for alice
#   relay-a      relay-a.provider-a.test  Postfix relay ("service provider A")
#   relay-b      relay-b.provider-b.test  Postfix relay ("service provider B")
#   mx-recipient mx.recipient.test        Postfix MX + Dovecot IMAP for bob
#
# Mail between the servers stays on private addresses inside the VPC.

locals {
  hosts = {
    "mta-sender" = {
      subnet     = 1
      role       = "sender"
      fqdn       = "mail.sender.test"
      domain     = "sender.test"
      mail_user  = "alice"
      elastic_ip = true
    }
    "relay-a" = {
      subnet     = 2
      role       = "relay"
      fqdn       = "relay-a.provider-a.test"
      domain     = "provider-a.test"
      mail_user  = ""
      elastic_ip = false
    }
    "relay-b" = {
      subnet     = 3
      role       = "relay"
      fqdn       = "relay-b.provider-b.test"
      domain     = "provider-b.test"
      mail_user  = ""
      elastic_ip = false
    }
    "mx-recipient" = {
      subnet     = 4
      role       = "recipient"
      fqdn       = "mx.recipient.test"
      domain     = "recipient.test"
      mail_user  = "bob"
      elastic_ip = true
    }
  }

  subnet_cidr  = { for k, h in local.hosts : k => cidrsubnet(var.vpc_cidr, 8, h.subnet) }
  private_ip   = { for k, h in local.hosts : k => cidrhost(local.subnet_cidr[k], 10) }
  vpc_resolver = cidrhost(var.vpc_cidr, 2) # AmazonProvidedDNS
  lab_dns_ip   = local.private_ip["mta-sender"]

  az   = sort(data.aws_ec2_instance_type_offerings.this.locations)[0]
  arch = contains(data.aws_ec2_instance_type.this.supported_architectures, "arm64") ? "arm64" : "amd64"

  mail_passwords = {
    alice = random_password.mail["alice"].result
    bob   = random_password.mail["bob"].result
  }
}

# ---------------------------------------------------------------------------------------------
# Lookups
# ---------------------------------------------------------------------------------------------

data "aws_ec2_instance_type" "this" {
  instance_type = var.instance_type
}

# An availability zone that offers the chosen instance type.
data "aws_ec2_instance_type_offerings" "this" {
  location_type = "availability-zone"

  filter {
    name   = "instance-type"
    values = [var.instance_type]
  }
}

data "aws_ami" "ubuntu" {
  most_recent = true
  owners      = ["099720109477"] # Canonical

  filter {
    name   = "name"
    values = ["ubuntu/images/hvm-ssd-gp3/ubuntu-noble-24.04-${local.arch}-server-*"]
  }

  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
}

resource "random_password" "mail" {
  for_each = toset(["alice", "bob"])
  length   = 20
  special  = false
}

# ---------------------------------------------------------------------------------------------
# Network
# ---------------------------------------------------------------------------------------------

resource "aws_vpc" "lab" {
  cidr_block           = var.vpc_cidr
  enable_dns_support   = true
  enable_dns_hostnames = true

  tags = { Name = "${var.name_prefix}-vpc" }
}

resource "aws_internet_gateway" "lab" {
  vpc_id = aws_vpc.lab.id
  tags   = { Name = "${var.name_prefix}-igw" }
}

# One subnet per server, so each "provider" sits in its own network segment. All subnets are
# public so the servers can install packages without a (paid) NAT gateway; the security groups
# below decide what can reach them.
resource "aws_subnet" "lab" {
  for_each = local.hosts

  vpc_id                  = aws_vpc.lab.id
  cidr_block              = local.subnet_cidr[each.key]
  availability_zone       = local.az
  map_public_ip_on_launch = true

  tags = { Name = "${var.name_prefix}-${each.key}" }
}

resource "aws_route_table" "public" {
  vpc_id = aws_vpc.lab.id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.lab.id
  }

  tags = { Name = "${var.name_prefix}-public" }
}

resource "aws_route_table_association" "lab" {
  for_each       = aws_subnet.lab
  subnet_id      = each.value.id
  route_table_id = aws_route_table.public.id
}

# ---------------------------------------------------------------------------------------------
# Security groups
# ---------------------------------------------------------------------------------------------

# Servers talk to each other freely (SMTP 25, DNS 53) on private addresses.
resource "aws_security_group" "internal" {
  name        = "${var.name_prefix}-internal"
  description = "Traffic between lab servers, and all outbound traffic"
  vpc_id      = aws_vpc.lab.id

  ingress {
    description = "Anything from inside the lab VPC"
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = [var.vpc_cidr]
  }

  egress {
    description = "All outbound"
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = { Name = "${var.name_prefix}-internal" }
}

# What Outlook on your laptop needs: IMAP and authenticated SMTP submission.
resource "aws_security_group" "mail_client" {
  name        = "${var.name_prefix}-mail-client"
  description = "IMAPS and SMTP submission from the admin laptop"
  vpc_id      = aws_vpc.lab.id

  ingress {
    description = "IMAP over TLS"
    from_port   = 993
    to_port     = 993
    protocol    = "tcp"
    cidr_blocks = [var.admin_cidr]
  }

  ingress {
    description = "SMTP submission (STARTTLS)"
    from_port   = 587
    to_port     = 587
    protocol    = "tcp"
    cidr_blocks = [var.admin_cidr]
  }

  tags = { Name = "${var.name_prefix}-mail-client" }
}

# The Outlook add-in on your laptop looks up the _mpcN._tcp SRV records on the lab DNS server.
resource "aws_security_group" "lab_dns" {
  name        = "${var.name_prefix}-lab-dns"
  description = "Lab DNS (SRV/MX/A records for the .test domains) from the admin laptop"
  vpc_id      = aws_vpc.lab.id

  ingress {
    description = "DNS over UDP"
    from_port   = 53
    to_port     = 53
    protocol    = "udp"
    cidr_blocks = [var.admin_cidr]
  }

  ingress {
    description = "DNS over TCP"
    from_port   = 53
    to_port     = 53
    protocol    = "tcp"
    cidr_blocks = [var.admin_cidr]
  }

  tags = { Name = "${var.name_prefix}-lab-dns" }
}

resource "aws_security_group" "ssh" {
  count = var.ssh_public_key == "" ? 0 : 1

  name        = "${var.name_prefix}-ssh"
  description = "SSH from the admin laptop"
  vpc_id      = aws_vpc.lab.id

  ingress {
    description = "SSH"
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = [var.admin_cidr]
  }

  tags = { Name = "${var.name_prefix}-ssh" }
}

# ---------------------------------------------------------------------------------------------
# Shell access through AWS Systems Manager Session Manager (no SSH key or open port needed)
# ---------------------------------------------------------------------------------------------

resource "aws_iam_role" "instance" {
  name = "${var.name_prefix}-instance"

  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Effect    = "Allow"
      Principal = { Service = "ec2.amazonaws.com" }
      Action    = "sts:AssumeRole"
    }]
  })
}

resource "aws_iam_role_policy_attachment" "ssm" {
  role       = aws_iam_role.instance.name
  policy_arn = "arn:aws:iam::aws:policy/AmazonSSMManagedInstanceCore"
}

resource "aws_iam_instance_profile" "instance" {
  name = "${var.name_prefix}-instance"
  role = aws_iam_role.instance.name
}

resource "aws_key_pair" "admin" {
  count      = var.ssh_public_key == "" ? 0 : 1
  key_name   = "${var.name_prefix}-admin"
  public_key = var.ssh_public_key
}

# ---------------------------------------------------------------------------------------------
# Servers
# ---------------------------------------------------------------------------------------------

resource "aws_instance" "lab" {
  for_each = local.hosts

  ami                  = data.aws_ami.ubuntu.id
  instance_type        = var.instance_type
  subnet_id            = aws_subnet.lab[each.key].id
  private_ip           = local.private_ip[each.key]
  iam_instance_profile = aws_iam_instance_profile.instance.name
  key_name             = var.ssh_public_key == "" ? null : aws_key_pair.admin[0].key_name

  vpc_security_group_ids = concat(
    [aws_security_group.internal.id],
    each.value.role == "relay" ? [] : [aws_security_group.mail_client.id],
    each.value.role == "sender" ? [aws_security_group.lab_dns.id] : [],
    var.ssh_public_key == "" ? [] : [aws_security_group.ssh[0].id],
  )

  # Installs and configures this server's role on first boot (log: /var/log/cloud-init-output.log).
  user_data = templatefile("${path.module}/templates/bootstrap.sh.tftpl", {
    role          = each.value.role
    fqdn          = each.value.fqdn
    domain        = each.value.domain
    mail_user     = each.value.mail_user
    mail_password = each.value.mail_user == "" ? "" : local.mail_passwords[each.value.mail_user]
    vpc_cidr      = var.vpc_cidr
    vpc_resolver  = local.vpc_resolver
    lab_dns_ip    = local.lab_dns_ip
    ip_sender     = local.private_ip["mta-sender"]
    ip_relay_a    = local.private_ip["relay-a"]
    ip_relay_b    = local.private_ip["relay-b"]
    ip_recipient  = local.private_ip["mx-recipient"]
  })
  user_data_replace_on_change = true

  root_block_device {
    volume_size = 8
    volume_type = "gp3"
    encrypted   = true
  }

  metadata_options {
    http_tokens = "required" # IMDSv2 only
  }

  # t3 instances otherwise default to "unlimited" CPU credits, which can be billed.
  credit_specification {
    cpu_credits = "standard"
  }

  tags = { Name = "${var.name_prefix}-${each.key}" }

  depends_on = [aws_route_table_association.lab, aws_iam_role_policy_attachment.ssm]
}

# Fixed public addresses for the two servers Outlook connects to, so they survive a stop/start.
resource "aws_eip" "lab" {
  for_each = { for k, h in local.hosts : k => h if h.elastic_ip }

  domain   = "vpc"
  instance = aws_instance.lab[each.key].id

  tags = { Name = "${var.name_prefix}-${each.key}" }

  depends_on = [aws_internet_gateway.lab]
}
