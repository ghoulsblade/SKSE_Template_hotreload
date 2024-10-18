module;

// #include "SKEE.h" // racemenu morphinterface

export module gregistry;

export class GRegistry {
    public:
    static GRegistry* singleton;
    static GRegistry& GetSingleton() { return *singleton; }
    static void SetSingleton(GRegistry* v) { singleton = v; }

    enum class RegEnum {
        kLast_CrosshairObject,
    };
    //using enum RegEnum; // Bring enumerators into class scope
    
    //SKEE::IBodyMorphInterface* morphInterface = nullptr;

    using tValue = std::variant<int, float, std::string, RE::FormID>;
    std::map<std::string, tValue> data;
    std::map<RegEnum, tValue> edata;
    tValue& operator[](const std::string& key) { return data[key]; }
    tValue& operator[](const RegEnum key) { return edata[key]; }
    bool contains(const std::string& key) { return data.contains(key); }
    bool contains(const RegEnum key) { return edata.contains(key); }

    RE::TESBoundObject* GetLast_CrosshairObject() { return LookUpFormID<RE::TESBoundObject>(RegEnum::kLast_CrosshairObject); }
    void SetLast_CrosshairObject (RE::TESBoundObject* p)
    {
        auto k=GRegistry::RegEnum::kLast_CrosshairObject;
        if (p) (*this)[k] = p->GetFormID(); else edata.erase(k);
    }

    template <typename T>
    T* LookUpFormID(RegEnum k)
    {
        auto it = edata.find(k);
        if (it == edata.end()) return nullptr;
        RE::FormID fid = std::get<RE::FormID>(it->second);
        return static_cast<T*>(RE::TESForm::LookupByID(fid));
    }
};

// Initialize the static member
GRegistry* GRegistry::singleton = nullptr;
