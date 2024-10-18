module;

// #include "SKEE.h"

export module mymodule;
import shared_util;
import logger;
import gregistry;

struct OurEventSinkBase {};

export class cMyModule :
    OurEventSinkBase
{
    public:
    //SKEE::IBodyMorphInterface* morphInterface = nullptr;
    
    static cMyModule& GetSingleton() {
        static cMyModule singleton;
        return singleton;
    }
    cMyModule* GetEventSink() { return this; }

// ***** api

    void init(bool bReload) {
        myprint("cMyModule init bReload={}",bReload);
        //morphInterface = GRegistry::GetSingleton().morphInterface;
        //if (morphInterface) myprint("g.BodyMorph Version {}", morphInterface->GetVersion()); else myprint("BodyMorph=null"); // kPostPostLoad
     }
    
    void cleanup() {
        myprint("cMyModule cleanup");	    
    }

    void debugHotkey1(RE::TESBoundObject* p) {
        myprint("cMyModule debugHotkey1 {}",p ? p->GetName() : "null");
        if (!p) return;
        RE::TESForm& a = *p;
        RE::TESNPC* npc = a.As<RE::TESNPC>();
        myprint("GetFormType={} {} AsNPC={}",(int)a.GetFormType(),RE::FormTypeToString(a.GetFormType()),npc?"ok":"null"); // 43 NPC_
        if (npc) handleNPC(*npc);
    }

    void handleNPC(RE::TESNPC& o)
    {
        using namespace RE;
        myprint("handleNPC {}",o.GetName()); // Bandit Plunderer
		RE::TESRace* r = o.GetRace(); 
        myprint("{} {}",o.IsFemale()?"female":"male",r?r->GetName():"null"); // Redguard
    }

// ***** rest

};
