using System.Runtime.InteropServices;
using System.Text;
using LegendaryExplorerCore;
using LegendaryExplorerCore.Packages;

namespace LegendaryExplorerCore_Native
{
    public static unsafe class LECInit
    {
        private static delegate* unmanaged<ulong, void*> NativeAlloc;

        [UnmanagedCallersOnly(EntryPoint = "NativeInit")]
        public static void NativeInit(int meGameToInitFor, delegate* unmanaged<ulong, void*> nativeAlloc)
        {
            NativeAlloc = nativeAlloc;
            LegendaryExplorerCoreLib.InitLib(TaskScheduler.FromCurrentSynchronizationContext(), s => { }, objectDBsToLoad: new[] { (MEGame)meGameToInitFor });
        }

        [UnmanagedCallersOnly(EntryPoint = "TestMethod")]
        public static char* Test()
        {
            char* ptr = (char*)NativeAlloc(10);

            ptr[0] = 'T';
            ptr[1] = 'e';
            ptr[2] = 's';
            ptr[3] = 't';
            ptr[4] = '\0';
            return ptr;
        }
    }
}