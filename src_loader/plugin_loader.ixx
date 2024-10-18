// 2024-10-10 ghoulsblade plugin_loader : the goal is to be able to reload a plugin dll without restarting skyrim
// so a 2nd dll is used as "loader", it loads the actual plugin on kDataLoaded, and listens for some hotkey or console command to reload it
// the plugin itself exports two functions OnLoadPlugin and OnUnLoadPlugin, which are called by the loader as needed
module;

#include <windows.h> // loader dll stuff
//#include "SKEE.h" // racemenu morphinterface

export module plugin_loader;
import shared_util;
import gregistry;

std::string GetErrorMessage(DWORD errorMessageID) {
    if (errorMessageID == 0) { return "No error"; }

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    // Free the buffer allocated by FormatMessage
    LocalFree(messageBuffer);
    
    // Replace \r\n with space
    size_t pos = 0;
    while ((pos = message.find("\r\n", pos)) != std::string::npos) {
        message.replace(pos, 2, " ");
        pos += 1; // Move past the replaced character
    }

    return message;
}

std::string GetLastErrorMessage() { return GetErrorMessage(::GetLastError()); }

void PrintLastError(std::string_view context) {
    std::string errorMessage = GetLastErrorMessage();
    myprint("{}: {}", context, errorMessage);
}

std::filesystem::path GetDllDirectoryPath(const char* szDllFileName="GHotReload_loader.dll")
{
    char path[MAX_PATH];
    HMODULE hModule = GetModuleHandleA(szDllFileName); // Get handle to the current module
    if (GetModuleFileNameA(hModule, path, MAX_PATH) == 0) return "GetModuleFileNameA_error";
    std::filesystem::path dllPath(path);
    return dllPath.parent_path(); // note: SetDllDirectoryA
}

HMODULE MyLoadLibrary(std::string dllPath)
{
    HMODULE hModule = LoadLibraryA(dllPath.c_str());
    if (!hModule) { PrintLastError(std::format("LoadLibraryA FAILED: {}",dllPath)); }
    return hModule;
}

struct LoaderEventSinkBase {};
class MyLoader :
	public RE::BSTEventSink<RE::InputEvent*>, // hotkey
	public RE::BSTEventSink<SKSE::CrosshairRefEvent>, // crosshair over object
    LoaderEventSinkBase
{
public:
    GRegistry gregistry;
    MyLoader() { GRegistry::SetSingleton(&gregistry); }
    ~MyLoader() {}

    MyLoader* GetEventSink () { return this; }

	const uint32_t SCANCODE_reload = 65; // f1=59.. f7=65  f11=87 
    std::string             sub_filename        = "GHotReload.dll";
    std::string             sub_filename_tmp    = "GHotReload-tmp.dll";
    std::string             sub_foldername      = "sub";
    std::filesystem::path   sub_dll_path;       // data\SKSE\Plugins\sub\GHotReload.dll
    std::filesystem::path   sub_dll_path_tmp;   // data\SKSE\Plugins\sub\GHotReload-tmp.dll
    HMODULE hModule = nullptr; // loaded lib

    // api
    typedef void (*tFun_OnLoadPlugin)(GRegistry*,bool);
    typedef void (*tFun_void_void)();
    typedef void (*tFun_OnButtonDown)(uint32_t);
    typedef void (*tFun_OnCrossHairObject)(RE::TESBoundObject*);
    typedef RE::BSEventNotifyControl (*tFun_OnInputEvent)(RE::InputEvent* const*, RE::BSTEventSource<RE::InputEvent*>*);
    

    
    tFun_OnLoadPlugin       pFun_OnLoadPlugin       = nullptr;
    tFun_void_void          pFun_OnUnLoadPlugin     = nullptr;
    tFun_OnButtonDown       pFun_OnButtonDown       = nullptr;
    tFun_OnCrossHairObject  pFun_OnCrossHairObject  = nullptr;
    tFun_OnInputEvent       pFun_OnInputEvent       = nullptr;

    void MySub_Load_Api ()
    {
        // Get the function pointers
        auto MyGetProcAddress = [](HMODULE hModule, const char* funcName) {
            auto proc = GetProcAddress(hModule, funcName);
            if (!proc) myprint("no function {}",funcName);
            return proc;
        };
        pFun_OnLoadPlugin           = (tFun_OnLoadPlugin)MyGetProcAddress(hModule,      "OnLoadPlugin");
        pFun_OnUnLoadPlugin         = (tFun_void_void)MyGetProcAddress(hModule,         "OnUnLoadPlugin");
        pFun_OnButtonDown           = (tFun_OnButtonDown)MyGetProcAddress(hModule,      "OnButtonDown");
        pFun_OnCrossHairObject      = (tFun_OnCrossHairObject)MyGetProcAddress(hModule, "OnCrossHairObject");
        pFun_OnInputEvent           = (tFun_OnInputEvent)MyGetProcAddress(hModule,      "OnInputEvent");

    }

    void MySub_UnLoad_Api ()
    {
        pFun_OnLoadPlugin           = nullptr;
        pFun_OnUnLoadPlugin         = nullptr;
        pFun_OnButtonDown           = nullptr;
        pFun_OnCrossHairObject      = nullptr;
        pFun_OnInputEvent           = nullptr;
    }

    void MySub_Load (bool bReload)
    {
        //myprint("GHotReload-Loader: Load.. {}",sub_dll_path.string());
        if (hModule) { myprint("MySub_Load: already loaded"); return; }
        if (sub_dll_path.empty()) { myprint("MySub_Load: empty path"); return; }

        try {
            std::filesystem::copy(sub_dll_path, sub_dll_path_tmp, std::filesystem::copy_options::overwrite_existing);
            //myprint("sub copied from {} to {}",sub_dll_path.string(),sub_dll_path_tmp.string());
        } catch (const std::filesystem::filesystem_error& e) {
            myprint("Error copying file: {}",e.what());
        }

        // Load the other DLL
        std::string path = sub_dll_path_tmp.string();
        hModule = MyLoadLibrary(path);
        if (!hModule) { myprint("MySub_Load FAIL"); return; }
        //myprint("successfully loaded {}!", path);

        MySub_Load_Api(); // get function pointers

        // Use the function
        //myprint("GHotReload-Loader:OnLoadPlugin...");
        if (pFun_OnLoadPlugin) pFun_OnLoadPlugin(&GRegistry::GetSingleton(),bReload);
        //myprint("GHotReload-Loader:OnLoadPlugin done.");
    }
    
    void MySub_UnLoad ()
    {
        //myprint("GHotReload-Loader: UnLoad.."); 
        if (!hModule) { myprint("MySub_UnLoad: not loaded"); return; }
        //myprint("GHotReload-Loader:OnUnLoadPlugin...");
        if (pFun_OnUnLoadPlugin) pFun_OnUnLoadPlugin();
        //myprint("GHotReload-Loader:OnUnLoadPlugin done.");

        // Free the DLL when done
        FreeLibrary(hModule);
        MySub_UnLoad_Api();
        hModule = nullptr;
    }

    void MySub_ReLoad ()
    {
        myprint("GHotReload-Loader: ReLoad.. {}",sub_dll_path.string()); 
        MySub_UnLoad();
        MySub_Load(true);
        //myprint("GHotReload-Loader: ReLoad done."); 
    }

    void MyDetectDllPaths()
    {
        try {
            // Get the current working directory
            std::filesystem::path folder_wd = std::filesystem::current_path(); // C:\game\...\Stock Game
            std::filesystem::path folder_dd_abs = GetDllDirectoryPath(); // C:\game\...\Stock Game\data\SKSE\Plugins
            std::filesystem::path folder_dd_rel = folder_dd_abs.lexically_relative(folder_wd); // data\SKSE\Plugins
            
            //myprint("folder_wd = {}", folder_wd.string());
            //myprint("folder_dd_abs = {}", folder_dd_abs.string());
            //myprint("folder_dd_rel = {}", folder_dd_rel.string());
            
            std::filesystem::path sub_folder = folder_dd_rel / sub_foldername;
            sub_dll_path        = sub_folder / sub_filename;     // data\SKSE\Plugins\sub\GHotReload.dll
            sub_dll_path_tmp    = sub_folder / sub_filename_tmp; // data\SKSE\Plugins\sub\GHotReload-tmp.dll

            //myprint("sub_folder = {}", sub_folder.string());
            //myprint("sub_dll_path = {}", sub_dll_path.string());
            //myprint("sub_dll_path_tmp = {}", sub_dll_path_tmp.string());

        } catch (const std::filesystem::filesystem_error& e) {
            myprint("Filesystem error: {}", std::string(e.what()));
        } catch (const std::exception& e) {
            myprint("Error: {}", std::string(e.what()));
        }
    }
    
    // kDataLoaded
    void OnDataLoaded()
    {
        // NOTE: RE::ConsoleLog::GetSingleton()->Print wont work before kDataLoaded
        MyDetectDllPaths();
        myprint("GHotReload-Loader: DataLoaded -> Load {}",sub_dll_path.string());
        MySub_Load(false);
        //myprint("GHotReload-Loader: DataLoaded done.");
    }
    
    // hotkeys
    void OnButtonDown(RE::ButtonEvent* button)
    {
        auto dxScanCode = button->GetIDCode();
        // myprint("OnButtonDown {}", dxScanCode);
        if (dxScanCode == SCANCODE_reload) MySub_ReLoad();
        if (pFun_OnButtonDown) pFun_OnButtonDown(dxScanCode);
    }

// ***** morphs etc 

    // kPostPostLoad
    #ifdef ENABLE_SKEE
    void myLoadSKEE() { // SKEE racemenu api from see C:\dev\xse\OBody-NG\src\SKEE.h
        myprint("myLoadSKEE...");
        SKEE::InterfaceExchangeMessage msg;
        auto intfc = SKSE::GetMessagingInterface();
        if (!intfc) { myprint("Couldn't get messaging interface!"); return; }
        intfc->Dispatch(SKEE::InterfaceExchangeMessage::kExchangeInterface, (void*)&msg,
                        sizeof(SKEE::InterfaceExchangeMessage*), "skee");
        if (!msg.interfaceMap) {
            myprint("Couldn't get interface map!");
            return;
        }
        myprint("myLoadSKEE got interface map!");

        auto morphInterface =
            static_cast<SKEE::IBodyMorphInterface*>(msg.interfaceMap->QueryInterface("BodyMorph"));
        if (!morphInterface) {
            myprint("Couldn't get serialization MorphInterface!");
            return;
        }

        myprint("BodyMorph Version {}", morphInterface->GetVersion());
        GRegistry::GetSingleton().morphInterface = morphInterface; // SKEE::IBodyMorphInterface*
        myprint("myLoadSKEE done.");
    }
    #endif

// ***** skse events 

    // GetMessagingInterface listener : input=hotkeys, kDataLoaded
    void OnMsgInterfaceMsg (SKSE::MessagingInterface::Message *message)
    {
        if (message->type == SKSE::MessagingInterface::kInputLoaded)
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(this);
        if (message->type == SKSE::MessagingInterface::kDataLoaded) OnDataLoaded();
        if (message->type == SKSE::MessagingInterface::kPostPostLoad) OnPostPostLoad();
    }

    void OnPostPostLoad () // kPostPostLoad
    {
        #ifdef ENABLE_SKEE
        myLoadSKEE();
        #endif
    } 
    
    // InputEvent
	RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>* eSrc)
	{
        RE::BSEventNotifyControl res = RE::BSEventNotifyControl::kContinue;
        if (pFun_OnInputEvent) res = pFun_OnInputEvent(eventPtr,eSrc);
        if (res == RE::BSEventNotifyControl::kContinue && eventPtr)
        {
            auto* e = *eventPtr;
            if (e && e->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) {
                auto* button = e->AsButtonEvent();
                if (button->IsDown()) OnButtonDown(button); // IsDown = IsPressed && HeldDuration=0
            }
        }
		return res;
	}

	RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* e, RE::BSTEventSource<SKSE::CrosshairRefEvent>*)
	{
        RE::TESBoundObject* res = nullptr;
		if (auto o = e->crosshairRef) {
            if (auto* p = o->GetBaseObject()) { // TESBoundObject
                res = p;
                myprint("CrosshairRefEvent GetName={} GetFormID={}",p->GetName(),p->GetFormID());
            }
		}
        GRegistry::GetSingleton().SetLast_CrosshairObject(res);
        if (pFun_OnCrossHairObject) pFun_OnCrossHairObject(res);
		return RE::BSEventNotifyControl::kContinue;
	}
    
// ***** SKSE api 

    // loader skse plugin entry point
    bool OnPluginLoad(const SKSE::LoadInterface *skse)
    {
        myprint_init();
        SKSE::Init(skse);
    	
	    if (auto* src = SKSE::GetCrosshairRefEventSource()) src->AddEventSink(GetEventSink()); else myprint("GetCrosshairRefEventSource=null?0");
	    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *message) {
            gMyLoader.OnMsgInterfaceMsg(message);
        });
        return true;
    }
} gMyLoader;

// loader skse plugin entry point
SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    return gMyLoader.OnPluginLoad(skse);
}
