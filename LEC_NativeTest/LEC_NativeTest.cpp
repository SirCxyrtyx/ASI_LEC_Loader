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

#include "dnne.h"
#include "LegendaryExplorerCore_NativeNE.h"


#define MYHOOK "LEC_NativeTest_"

SPI_PLUGINSIDE_SUPPORT(L"LEC_NativeTest", L"SirCxyrtyx", L"0.1.0", SPI_GAME_LE3, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


SPI_IMPLEMENT_ATTACH
{
	Common::OpenConsole();
	INIT_CHECK_SDK();

	//uncomment to create a pause point for attaching the debugger
	//Common::MessageBoxError(L"LEC_NativeTest", L"Stopping for attach...");

	preload_runtime();

	LEC_NativeInit(6, reinterpret_cast<intptr_t>(malloc));

	wchar_t* wstr = LEC_TestMethod(L"..\\..\\BioGame\\CookedPCConsole\\core.pcc");
	wprintf(wstr);
	free(wstr);

	return true;
}

SPI_IMPLEMENT_DETACH
{
	//Common::CloseConsole();
	return true;
}
