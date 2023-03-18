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


#define MYHOOK "LE3LevelGen_"

SPI_PLUGINSIDE_SUPPORT(L"LE3LevelGen", L"SirCxyrtyx", L"0.1.0", SPI_GAME_LE3, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;



typedef void (*tProcessEvent)(UObject* Context, UFunction* Function, void* Parms, void* Result);
tProcessEvent ProcessEvent = nullptr;
tProcessEvent ProcessEvent_orig = nullptr;

size_t MaxMemoryHit = 0;
bool DrawSLH = false;
bool CanToggleDrawSLH = false;
char lastTouchedTriggerStream[512];
float textScale = 1.0f;
float lineHeight = 12.0f;
static void RenderTextSLH(std::wstring msg, const float x, const float y, const char r, const char g, const char b, const float alpha, UCanvas* can)
{
	can->SetDrawColor(r, g, b, alpha * 255);
	can->SetPos(x, y + 64); //+ is Y start. To prevent overlay on top of the power bar thing
	can->DrawTextW(FString{ const_cast<wchar_t*>(msg.c_str()) }, 1, textScale, textScale, nullptr);
}

const char* FormatBytes(size_t bytes, char* keepInStackStr)
{
	const char* sizes[4] = { "B", "KB", "MB", "GB" };

	int i;
	double dblByte = bytes;
	for (i = 0; i < 4 && bytes >= 1024; i++, bytes /= 1024)
		dblByte = bytes / 1024.0;

	sprintf(keepInStackStr, "%.2f", dblByte);

	return strcat(strcat(keepInStackStr, " "), sizes[i]);
}

int line = 0;
PROCESS_MEMORY_COUNTERS pmc;

void SetTextScale()
{
	HWND activeWindow = FindWindowA(NULL, "Mass Effect 3");
	if (activeWindow)
	{
		RECT rcOwner;
		if (GetWindowRect(activeWindow, &rcOwner))
		{
			long width = rcOwner.right - rcOwner.left;
			long height = rcOwner.bottom - rcOwner.top;

			if (width > 2560 && height > 1440)
			{
				textScale = 2.0f;
			}
			else if (width > 1920 && height > 1080)
			{
				textScale = 1.5f;
			}
			else
			{
				textScale = 1.0f;
			}
			lineHeight = 12.0f * textScale;
		}
	}
}

wstring nameTable;

void ProcessEvent_hook(UObject* Context, UFunction* Function, void* Parms, void* Result)
{
	auto funcFullName = Function->GetFullName();
	if (!strcmp(Function->Name.GetName(), "DrawHUD"))
	{
		//writeln(L"Func: %hs", Function->GetFullName());
		auto hud = reinterpret_cast<AHUD*>(Context);
		if (hud != nullptr)
		{
			hud->Canvas->SetDrawColor(0, 0, 0, 255);
			hud->Canvas->SetPos(0, 0);
			hud->Canvas->DrawBox(1920, 1080);
		}
	}
	else if (!strcmp(funcFullName, "Function SFXGame.BioHUD.PostRender"))
	{
		line = 0;
		auto hud = reinterpret_cast<ABioHUD*>(Context);
		if (hud != nullptr)
		{
			// Toggle drawing/not drawing
			if ((GetKeyState('T') & 0x8000) && (GetKeyState(VK_CONTROL) & 0x8000)) {
				if (CanToggleDrawSLH) {
					CanToggleDrawSLH = false; // Will not activate combo again until you re-press combo
					DrawSLH = !DrawSLH;
					if (DrawSLH)
					{
						wchar_t* wstr = TestMethod();
						nameTable = wstr;
						free(wstr);
					}
				}
			}
			else
			{
				if (!(GetKeyState('T') & 0x8000) || !(GetKeyState(VK_CONTROL) & 0x8000)) {
					CanToggleDrawSLH = true; // can press key combo again
				}
			}

			if (DrawSLH && hud->WorldInfo)
			{
				SetTextScale();

				RenderTextSLH(nameTable, 5, 0, 0, 255, 64, 1.0f, hud->Canvas);
			}
		}
	}

	ProcessEvent_orig(Context, Function, Parms, Result);
}


DNNE_API void dnne_abort(enum failure_type type, int error_code)
{
	Common::MessageBoxError(L"LE3LevelGen Error", L".NET 7 runtime not installed!");
	exit(error_code);
}

SPI_IMPLEMENT_ATTACH
{
	INIT_CHECK_SDK();

	Common::MessageBoxError(L"LE3LevelGen Error", L"Beep Boop");
	preload_runtime();

	NativeInit(6, reinterpret_cast<intptr_t>(malloc));

	wchar_t* wstr = TestMethod();

	INIT_POSTHOOK(ProcessEvent, LE_PATTERN_POSTHOOK_PROCESSEVENT);

	return true;
}

SPI_IMPLEMENT_DETACH
{
	//Common::CloseConsole();
	return true;
}
