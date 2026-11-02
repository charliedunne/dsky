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
 * Runs periodically and shows the list of running programs on the left LCD,
 * marked VERB 10 style. +/- cycle the selection with wrap-around; ENTER stops
 * the selected program (blocking) and ends the action. With no programs
 * running the list shows "NONE" and keys are ignored until RSET.
 */
class StopBgProg : public Action {

    private:

    /** @brief Registry of the programs this action can stop (owned by HmiLogic). */
    ListOfPrograms &programs_;

    /** @brief Index into runningIds() of the currently selected line, -1 = none. */
    std::atomic<int> selected_{-1};

    /** @brief Program ID requested for stopping, -1 = none (set by ENTER). */
    std::atomic<int> pendingStopId_{-1};

    public:

    /**
     * @param programs[in] Program registry, owned by HmiLogic for the whole
     *                     lifetime of this action.
     * @param data[in] HMI data shared with the display thread.
     */
    StopBgProg(ListOfPrograms &programs, HmiData &data);

    /**
     * @brief Handle a keypad event while the action is running.
     *
     * +/- cycle the selection through the running programs with wrap-around;
     * ENTER requests the stop of the selected program. All keys are ignored
     * when no program is running.
     *
     * @param key The keypad event to handle.
     * @return true if the key was handled by this action.
     */
    bool handleKey(EventId_t key) override;

    private:

    /** @brief Reset per-run state at the start of every run. */
    void start() override;

    /**
     * @brief Periodic tick: perform a pending stop or refresh the list display.
     */
    void operation() override;

    /** @brief Redraw the running-programs list on the left LCD. */
    void updateScreens();

};

#endif // STOP_BG_PROG_H
