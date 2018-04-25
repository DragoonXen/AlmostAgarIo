#include "../nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include "Constants.h"
#include "Context.h"
#include "Solver.h"
#include "SimpleLog.h"
#include <chrono>

#ifdef REWIND_VIEWER

#include "../rewind/RewindClient.h"

#endif

#include "ExtContext.h"
#include "models/simulation/FoodDetection.h"

using json = nlohmann::json;

int main(int argc, char *argv[]) {
    srand(270718);
#ifdef LOCAL_RUN
    std::fstream file;
    if (argc == 1) {
        file.open("data.json", std::ios::out);
    } else {
        file.open(argv[1], std::ios::in);
    }
    std::vector<std::pair<ExtContext, std::string>> contexts;
#endif
    {
        std::string params;
#ifdef LOCAL_RUN
        if (argc == 1) {
            std::cin >> params;
            file << params << std::endl;
        } else {
            file >> params;
        }
#else
        std::cin >> params;
#endif
        auto parsed = json::parse(params);
//        LOG_DEBUG("%", parsed.dump());
        Constants::initConstants(parsed);
    }

    std::string data;
    ExtContext context(0);
    for (auto &player : context.players) {
        player.aimX = NAN;
    }
    Solver solver;
    int micros = 0;
    int totalMicros = 0;
    int totalTlPrevention = 0;
    while (context.getTickIndex() < Constants::GAME_TICKS()) {
        LOG_DEBUG("%", context.getTickIndex())
#ifdef LOCAL_RUN
        if (argc == 1) {
            std::cin >> data;
            file << data << std::endl;
        } else {
            file >> data;
            if (file.eof()) {
                break;
            }
        }
        LOG_INFO("%", data);
        contexts.emplace_back(context, data);
#else
        std::cin >> data;
#endif
        using std::chrono::system_clock;
        system_clock::time_point start = system_clock::now();
        auto parsed = json::parse(data);
        auto command = solver.onTick(context, parsed);
#ifdef REWIND_VIEWER
        RewindClient &instance = RewindClient::instance();
        double cX = command.getX();
        double cY = command.getY();
        for (auto &player : context.me->parts) {
            instance.line(player.getX(), player.getY(), cX, cY, 0xFF8800, 3);
        }
        instance.line(cX + 5, cY + 5, cX - 5, cY - 5, 0xFF8800, 3);
        instance.line(cX - 5, cY + 5, cX + 5, cY - 5, 0xFF8800, 3);
#ifdef DRAW_PP
        // draw food field
        for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
            for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                double color = std::min(FoodDetection::foodField[i][j] * 20., 1.);
                auto green = (uint32_t) (255 * color + .01);
                green <<= 8; // green channel
                instance.rect(i * Constants::COORDS_IN_ONE_FOOD_FIELD,
                              j * Constants::COORDS_IN_ONE_FOOD_FIELD,
                              (i + 1) * Constants::COORDS_IN_ONE_FOOD_FIELD,
                              (j + 1) * Constants::COORDS_IN_ONE_FOOD_FIELD,
                              green,
                              1);

            }
        }
#endif
        instance.end_frame();
#endif
        system_clock::time_point end = system_clock::now();

        command.appendDebug(" time: " + std::to_string(micros));
        if (context.tlPrevention > 0) {
            totalTlPrevention += context.tlPrevention;
            command.appendDebug(" tlPrevention: " + std::to_string(context.tlPrevention) + " totalTlPrevention: " + std::to_string(totalTlPrevention));
        } else if (context.disableAddPoints) {
            command.appendDebug(" add points disabled");
        }
        if (context.me->parts.empty() || context.getTickIndex() + 1 == Constants::GAME_TICKS()) {
            totalMicros += micros;
            micros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            totalMicros += micros;
            command.appendDebug(" dead. avg time: " + std::to_string(((double) totalMicros) / (context.getTickIndex() + 1)));
            if (context.tlPrevention <= 0) {
                command.appendDebug(" totalTlPrevention: " + std::to_string(totalTlPrevention));
            }
            std::cout << command.toJson().dump() << std::endl;
            break;
        }

        std::cout << command.toJson().dump() << std::endl;
#ifdef PERF
        std::cout << context.getTickIndex() << std::endl;
#endif
        micros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        totalMicros += micros;
        context.totalLength = totalMicros;
        context.turnTime.push_back(micros);
        LOG_DEBUG("%", command.toJson().dump())
        context.nextTick(command.getX(), command.getY());
    }


#ifdef LOCAL_RUN
#ifndef PERF
    file.close();
    std::cout << Command(0, 0, "dead. avg time: " + std::to_string(((double) totalMicros) / (context.getTickIndex() + 1))).toJson().dump() << std::endl;
    int idxToDebug;
    while (true) {
        std::cout << "type index of debug tick (exit if negative)" << std::endl;
        std::cin >> idxToDebug;
        if (idxToDebug < 0) {
            break;
        }
        ExtContext contextCopy = contexts[idxToDebug].first;
        auto dataCopy = json::parse(contexts[idxToDebug].second);
        auto command = solver.onTick(contextCopy, dataCopy);

        std::cout << command.toJson().dump() << std::endl;
#ifdef REWIND_VIEWER
        RewindClient &instance = RewindClient::instance();
        double cX = command.getX();
        double cY = command.getY();
        for (auto &player : contextCopy.me->parts) {
            instance.line(player.getX(), player.getY(), cX, cY, 0xFF8800, 3);
        }
        instance.line(cX + 5, cY + 5, cX - 5, cY - 5, 0xFF8800, 3);
        instance.line(cX - 5, cY + 5, cX + 5, cY - 5, 0xFF8800, 3);

        instance.end_frame();
#endif
    }
#endif
#endif
    return 0;
}