using System.Runtime.InteropServices;
using System.Text;
using LegendaryExplorerCore;
using LegendaryExplorerCore.Helpers;
using LegendaryExplorerCore.Misc;
using LegendaryExplorerCore.Packages;
using LegendaryExplorerCore.Unreal.BinaryConverters;

namespace LegendaryExplorerCore_Native
{
    public static unsafe class LECInit
    {
        private static delegate* unmanaged<ulong, void*> NativeAlloc;

        [UnmanagedCallersOnly(EntryPoint = "LEC_NativeInit")]
        public static void LEC_NativeInit(int meGameToInitFor, delegate* unmanaged<ulong, void*> nativeAlloc)
        {
            try
            {
                MemoryAnalyzer.IsTrackingMemory = false;
                MEPackageHandler.GlobalSharedCacheEnabled = false;
                NativeAlloc = nativeAlloc;
                LegendaryExplorerCoreLib.InitLib(TaskScheduler.Current, _ => { }, objectDBsToLoad: new[] { (MEGame)meGameToInitFor });
            }
            catch (Exception e)
            {
                Console.WriteLine(e.FlattenException());
            }
        }

        [UnmanagedCallersOnly(EntryPoint = "LEC_TestMethod")]
        public static char* LEC_TestMethod(char* packagePath)
        {
            try
            {
                using IMEPackage? corePcc = MEPackageHandler.OpenMEPackage(new string(packagePath));
                var sb = new StringBuilder();
                foreach (TreeNode<IEntry, int> treeRoot in corePcc.Tree.Roots)
                {
                    sb.AppendLine($"{treeRoot.Data.UIndex}: {treeRoot.Data.ObjectName.Instanced}");
                }
                var rootsString = sb.ToString();
                int numBytes = Encoding.Unicode.GetByteCount(rootsString);
                char* ptr = (char*)NativeAlloc((ulong)numBytes + 2);
                Encoding.Unicode.GetBytes(rootsString.AsSpan(), new Span<byte>(ptr, numBytes));
                ptr[numBytes / 2] = '\0';
                return ptr;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.FlattenException());
            }
            return null;
        }
    }
}