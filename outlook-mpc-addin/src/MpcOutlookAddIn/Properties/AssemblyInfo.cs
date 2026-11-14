using System.Reflection;
using System.Runtime.InteropServices;

[assembly: AssemblyTitle("MPC Outlook Add-in")]
[assembly: AssemblyDescription("Splits outgoing mail into X-MPC chunks and rejoins incoming X-MPC chunks")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyCompany("")]
[assembly: AssemblyProduct("MpcOutlookAddIn")]
[assembly: AssemblyCopyright("")]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

// Only MpcRibbon is exposed to COM (it carries its own [ComVisible(true)]).
[assembly: ComVisible(false)]

[assembly: Guid("a5d76915-c094-4c6f-8e5c-c9e9cadf93e8")]

[assembly: AssemblyVersion("1.0.0.0")]
[assembly: AssemblyFileVersion("1.0.0.0")]
