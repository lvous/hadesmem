using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.ComponentModel;

namespace HadesAD
{
    internal static class CmdLineToArgvW
    {
        // The previous examples on this page used incorrect
        // pointer logic and were removed.

        internal static string[] SplitArgs(string unsplitArgumentLine)
        {
            int numberOfArgs;
            IntPtr ptrToSplitArgs;
            string[] splitArgs;

            ptrToSplitArgs = CommandLineToArgvW(unsplitArgumentLine, 
                out numberOfArgs);

            // CommandLineToArgvW returns NULL upon failure.
            if (ptrToSplitArgs == IntPtr.Zero)
                throw new ArgumentException("Unable to split argument.", 
                    new Win32Exception());

            // Make sure the memory ptrToSplitArgs to is freed, even upon 
            // failure.
            try
            {
                splitArgs = new string[numberOfArgs];

                // ptrToSplitArgs is an array of pointers to null terminated 
                // Unicode strings.
                // Copy each of these strings into our split argument array.
                for (int i = 0; i < numberOfArgs; i++)
                {
                    splitArgs[i] = Marshal.PtrToStringUni(Marshal.ReadIntPtr(
                        ptrToSplitArgs, i * IntPtr.Size));
                }

                return splitArgs;
            }
            finally
            {
                // Free memory obtained by CommandLineToArgW.
                LocalFree(ptrToSplitArgs);
            }
        }

        [DllImport("shell32.dll", SetLastError = true)]
        static extern IntPtr CommandLineToArgvW(
            [MarshalAs(UnmanagedType.LPWStr)] string lpCmdLine,
            out int pNumArgs);

        [DllImport("kernel32.dll")]
        static extern IntPtr LocalFree(IntPtr hMem);
    }

    public interface IHadesVM
    {
        void RegisterOnFrame([In] IntPtr callback);
        uint EnumDomains([In] IntPtr callback, [In] int pData);
        dlgExecute GetExecute();
        dlgFrame GetFrame();
        bool RunAssembly([In] string appDomainName, [In] string assemblyName,
            [In] string parameters);
        bool IsDomainActive([In] string appDomainName);
        bool UnloadDomain([In] string appDomainName);
    }

    public delegate void dlgDomainEnumCallback(string callback, int pData);
    public delegate bool dlgExecute(string appDomain, string appName,
        string parameters);
    public delegate void dlgFrame();
    internal delegate void dlgSetFrame(IntPtr frameFunc);
    
    public class Hades
    {
        public static void Echo(string output)
        {
            MessageBox.Show(output);
        }
    }

    public class HadesVM : AppDomainManager, IHadesVM
    {
        private static Dictionary<string, AppDomain> Domains =
          new Dictionary<string, AppDomain>();
        private static List<dlgFrame> FrameHandlers = new List<dlgFrame>();
        private static dlgExecute Execute = new dlgExecute(HadesVM.Run);
        private static dlgFrame Frame = new dlgFrame(HadesVM.OnFrame);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, 
            SetLastError = true)]
        static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)] 
        internal delegate IntPtr dlgRunLuaScript([MarshalAs(UnmanagedType.
            LPStr)] string Script, uint Index);

        public static string GetScriptResult(string Script, uint Index)
        {
            IntPtr HadesKernelMod = GetModuleHandle("Hades-Kernel_IA32.dll");
            if (HadesKernelMod.ToInt32() == 0)
            {
                throw new Exception("Could not find Hades Kernel DLL.");
            }

            IntPtr pRunLuaScript = GetProcAddress(HadesKernelMod, 
                "_RunLuaScript@8");
            if (pRunLuaScript.ToInt32() == 0)
            {
                throw new Exception("Could not find RunLuaScript export.");
            }

            dlgRunLuaScript RunLuaScriptFunc = (dlgRunLuaScript)Marshal.
                GetDelegateForFunctionPointer(pRunLuaScript, 
                typeof(dlgRunLuaScript));

            IntPtr Result = RunLuaScriptFunc(Script, Index);
            if (Result.ToInt32() == 0)
            {
                throw new Exception("Could not get requested result.");
            }

            return Marshal.PtrToStringAnsi(Result);
        }

        internal static void AssemblyExecuter(object Obj)
        {
            AsEx ex = (AsEx)Obj;
            Thread.CurrentThread.Name = ex.appdomainName +
                " AppDomain main ExecThread";
            try
            {
                ex.ExecuteAssembly();
            }
            catch (Exception exception)
            {
                Hades.Echo("Unhandled exception in application.");
                Hades.Echo("Outer Exception: " + exception.ToString());
                EchoInnerExceptions(exception);
            }

            lock (Domains)
            {
                try
                {
                    Domains.Remove(ex.appdomainName);
                    DomainUnloader(ex.Domain);
                }
                catch (Exception e)
                {
                    Hades.Echo("Unhandled exception post application death.");
                    Hades.Echo("Outer Exception: " + e.ToString());
                }
            }
        }

        public void RegisterOnFrame([In] IntPtr value)
        {
            dlgSetFrame delegateForFunctionPointer =
              (dlgSetFrame)Marshal.GetDelegateForFunctionPointer(value,
              typeof(dlgSetFrame));
            IntPtr ptrForDelegate =
                Marshal.GetFunctionPointerForDelegate(Frame);

            delegateForFunctionPointer(ptrForDelegate);
        }

        public bool RunAssembly([In] string appDomainName,
            [In] string assemblyName, [In] string parameters)
        {
            Run(appDomainName, assemblyName, parameters);
            return true;
        }

        public static void DomainUnloader(object Obj)
        {
            AppDomain domain = (AppDomain)Obj;
            try
            {
                AppDomain.Unload(domain);
            }
            catch (Exception exception)
            {
                Hades.Echo("Exception unloading domain: " + 
                    exception.ToString());
            }
        }

        public void EchoDomains()
        {
            Hades.Echo("Current Application Domains\n---------------------" +
                "--------------");
            lock (Domains)
            {
                foreach (string domain in Domains.Keys)
                {
                    Hades.Echo(domain);
                }
            }
        }

        internal static void EchoInnerExceptions(Exception e)
        {
            for (int i = 1; i <= 10; i++)
            {
                if (e.InnerException == null)
                {
                    return;
                }
                e = e.InnerException;
                Hades.Echo("Inner Exception[" + i.ToString() + "]: " + 
                    e.ToString());
            }
        }

        public uint EnumDomains(IntPtr callback, int pData)
        {
            dlgDomainEnumCallback delegateForFunctionPointer =
                (dlgDomainEnumCallback)Marshal.GetDelegateForFunctionPointer(
                callback, typeof(dlgDomainEnumCallback));
            uint num = 0;
            lock (Domains)
            {
                foreach (string domain in Domains.Keys)
                {
                    num++;
                    delegateForFunctionPointer(domain, pData);
                }
            }
            return num;
        }

        public dlgExecute GetExecute()
        {
            return Execute;
        }

        public dlgFrame GetFrame()
        {
            return Frame;
        }

        public override void InitializeNewDomain(AppDomainSetup appDomainInfo)
        {
            base.InitializationFlags =
                AppDomainManagerInitializationOptions.RegisterWithHost;
        }

        public bool IsDomainActive([In] string appDomainName)
        {
            lock (Domains)
            {
                return Domains.ContainsKey(appDomainName);
            }
        }

        public static void OnFrame()
        {
            try
            {
                lock (Domains)
                {
                    foreach (AppDomain appDomain in Domains.Values)
                    {
                        appDomain.DoCallBack(OnFrame);
                    }
                }

                foreach (dlgFrame frameHandler in FrameHandlers)
                    frameHandler();
            }
            catch (Exception e)
            {
                Hades.Echo("Exception in frame handler: " + e.ToString());
            }
        }

        public static void AddFrameHandler(dlgFrame frameHandler)
        {
            FrameHandlers.Add(frameHandler);
        }

        public static void RemoveFrameHandler(dlgFrame frameHandler)
        {
            if (FrameHandlers.Contains(frameHandler))
                FrameHandlers.Remove(frameHandler);
        }

        public static bool Run(string appDomainName, string assemblyName,
            string parameters)
        {
            try
            {
                AppDomain domain;
                if (!Domains.TryGetValue(appDomainName, out domain))
                {
                    AppDomainSetup info = new AppDomainSetup();
                    info.PrivateBinPath = ".NET Programs";
                    info.ApplicationBase = Path.GetDirectoryName(
                      Process.GetCurrentProcess().MainModule.FileName);
                    domain = AppDomain.CreateDomain(appDomainName, null, info);
                    domain.SetData("parameters", parameters);
                    if (domain == null)
                    {
                        Hades.Echo("Could not create domain called " +
                            appDomainName);
                        return false;
                    }
                    Domains.Add(appDomainName, domain);
                }
                else
                {
                    Hades.Echo("Could not create domain called " +
                        appDomainName + ". The domain already exists.");
                    Hades.Echo("If you want to launch another application, " + 
                        "including the same application, please choose " + 
                        "another name.");
                    return false;
                }
                AsEx parameter = new AsEx(domain, assemblyName, appDomainName,
                  parameters);
                Thread ExecThread = new Thread(HadesVM.AssemblyExecuter);
                ExecThread.SetApartmentState(ApartmentState.STA);
                ExecThread.Start(parameter);
                return true;
            }
            catch (Exception exception)
            {
                Hades.Echo("Exception loading assembly: " + 
                    exception.ToString());
                return false;
            }
        }

        public bool UnloadDomain([In] string appDomainName)
        {
            try
            {
                lock (Domains)
                {
                    AppDomain domain;
                    if (!Domains.TryGetValue(appDomainName, out domain))
                    {
                        return false;
                    }
                    Domains.Remove(appDomainName);
                    new Thread(HadesVM.DomainUnloader).Start(domain);
                }
            }
            catch (Exception exception)
            {
                Hades.Echo("Exception unloading domain: " + 
                    exception.ToString());
                return false;
            }
            return true;
        }

        internal class AsEx
        {
            internal string appdomainName;
            internal string[] parameters;
            internal string assemblyName;
            internal AppDomain Domain;

            internal AsEx(AppDomain p_Domain, string p_assemblyName,
                string p_appdomainName, string parameters)
            {
                this.assemblyName = p_assemblyName;
                this.parameters = CmdLineToArgvW.SplitArgs(parameters);
                this.Domain = p_Domain;
                this.appdomainName = p_appdomainName;
            }

            internal int ExecuteAssembly()
            {
                int num;
                try
                {
                    return this.Domain.ExecuteAssembly(this.assemblyName, 
                        new System.Security.Policy.Evidence(), parameters);
                }
                catch (FileNotFoundException exception)
                {
                    num = this.ExecuteAssemblyByName(exception);
                }
                catch (FileLoadException exception2)
                {
                    num = this.ExecuteAssemblyByName(exception2);
                }
                catch (Exception e)
                {
                    Hades.Echo(e.ToString());
                    throw;
                }
                return num;
            }

            private int ExecuteAssemblyByName(Exception priorException)
            {
                try
                {
                    return this.Domain.ExecuteAssemblyByName(
                        this.assemblyName);
                }
                catch (FileNotFoundException exception)
                {
                    Hades.Echo("Exception[1] executing assembly: " +
                      priorException.ToString());
                    HadesVM.EchoInnerExceptions(priorException);
                    Hades.Echo("Exception[2] executing assembly: " +
                      exception.ToString());
                    HadesVM.EchoInnerExceptions(exception);
                }
                catch (FileLoadException exception2)
                {
                    Hades.Echo("Exception[1] executing assembly: " +
                        priorException.ToString());
                    HadesVM.EchoInnerExceptions(priorException);
                    Hades.Echo("Exception[2] executing assembly: " +
                        exception2.ToString());
                    HadesVM.EchoInnerExceptions(exception2);
                }
                catch (Exception)
                {
                    throw;
                }
                return -1;
            }
        }
    }
}
