module;

export module plugin_main;
import mypch; // Import the new module that includes the headers
import shared_util;
import gregistry;
import mymodule;

// ***** main api

#define PLUGIN_LOADER_API extern "C" __declspec(dllexport)

// Load
PLUGIN_LOADER_API void OnLoadPlugin(GRegistry* registry,bool bReload)
{
    myprint_init(true);
    myprint("OnLoadPlugin bReload={}",bReload);
    GRegistry::SetSingleton(registry);
    cMyModule::GetSingleton().init(bReload);
    cMyModule::GetSingleton().debugHotkey1(GRegistry::GetSingleton().GetLast_CrosshairObject());
}

// unload, before reload
PLUGIN_LOADER_API void OnUnLoadPlugin()
{
    myprint("OnUnLoadPlugin");
    cMyModule::GetSingleton().cleanup();
}

// hotkeys
PLUGIN_LOADER_API void OnButtonDown(uint32_t scancode)
{
    if (scancode == 59) cMyModule::GetSingleton().debugHotkey1(GRegistry::GetSingleton().GetLast_CrosshairObject()); // f1
    if (!std::ranges::contains(std::array{8, 9, 201, 209}, scancode)) { // mousewheel and pageup/down
        // myprint("OnButtonDown {}", scancode);
    }
}

// crosshair

PLUGIN_LOADER_API void OnCrossHairObject (RE::TESBoundObject* p)
{
    if (p) myprint("g.OnCrossHairObject GetName={} GetFormID={}",p->GetName(),p->GetFormID()); else myprint("g.OnCrossHairObject null");
    // NOTE: GetFormEditorID,GetObjectTypeName empty for bandit and skeleton
}

// input generic
PLUGIN_LOADER_API RE::BSEventNotifyControl OnInputEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>* eSrc)
{
    // if (*eventPtr && (*eventPtr)->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) myprint("OnInputEvent kButton");
    // kButton,kMouseMove,kChar,kThumbstick,kDeviceConnect,kKinect..
	return RE::BSEventNotifyControl::kContinue;
}
