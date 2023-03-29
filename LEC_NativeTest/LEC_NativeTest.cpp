#include <shlwapi.h>
#include <psapi.h>
#include <string>
#include <fstream>
#include <ostream>
#include <streambuf>
#include <sstream>
#include <thread>

#include "../submodules/LEASIMods/Shared-ASI/Interface.h"
#include "../submodules/LEASIMods/Shared-ASI/Common.h"
#include "../submodules/LEASIMods/Shared-ASI/ME3Tweaks/ME3TweaksHeader.h"

#include "LegendaryExplorerCore_NativeNE.h"


#define MYHOOK "LEC_NativeTest_"

SPI_PLUGINSIDE_SUPPORT(L"LEC_NativeTest", L"SirCxyrtyx", L"0.1.0", SPI_GAME_LE3, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_POSTLOAD;
//SPI_PLUGINSIDE_SEQATTACH;
SPI_PLUGINSIDE_ASYNCATTACH;

#pragma pack(4)
class ThreadSafeCounter
{
public:
	ThreadSafeCounter() : Counter(0) {}
	
	void Increment()
	{
		InterlockedIncrement((LPLONG)&Counter);
	}
	
	void Decrement()
	{
		InterlockedDecrement((LPLONG)&Counter);
	}

	void UnsafeSet(int value)
	{
		Counter = value;
	}

private:
	ThreadSafeCounter(const ThreadSafeCounter& Other) {}
	void operator=(const ThreadSafeCounter& Other) {}
	
	volatile int Counter;
};

struct FPackagePreCacheInfo
{
	ThreadSafeCounter* SynchronizationObject;
	void* PackageData;
	int PackageDataSize;

	FPackagePreCacheInfo() : SynchronizationObject(nullptr), PackageData(nullptr), PackageDataSize(0) {}

	~FPackagePreCacheInfo()
	{
		if (SynchronizationObject)
		{
			GMalloc.Free(SynchronizationObject);
		}
	}
};

DWORD GetTypeHash(const FString& fStr)
{
	constexpr DWORD GCRCTable[] = {
		0, 79764919, 159529838, 222504665, 319059676, 398814059, 445009330, 507990021, 638119352, 583659535, 797628118,
		726387553, 890018660, 835552979, 1015980042, 944750013, 1276238704, 1221641927, 1167319070, 1095957929,
		1595256236, 1540665371, 1452775106, 1381403509, 1780037320, 1859660671, 1671105958, 1733955601, 2031960084,
		2111593891, 1889500026, 1952343757, 2552477408, 2632100695, 2443283854, 2506133561, 2334638140, 2414271883,
		2191915858, 2254759653, 3190512472, 3135915759, 3081330742, 3009969537, 2905550212, 2850959411, 2762807018,
		2691435357, 3560074640, 3505614887, 3719321342, 3648080713, 3342211916, 3287746299, 3467911202, 3396681109,
		4063920168, 4143685023, 4223187782, 4286162673, 3779000052, 3858754371, 3904687514, 3967668269, 881225847,
		809987520, 1023691545, 969234094, 662832811, 591600412, 771767749, 717299826, 311336399, 374308984, 453813921,
		533576470, 25881363, 88864420, 134795389, 214552010, 2023205639, 2086057648, 1897238633, 1976864222, 1804852699,
		1867694188, 1645340341, 1724971778, 1587496639, 1516133128, 1461550545, 1406951526, 1302016099, 1230646740,
		1142491917, 1087903418, 2896545431, 2825181984, 2770861561, 2716262478, 3215044683, 3143675388, 3055782693,
		3001194130, 2326604591, 2389456536, 2200899649, 2280525302, 2578013683, 2640855108, 2418763421, 2498394922,
		3769900519, 3832873040, 3912640137, 3992402750, 4088425275, 4151408268, 4197601365, 4277358050, 3334271071,
		3263032808, 3476998961, 3422541446, 3585640067, 3514407732, 3694837229, 3640369242, 1762451694, 1842216281,
		1619975040, 1682949687, 2047383090, 2127137669, 1938468188, 2001449195, 1325665622, 1271206113, 1183200824,
		1111960463, 1543535498, 1489069629, 1434599652, 1363369299, 622672798, 568075817, 748617968, 677256519,
		907627842, 853037301, 1067152940, 995781531, 51762726, 131386257, 177728840, 240578815, 269590778, 349224269,
		429104020, 491947555, 4046411278, 4126034873, 4172115296, 4234965207, 3794477266, 3874110821, 3953728444,
		4016571915, 3609705398, 3555108353, 3735388376, 3664026991, 3290680682, 3236090077, 3449943556, 3378572211,
		3174993278, 3120533705, 3032266256, 2961025959, 2923101090, 2868635157, 2813903052, 2742672763, 2604032198,
		2683796849, 2461293480, 2524268063, 2284983834, 2364738477, 2175806836, 2238787779, 1569362073, 1498123566,
		1409854455, 1355396672, 1317987909, 1246755826, 1192025387, 1137557660, 2072149281, 2135122070, 1912620623,
		1992383480, 1753615357, 1816598090, 1627664531, 1707420964, 295390185, 358241886, 404320391, 483945776,
		43990325, 106832002, 186451547, 266083308, 932423249, 861060070, 1041341759, 986742920, 613929101, 542559546,
		756411363, 701822548, 3316196985, 3244833742, 3425377559, 3370778784, 3601682597, 3530312978, 3744426955,
		3689838204, 3819031489, 3881883254, 3928223919, 4007849240, 4037393693, 4100235434, 4180117107, 4259748804,
		2310601993, 2373574846, 2151335527, 2231098320, 2596047829, 2659030626, 2470359227, 2550115596, 2947551409,
		2876312838, 2788305887, 2733848168, 3165939309, 3094707162, 3040238851, 2985771188
	};

	wchar_t* data = fStr.Data;
	DWORD hash = 0;
	while (*data)
	{
		const wchar_t ch = toupper(*data++);
		WORD  b = ch;
		hash = ((hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(hash ^ b) & 0x000000FF];
		b = ch >> 8;
		hash = ((hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(hash ^ b) & 0x000000FF];
	}
	return hash;
}

bool operator==(const FString& a, const FString& b)
{
	return wcscmp(a.Data, b.Data) == 0;
}

TMap<FString, FPackagePreCacheInfo>* PackagePrecacheMap;

int* (*AddToPackagePrecacheMap)(TMap<FString, FPackagePreCacheInfo>* packagePrecacheMap, int* out_Index, wchar_t** PackageFilename, void* zero);

typedef unsigned (*tCreateLoader)(ULinkerLoad* This);
tCreateLoader CreateLoader = nullptr;
tCreateLoader CreateLoader_orig = nullptr;

unsigned CreateLoader_hook(ULinkerLoad* This)
{
	//CreateLoader is potentially called multiple times during a single package load.
	//We only want to run on the first call, when Loader has not been created.
	if (This->Loader == nullptr)
	{
		bool shouldIntercept = true;
		//TODO: check This->Filename against files we want to intercept
		if (shouldIntercept)
		{
			wprintf(L"%s \n", This->Filename.Data);

			//we make a copy of the FString since the one we pass to PackagePrecacheMap will be deallocated when it's removed.
			FString filename;
			filename.Max = filename.Count = This->Filename.Count;
			filename.Data = static_cast<wchar_t*>(GMalloc.Malloc(filename.Count * sizeof(wchar_t)));
			memcpy(filename.Data, This->Filename.Data, filename.Count * sizeof(wchar_t));
			
			FPackagePreCacheInfo& precacheInfo = PackagePrecacheMap->Set(filename, FPackagePreCacheInfo());

			precacheInfo.SynchronizationObject = new(GMalloc.Malloc(4)) ThreadSafeCounter;
			precacheInfo.SynchronizationObject->UnsafeSet(1);

			//This block could be done on a different thread
			{
				precacheInfo.PackageData = LEC_LoadPccUncompressed(filename.Data, &precacheInfo.PackageDataSize);

				if (precacheInfo.PackageData == nullptr)
				{
					Common::MessageBoxError(L"LegendaryExplorerCore ASI Test Error", L"Unrecoverable error opening package \"%s\", program will now exit.", filename.Data);
					abort();
				}

				precacheInfo.SynchronizationObject->Decrement();
			}

			//Don't free the data in filename! It will be freed when it is removed from PackagePrecacheMap in CreateLoader_orig
			//GMalloc.Free(filename.Data);
		}
	}
	return CreateLoader_orig(This);
}


void* unrealMallocProxy(const size_t size)
{
	return GMalloc.Malloc(static_cast<DWORD>(size));
}

SPI_IMPLEMENT_ATTACH
{
	Common::OpenConsole();
	INIT_CHECK_SDK();

	//wprintf(L"%llu \n", offsetof(ULinkerLoad, UnknownData00));

	//uncomment to create a pause point for attaching the debugger
	Common::MessageBoxError(L"LEC_NativeTest", L"Stopping for attach...");

	preload_runtime();

	LEC_NativeInit(6, reinterpret_cast<intptr_t>(unrealMallocProxy));

	/*wchar_t* wstr = LEC_TestMethod(L"..\\..\\BioGame\\CookedPCConsole\\core.pcc");
	wprintf(wstr);
	free(wstr);*/

	constexpr auto createLoaderPattern =
#ifdef GAMELE1
		;
#elif defined GAMELE2
		;
#elif defined GAMELE3
		/*48 8b c4 55 41*/ "54 41 55 41 56 41 57 48 8d a8 e8 fe ff ff 48 81 ec f0 01 00 00 48 c7 85 b0 00 00 00 fe ff ff ff";
#endif
	INIT_POSTHOOK(CreateLoader, createLoaderPattern)

		constexpr auto packagePrecacheMapPattern =
#ifdef GAMELE1 
		;
#elif defined(GAMELE2)
		;
#elif defined(GAMELE3)
		"48 8d 0d c8 24 70 01 e8 fb fa ff ff";
#endif
	PackagePrecacheMap = static_cast<TMap<FString, FPackagePreCacheInfo>*>(findAddressLeaMov("PackagePrecacheMap", packagePrecacheMapPattern));

	constexpr auto addToMapPattern =
#ifdef GAMELE1 
		;
#elif defined(GAMELE2)
		;
#elif defined(GAMELE3)
		/*4c 89 44 24 18*/"55 56 57 41 54 41 55 41 56 41 57 48 83 ec 50 48 c7 44 24 20 fe ff ff ff 48 89 9c 24 98 00 00 00";
#endif
	INIT_FIND_PATTERN_POSTHOOK(AddToPackagePrecacheMap, addToMapPattern)
	
	return true;
}

SPI_IMPLEMENT_DETACH
{
	//Common::CloseConsole();
	return true;
}
