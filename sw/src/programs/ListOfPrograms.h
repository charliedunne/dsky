#ifndef _LISTOFPROGRAMS_H_
#define _LISTOFPROGRAMS_H_

// Generic Program
#include "Program.h"

// STL
#include <vector>
#include <string>

/**
 * @brief Command registry/dispatcher mapping a program id to a registered Program.
 *
 * @note Intentionally NOT a Program subclass: this container is never executed
 *       itself, it only performs cheap stateless lookups for the HMI loop, so
 *       it has no id of its own and no thread machinery. Programs are launched
 *       from the HMI with the PRO key followed by a program id; unlike Actions
 *       there is no (verb, noun) pair to resolve, only a single id.
 */
class ListOfPrograms {

    private:

    /**
     * @brief Array of programs
     */
    std::vector<std::reference_wrapper<Program>> programs_;

    public:

    ListOfPrograms();
    virtual ~ListOfPrograms();

    /**
     * @brief Register a new program in the list
     *
     * @param program[in] Reference to the program to register
     */
    void registerProgram(Program &program);

    /**
     * @brief Overload of operator() to make a functor.
     *
     * Resolves the registered program by its id.
     *
     * @param id[in] Program id selected on the keypad
     *
     * @return Reference to the matching registered Program
     * @throws ExcInvalidCommand If no program matches the given id
     */
    Program & operator()(int id);

    /**
     * @brief Human-readable description of every registered program,
     *        sorted by id ascending. Used for tracing / diagnostics.
     *
     * @return One string per program, e.g. "P01 DUMMY"
     */
    std::vector<std::string> describe() const;

    /**
     * @brief Stop every registered program.
     *
     * Called during shutdown so that no background program keeps running while
     * the program objects are being destroyed (a late virtual call of
     * operation() on a half-destroyed object would hit a pure-virtual
     * terminator and abort). Safe on programs that were never started.
     */
    void stopAll();

};

#endif /* _LISTOFPROGRAMS_H_ */
