using System;
using System.Runtime.InteropServices;
using Outlook = Microsoft.Office.Interop.Outlook;

namespace MpcOutlookAddIn
{
    /// <summary>Reading and writing X-MPC-* headers and other MAPI properties on Outlook items.</summary>
    internal static class OutlookMpc
    {
        /// <summary>
        /// PS_INTERNET_HEADERS. A string property in this namespace becomes an Internet header
        /// of the same name when Outlook sends the message.
        /// </summary>
        public const string InternetHeaderNamespace =
            "http://schemas.microsoft.com/mapi/string/{00020386-0000-0000-C000-000000000046}/";

        /// <summary>PR_TRANSPORT_MESSAGE_HEADERS: the raw header block of a received message.</summary>
        public const string TransportHeadersProperty = "http://schemas.microsoft.com/mapi/proptag/0x007D001F";

        /// <summary>PR_SMTP_ADDRESS on an address entry.</summary>
        public const string SmtpAddressProperty = "http://schemas.microsoft.com/mapi/proptag/0x39FE001F";

        /// <summary>Private flag this add-in sets on a message it has rejoined.</summary>
        public const string RejoinedMarkerProperty =
            "http://schemas.microsoft.com/mapi/string/{6F1C3A52-7D4B-4E0B-9C1E-5A2B8D3F4E61}/MpcRejoined";

        public static void SetInternetHeader(Outlook.MailItem mail, string name, string value)
        {
            Outlook.PropertyAccessor accessor = mail.PropertyAccessor;
            try
            {
                accessor.SetProperty(InternetHeaderNamespace + name, value ?? string.Empty);
            }
            finally
            {
                Release(accessor);
            }
        }

        /// <summary>Value of an X- header property on an item, or null when it is not set.</summary>
        public static string GetInternetHeader(Outlook.MailItem mail, string name)
        {
            // Exchange stores received headers under lower-case names; Outlook keeps the
            // original case on items it sends.
            string value = GetStringProperty(mail, InternetHeaderNamespace + name);
            if (value == null) value = GetStringProperty(mail, InternetHeaderNamespace + name.ToLowerInvariant());
            return value;
        }

        public static string GetTransportHeaders(Outlook.MailItem mail)
        {
            return GetStringProperty(mail, TransportHeadersProperty);
        }

        public static bool TrySetTransportHeaders(Outlook.MailItem mail, string headers)
        {
            Outlook.PropertyAccessor accessor = mail.PropertyAccessor;
            try
            {
                accessor.SetProperty(TransportHeadersProperty, headers);
                return true;
            }
            catch (Exception ex)
            {
                AddInLog.Write("Could not update the transport headers of the rejoined message: " + ex.Message);
                return false;
            }
            finally
            {
                Release(accessor);
            }
        }

        public static bool IsRejoined(Outlook.MailItem mail)
        {
            Outlook.PropertyAccessor accessor = mail.PropertyAccessor;
            try
            {
                object value = accessor.GetProperty(RejoinedMarkerProperty);
                return value is bool && (bool)value;
            }
            catch (Exception)
            {
                return false; // property not set
            }
            finally
            {
                Release(accessor);
            }
        }

        public static void MarkRejoined(Outlook.MailItem mail)
        {
            Outlook.PropertyAccessor accessor = mail.PropertyAccessor;
            try
            {
                accessor.SetProperty(RejoinedMarkerProperty, true);
            }
            finally
            {
                Release(accessor);
            }
        }

        /// <summary>The SMTP address of a recipient, including Exchange users and groups.</summary>
        public static string GetSmtpAddress(Outlook.Recipient recipient)
        {
            Outlook.AddressEntry entry = null;
            try
            {
                entry = recipient.AddressEntry;
                if (entry != null && string.Equals(entry.Type, "EX", StringComparison.OrdinalIgnoreCase))
                {
                    Outlook.ExchangeUser user = entry.GetExchangeUser();
                    if (user != null)
                    {
                        string smtp = user.PrimarySmtpAddress;
                        Release(user);
                        if (!string.IsNullOrEmpty(smtp)) return smtp;
                    }

                    Outlook.ExchangeDistributionList list = entry.GetExchangeDistributionList();
                    if (list != null)
                    {
                        string smtp = list.PrimarySmtpAddress;
                        Release(list);
                        if (!string.IsNullOrEmpty(smtp)) return smtp;
                    }

                    Outlook.PropertyAccessor accessor = entry.PropertyAccessor;
                    try
                    {
                        string smtp = accessor.GetProperty(SmtpAddressProperty) as string;
                        if (!string.IsNullOrEmpty(smtp)) return smtp;
                    }
                    catch (Exception)
                    {
                        // fall back to the address below
                    }
                    finally
                    {
                        Release(accessor);
                    }
                }
            }
            catch (Exception ex)
            {
                AddInLog.Write("Could not read the SMTP address of " + recipient.Name + ": " + ex.Message);
            }
            finally
            {
                Release(entry);
            }

            return recipient.Address;
        }

        public static void Release(object comObject)
        {
            if (comObject != null && Marshal.IsComObject(comObject))
            {
                Marshal.ReleaseComObject(comObject);
            }
        }

        private static string GetStringProperty(Outlook.MailItem mail, string schemaName)
        {
            Outlook.PropertyAccessor accessor = mail.PropertyAccessor;
            try
            {
                return accessor.GetProperty(schemaName) as string;
            }
            catch (Exception)
            {
                return null; // property not set
            }
            finally
            {
                Release(accessor);
            }
        }
    }
}
