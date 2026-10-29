#include "VariableStore.h"

// STL
#include <fstream>
#include <cctype>
#include <cstdio>

// Logging
#include "Logger.h"

VariableStore::VariableStore(const std::string &filePath)
{
    filePath_ = filePath;
}

VariableStore::~VariableStore() {}

void VariableStore::registerVar(int noun, const std::string &name, VarType_e type, int defaultValue, ApplyFn apply)
{
    Var var;
    var.noun = noun;
    var.name = name;
    var.type = type;
    var.defaultValue = defaultValue;
    var.apply = std::move(apply);
    var.value = defaultValue;
    vars_[noun] = var;

    LogDebug << "VariableStore: registered NOUN " << noun << " -> " << name << std::endl;
}

bool VariableStore::get(int noun, int &value) const
{
    auto it = vars_.find(noun);
    if (it == vars_.end())
        return false;

    value = it->second.value;
    return true;
}

bool VariableStore::set(int noun, int value)
{
    auto it = vars_.find(noun);
    if (it == vars_.end())
    {
        LogWarning << "VariableStore: unknown NOUN " << noun << std::endl;
        return false;
    }

    Var &var = it->second;

    // Bool variables accept strictly 0 or 1
    if (var.type == VAR_BOOL && value != 0 && value != 1)
    {
        LogWarning << "VariableStore: invalid bool value " << value << " for " << var.name << std::endl;
        return false;
    }

    var.value = value;

    // A persistence failure must be visible: the in-memory value and hardware
    // side effect still take effect, but the change will NOT survive a reboot.
    if (!save())
        LogError << "VariableStore: persistence failed for " << var.name << ", value will not survive reboot" << std::endl;

    if (var.apply)
        var.apply(value);

    LogDebug << "VariableStore: " << var.name << " = " << value << std::endl;
    return true;
}

bool VariableStore::isBool(int noun) const
{
    auto it = vars_.find(noun);
    if (it == vars_.end())
        return false;

    return it->second.type == VAR_BOOL;
}

void VariableStore::applyAll()
{
    for (const auto &kv : vars_)
    {
        if (kv.second.apply)
            kv.second.apply(kv.second.value);
    }
}

bool VariableStore::save()
{
    std::string tmp = filePath_ + ".tmp";
    std::ofstream out(tmp);
    if (!out.is_open())
    {
        LogError << "VariableStore: cannot open " << tmp << " for writing" << std::endl;
        return false;
    }

    out << "{\n";
    bool first = true;
    for (const auto &kv : vars_)
    {
        if (!first)
            out << ",\n";
        first = false;
        out << "  \"" << kv.second.name << "\": " << kv.second.value;
    }
    out << "\n}\n";
    out.close();

    if (out.fail())
    {
        LogError << "VariableStore: write failed for " << tmp << std::endl;
        return false;
    }

    // Atomic replace so a crash mid-write cannot corrupt the file
    if (::rename(tmp.c_str(), filePath_.c_str()) != 0)
    {
        LogError << "VariableStore: rename to " << filePath_ << " failed" << std::endl;
        return false;
    }

    return true;
}

void VariableStore::load()
{
    std::ifstream in(filePath_);
    if (!in.is_open())
    {
        LogWarning << "VariableStore: cannot open " << filePath_ << ", using defaults" << std::endl;
        return;
    }

    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    // Minimal flat-JSON scanner: sequence of "key": <int> pairs.
    // Unknown keys are ignored; missing keys keep their default.
    int applied = 0;
    size_t pos = 0;
    while (true)
    {
        size_t q1 = content.find('"', pos);
        if (q1 == std::string::npos)
            break;
        size_t q2 = content.find('"', q1 + 1);
        if (q2 == std::string::npos)
            break;
        std::string key = content.substr(q1 + 1, q2 - q1 - 1);

        size_t colon = content.find(':', q2 + 1);
        if (colon == std::string::npos)
            break;

        // Parse value: optional '-' followed by digits
        size_t vpos = colon + 1;
        while (vpos < content.size() && (content[vpos] == ' ' || content[vpos] == '\t'))
            vpos++;
        bool neg = false;
        if (vpos < content.size() && content[vpos] == '-')
        {
            neg = true;
            vpos++;
        }
        long val = 0;
        bool any = false;
        while (vpos < content.size() && isdigit((unsigned char)content[vpos]))
        {
            val = val * 10 + (content[vpos] - '0');
            any = true;
            vpos++;
        }

        pos = vpos; // continue scanning after the value
        if (!any)
            continue; // malformed entry, skip it

        int ivalue = neg ? -(int)val : (int)val;

        // Look up by name (map is keyed by NOUN)
        for (auto &kv : vars_)
        {
            if (kv.second.name != key)
                continue;

            if (kv.second.type == VAR_BOOL && ivalue != 0 && ivalue != 1)
            {
                LogWarning << "VariableStore: invalid persisted bool value " << ivalue << " for " << key << ", using default" << std::endl;
                break;
            }

            kv.second.value = ivalue;
            applied++;
            LogDebug << "VariableStore: loaded " << key << " = " << ivalue << std::endl;
            break;
        }
    }

    if (applied == 0)
        LogWarning << "VariableStore: no valid entries found in " << filePath_ << ", using defaults" << std::endl;
}
