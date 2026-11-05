#include "ListOfPrograms.h"

// STL
#include <stdexcept>
#include <algorithm>   // std::sort
#include <cstdio>      // std::snprintf

// Exceptions
#include "ExcInvalidCommand.h"

ListOfPrograms::ListOfPrograms() {}
ListOfPrograms::~ListOfPrograms() {}

void ListOfPrograms::registerProgram(Program &program) {
    programs_.push_back(program);
}

Program & ListOfPrograms::operator()(int id)
{
    /* Single pass: resolve the registered program by its id. */
    for (int i = 0; i < programs_.size(); ++i)
    {
        if (programs_[i].operator Program &().getId() == id)
        {
            return programs_[i];
        }
    }

    /* In case of not find the program required rise an exception */
    throw ExcInvalidCommand(id, -1);
}

std::vector<std::string> ListOfPrograms::describe() const {

    // Build a sortable list of pointers to the registered programs
    std::vector<const Program*> sorted;
    for (const auto &it : programs_) {
        sorted.push_back(&it.get());
    }

    // Sort by id ascending so the listing is deterministic
    std::sort(sorted.begin(), sorted.end(),
              [](const Program *a, const Program *b) {
                  return a->getId() < b->getId();
              });

    // Format entries as "Pxx KIND LABEL" (one line per program, kind = BG or FG)
    std::vector<std::string> out;
    for (const auto *p : sorted) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "P%02d %s %s", p->getId(),
                      p->foreground() ? "FG" : "BG", p->getLabel().c_str());
        out.push_back(std::string(buf));
    }

    return out;
}

std::vector<std::string> ListOfPrograms::describeRunning() const {

    std::vector<std::string> out;

    // Build a sortable list of pointers to the running programs only
    std::vector<const Program*> sorted;
    for (const auto &it : programs_) {
        if (it.get().isRunning()) {
            sorted.push_back(&it.get());
        }
    }

    // Sort by id ascending so the listing is deterministic
    std::sort(sorted.begin(), sorted.end(),
              [](const Program *a, const Program *b) {
                  return a->getId() < b->getId();
              });

    if (sorted.empty()) {
        out.push_back("NONE");
        return out;
    }

    // Format entries as "Pxx LABEL" (one line per running program)
    for (const auto *p : sorted) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "P%02d %s", p->getId(), p->getLabel().c_str());
        out.push_back(std::string(buf));
    }

    return out;
}

std::vector<int> ListOfPrograms::runningIds() const {

    std::vector<int> ids;

    for (const auto &it : programs_) {
        if (it.get().isRunning()) {
            ids.push_back(it.get().getId());
        }
    }

    // Sort by id ascending to match the order of describeRunning()
    std::sort(ids.begin(), ids.end());

    return ids;
}

void ListOfPrograms::stopAll() {
    for (const auto &it : programs_) {
        it.get().stop();
    }
}
