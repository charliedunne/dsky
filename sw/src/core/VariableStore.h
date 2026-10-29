#ifndef _VARIABLESTORE_H_
#define _VARIABLESTORE_H_

#include <functional>
#include <map>
#include <string>

/**
 * @brief Data type of a persisted variable.
 */
typedef enum
{

    VAR_BOOL,
    VAR_INT,

} VarType_e;

class VariableStore
{
public:
    /**
     * @brief Callback invoked after a variable value has been committed in memory.
     *        Used to propagate the new value to hardware (e.g. message queue).
     */
    using ApplyFn = std::function<void(int)>;

private:
    struct Var
    {
        int noun;         ///< NOUN selector for VERB 8/9
        std::string name; ///< Key used in the JSON persistence file
        VarType_e type;   ///< Data type (bools are strictly validated as 0/1)
        int defaultValue; ///< Value used when no persisted value exists
        ApplyFn apply;    ///< Optional hardware side effect, may be null
        int value;        ///< Current in-memory value
    };

    std::map<int, Var> vars_; ///< Registered variables, keyed by NOUN
    std::string filePath_;    ///< Persistence file path

public:
    /**
     * @brief Construct the store bound to a persistence file.
     * @param filePath Path of the JSON file holding persisted values
     */
    VariableStore(const std::string &filePath);
    virtual ~VariableStore();

    /**
     * @brief Register a variable in the store.
     * @param noun NOUN value selecting this variable under VERB 8/9
     * @param name Key used in the JSON persistence file
     * @param type Data type (bools are strictly validated as 0/1)
     * @param defaultValue Value used when no persisted value exists
     * @param apply Optional callback invoked on every set() with the new value
     */
    void registerVar(int noun, const std::string &name, VarType_e type, int defaultValue, ApplyFn apply = nullptr);

    /**
     * @brief Load persisted values from the JSON file.
     *        Missing or malformed file: keep defaults and log a warning.
     *        Unknown keys are ignored; missing keys get their default.
     */
    void load();

    /**
     * @brief Persist all current values to the JSON file (write-through).
     * @return true on success, false if the file could not be written or renamed
     */
    bool save();

    /**
     * @brief Get the current value of a variable.
     * @param noun NOUN selector for the variable
     * @param value Output: current value on success
     * @return true if the NOUN is registered, false otherwise
     */
    bool get(int noun, int &value) const;

    /**
     * @brief Set a variable value with validation, write-through persistence and apply callback.
     *        Bool variables accept strictly 0 or 1.
     * @param noun NOUN selector for the variable
     * @param value New value to commit
     * @return true on success, false if the NOUN is unknown or the value is invalid
     */
    bool set(int noun, int value);

    /**
     * @brief Query whether a variable is of bool type.
     * @param noun NOUN selector for the variable
     * @return true if registered and of bool type, false otherwise
     */
    bool isBool(int noun) const;

    /**
     * @brief Re-apply all current values to hardware (used at startup).
     */
    void applyAll();
};

#endif /* _VARIABLESTORE_H_ */
