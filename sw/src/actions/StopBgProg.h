#ifndef STOP_BG_PROG_H
#define STOP_BG_PROG_H

#include <atomic>

// Generic Action
#include "Action.h"

// Programs
#include "ListOfPrograms.h"

/**
 * @brief VERB 7 action: stop a running background program.
 *
 * One-shot: start() captures the target program ID from the HMI noun field
 * (selected by the user via +/- cycling or a typed number before ENTER), and
 * the first periodic tick stops that program (blocking) and finishes the
 * action. If no valid program ID was selected, OP ERROR blinks instead.
 */
class StopBgProg : public Action {

    private:

    /** @brief Registry of the programs this action can stop (owned by HmiLogic). */
    ListOfPrograms &programs_;

    /** @brief Target program ID, captured from the noun field at start(); -1 = none. */
    std::atomic<int> pendingStopId_{-1};

    public:

    /**
     * @param programs[in] Program registry, owned by HmiLogic for the whole
     *                     lifetime of this action.
     * @param data[in] HMI data shared with the display thread.
     */
    StopBgProg(ListOfPrograms &programs, HmiData &data);

    private:

    /** @brief Capture the target program ID from the noun field at start of run. */
    void start() override;

    /**
     * @brief Periodic tick: one-shot stop of the captured program, then finish.
     */
    void operation() override;

};

#endif // STOP_BG_PROG_H
