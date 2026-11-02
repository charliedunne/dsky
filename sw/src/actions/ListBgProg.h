#ifndef LIST_BG_PROG_H
#define LIST_BG_PROG_H

// Generic Action
#include "Action.h"

// Programs
#include "ListOfPrograms.h"

/**
 * @brief VERB 6 action: list currently running background programs on the left LCD.
 *
 * Verb-only action (noun -1, no typed noun required). The list persists on the
 * left LCD until the user presses any key, at which point the idle() path
 * clears it via clearVerbList().
 */
class ListBgProg : public Action {

    private:

    /** @brief Registry of the programs to inspect (owned by HmiLogic). */
    ListOfPrograms &programs_;

    public:

    /**
     * @param programs[in] Program registry, owned by HmiLogic for the whole
     *                     lifetime of this action.
     * @param data[in] HMI data used to display the list on the left LCD.
     */
    ListBgProg(ListOfPrograms &programs, HmiData &data);

    private:

    /**
     * @brief Build the list of running programs and display it on the left LCD.
     */
    void operation() override;

};

#endif /* LIST_BG_PROG_H */
