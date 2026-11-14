using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace Mpc.Core.Tests
{
    [AttributeUsage(AttributeTargets.Method)]
    internal sealed class TestAttribute : Attribute { }

    internal static class Assert
    {
        public static void Equal<T>(T expected, T actual, string what = null)
        {
            if (!EqualityComparer<T>.Default.Equals(expected, actual))
            {
                throw new Exception((what == null ? "" : what + ": ") + "expected <" + expected + "> but got <" + actual + ">");
            }
        }

        public static void True(bool condition, string what)
        {
            if (!condition) throw new Exception("expected true: " + what);
        }

        public static void False(bool condition, string what)
        {
            if (condition) throw new Exception("expected false: " + what);
        }

        public static void Null(object value, string what)
        {
            if (value != null) throw new Exception("expected null: " + what + " (got " + value + ")");
        }

        public static void NotNull(object value, string what)
        {
            if (value == null) throw new Exception("expected a value: " + what);
        }

        public static void Sequence<T>(IEnumerable<T> expected, IEnumerable<T> actual, string what)
        {
            var e = expected.ToList();
            var a = actual.ToList();
            if (!e.SequenceEqual(a))
            {
                throw new Exception(what + ": expected [" + string.Join(", ", e) + "] but got [" + string.Join(", ", a) + "]");
            }
        }

        public static TException Throws<TException>(Action action, string what) where TException : Exception
        {
            try
            {
                action();
            }
            catch (TException ex)
            {
                return ex;
            }
            catch (Exception ex)
            {
                throw new Exception(what + ": expected " + typeof(TException).Name + " but got " + ex.GetType().Name + ": " + ex.Message);
            }
            throw new Exception(what + ": expected " + typeof(TException).Name + " but nothing was thrown");
        }
    }

    internal static class Program
    {
        private static int Main()
        {
            var tests = typeof(Program).Assembly.GetTypes()
                .SelectMany(t => t.GetMethods(BindingFlags.Public | BindingFlags.Static))
                .Where(m => m.GetCustomAttribute<TestAttribute>() != null)
                .OrderBy(m => m.DeclaringType.Name).ThenBy(m => m.Name)
                .ToList();

            int failed = 0;
            foreach (MethodInfo test in tests)
            {
                string name = test.DeclaringType.Name + "." + test.Name;
                try
                {
                    test.Invoke(null, null);
                    Console.WriteLine("PASS  " + name);
                }
                catch (TargetInvocationException ex)
                {
                    failed++;
                    Console.WriteLine("FAIL  " + name + "\n      " + ex.InnerException.Message);
                }
            }

            Console.WriteLine();
            Console.WriteLine((tests.Count - failed) + " passed, " + failed + " failed, " + tests.Count + " total");
            return failed == 0 ? 0 : 1;
        }
    }
}
