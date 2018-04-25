//
// Created by dragoon on 4/1/18.
//

#ifndef ALMOSTAGARIO_SIMULATOR_H
#define ALMOSTAGARIO_SIMULATOR_H


#include "../../Context.h"
#include "SimCommand.h"
#include "../../ExtContext.h"
#include "SimulateType.h"

class Simulator {
private:
    static double simulate(Context context, const std::vector<std::pair<SimCommand, int>> &commandList, int totalTicks, bool cleanDetect);

public:
    static void externalSimulateTick(Context &context, double aimX, double aimY);

    static std::pair<double, SimulateType>
    simulateProxy(const ExtContext &context,
                  const std::vector<std::pair<SimCommand, int>> &commandList,
                  int totalTicks,
                  Context const *const maxTtfContext,
                  const double currMax);

#ifdef REWIND_VIEWER
    public:
        static double debugSimulateProxy(const ExtContext &context,
                                         const std::vector<std::pair<SimCommand, int>> &commandList,
                                         int totalTicks,
                                         Context const *const maxTtfContext,
                                         SimulateType simulateType);

    private:
        static double debugSimulate(Context context, const std::vector<std::pair<SimCommand, int>> &commandList, int totalTicks, bool cleanDetect);

#endif
};


#endif //ALMOSTAGARIO_SIMULATOR_H
