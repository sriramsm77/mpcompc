variable "region" {
  description = "AWS region for the lab."
  type        = string
  default     = "ap-south-1" # Mumbai
}

variable "admin_cidr" {
  description = <<-EOT
    Your laptop's public IP address as a /32, e.g. "203.0.113.25/32". Only this address can
    reach the lab's IMAP (993), SMTP submission (587), DNS (53) and SSH (22) ports.
    Find it with: curl https://checkip.amazonaws.com
  EOT
  type        = string

  validation {
    condition     = can(cidrhost(var.admin_cidr, 0)) && var.admin_cidr != "0.0.0.0/0"
    error_message = "admin_cidr must be a CIDR block such as \"203.0.113.25/32\", and not 0.0.0.0/0."
  }
}

variable "instance_type" {
  description = "EC2 instance type for all four servers. t3.micro and t4g.micro are free-plan eligible."
  type        = string
  default     = "t3.micro"
}

variable "vpc_cidr" {
  description = "Address range of the lab VPC. Each server gets its own /24 inside it."
  type        = string
  default     = "10.20.0.0/16"
}

variable "ssh_public_key" {
  description = <<-EOT
    Optional SSH public key (contents of ~/.ssh/id_ed25519.pub). Leave empty to use AWS Systems
    Manager Session Manager only, which needs no open SSH port.
  EOT
  type        = string
  default     = ""
}

variable "name_prefix" {
  description = "Prefix for resource names."
  type        = string
  default     = "mpc-lab"
}
