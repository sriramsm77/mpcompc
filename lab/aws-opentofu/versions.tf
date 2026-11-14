# OpenTofu project (the "terraform" block name is what OpenTofu reads too).
terraform {
  required_version = ">= 1.6.0" # OpenTofu 1.6 or later

  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = ">= 5.0, < 7.0"
    }
    random = {
      source  = "hashicorp/random"
      version = ">= 3.5"
    }
  }
}

provider "aws" {
  region = var.region

  default_tags {
    tags = {
      Project   = "mpcompc-lab"
      ManagedBy = "opentofu"
    }
  }
}
