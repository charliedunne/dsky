#ifndef STOP_BG_PROG_H
#define STOP_BG_PROG_H

// Generic Action
#include "Action.h"

// Programs
#include "ListOfPrograms.h"

/**
 * @brief VERB 7 action: stop a running background program by its ID.
 *
 * Registered with noun -1 so the dispatcher's verb-only pass resolves it for
 * any typed N; the target program ID is read from the HMI data (nNoun) at
 * execution time. The stop is blocking (joins the program thread) but runs on
 * the action worker thread, so the HMI loop keeps running.
 */
class StopBgProg : public Action {

    private:

    /** @brief Registry of the programs this action can stop (owned by HmiLogic). */
    ListOfPrograms &programs_;

    public:

    /**
     * @param programs[in] Program registry, owned by HmiLogic for the whole
     *                     lifetime of this action.
     * @param data[in] HMI data; the target program ID is read from nNoun.
     */
    StopBgProg(ListOfPrograms &programs, HmiData &data);

    private:

    /**
     * @brief Resolve the program ID from nNoun and stop it if running.
     *
     * Gives LCD feedback for an unknown ID or a program that is not running;
     * otherwise blocks until the program thread has joined.
     */
    void operation() override;

};

#endif /* STOP_BG_PROG_H */
