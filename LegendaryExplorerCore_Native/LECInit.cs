using System.Runtime.InteropServices;
using System.Text;
using LegendaryExplorerCore;
using LegendaryExplorerCore.Helpers;
using LegendaryExplorerCore.Misc;
using LegendaryExplorerCore.Packages;
using LegendaryExplorerCore.Textures;
using LegendaryExplorerCore.Unreal;
using LegendaryExplorerCore.Unreal.BinaryConverters;
using LegendaryExplorerCore.Unreal.Classes;

namespace LegendaryExplorerCore_Native
{
    public static unsafe class LECInit
    {
        private static delegate* unmanaged<ulong, void*> NativeAlloc;

        [UnmanagedCallersOnly(EntryPoint = nameof(LEC_NativeInit))]
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

        [UnmanagedCallersOnly(EntryPoint = nameof(LEC_LoadPccUncompressed))]
        public static void* LEC_LoadPccUncompressed(char* filePath, int* packageDataSize)
        {
            try
            {
                using IMEPackage pcc = MEPackageHandler.OpenMEPackage(new string(filePath));

                Borgerify(pcc);

                using MemoryStream ms = pcc.SaveToStream(false);
                ms.Position = 0;
                int allocSize = (int)ms.Length;
                void *packageData = NativeAlloc((ulong)allocSize);
                ms.ReadToSpan(new Span<byte>(packageData, allocSize));
                *packageDataSize = allocSize;
                return packageData;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.FlattenException());
                *packageDataSize = 0;
                return null;
            }
        }

        [UnmanagedCallersOnly(EntryPoint = nameof(LEC_TestMethod))]
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

        private static void Borgerify(IMEPackage pcc)
        {
            const string imagePath = "Borger.PNG";
            foreach (ExportEntry exportEntry in pcc.Exports.Where(exp => exp.ClassName == "Texture2D"))
            {
                try
                {
                    var props = exportEntry.GetProperties();
                    var listedWidth = props.GetProp<IntProperty>("SizeX")?.Value ?? 0;
                    var listedHeight = props.GetProp<IntProperty>("SizeY")?.Value ?? 0;
                    Image image = Image.LoadFromFile(imagePath, PixelFormat.ARGB);
                    if (image.mipMaps[0].origWidth / image.mipMaps[0].origHeight != listedWidth / listedHeight)
                    {
                        continue;
                    }
                    var texture = new Texture2D(exportEntry);
                    texture.Replace(image, props, imagePath, isPackageStored: true);
                }
                catch (Exception e)
                {
                    continue;
                }
            }
        } 
    }
}