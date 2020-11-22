/*
 * peripheral/computer.cpp
 * CraftOS-PC 2
 * 
 * This file defines the methods for the computer peripheral.
 * 
 * This code is licensed under the MIT License.
 * Copyright (c) 2019-2020 JackMacWindows.
 */

#define CRAFTOSPC_INTERNAL
#include <cstring>
#include <regex>
#include "computer.hpp"
#include "../runtime.hpp"

int computer::turnOn(lua_State *L) {return 0;}

int computer::isOn(lua_State *L) {
    lua_pushboolean(L, freedComputers.find(comp) == freedComputers.end());
    return 1;
}

int computer::getID(lua_State *L) {
    if (freedComputers.find(comp) != freedComputers.end()) return 0;
    lua_pushinteger(L, comp->id);
    return 1;
}

int computer::shutdown(lua_State *L) {
    if (freedComputers.find(comp) != freedComputers.end()) return 0;
    comp->running = 0;
    return 0;
}

int computer::reboot(lua_State *L) {
    if (freedComputers.find(comp) != freedComputers.end()) return 0;
    comp->running = 2;
    return 0;
}

int computer::getLabel(lua_State *L) {
    if (freedComputers.find(comp) != freedComputers.end()) return 0;
    lua_pushlstring(L, comp->config->label.c_str(), comp->config->label.length());
    return 1;
}

computer::computer(lua_State *L, const char * side) {
    if (strlen(side) < 10 || std::string(side).substr(0, 9) != "computer_" || (strlen(side) > 9 && !std::all_of(side + 9, side + strlen(side), ::isdigit))) 
        throw std::invalid_argument("\"side\" parameter must be a number (the computer's ID)");
    int id = atoi(&side[9]);
    comp = NULL;
    {
        LockGuard lock(computers);
        for (Computer * c : *computers) if (c->id == id) comp = c;
    }
    if (comp == NULL) comp = (Computer*)queueTask([ ](void* arg)->void*{return startComputer(*(int*)arg);}, &id);
    if (comp == NULL) throw std::runtime_error("Failed to open computer");
    thiscomp = get_comp(L);
    comp->referencers.push_back(thiscomp);
}

computer::~computer() {
    if (thiscomp->peripherals_mutex.try_lock()) thiscomp->peripherals_mutex.unlock();
    else return;
    for (auto it = comp->referencers.begin(); it != comp->referencers.end(); ++it) {
        if (*it == thiscomp) {
            it = comp->referencers.erase(it);
            if (it == comp->referencers.end()) break;
        }
    }
}

int computer::call(lua_State *L, const char * method) {
    const std::string m(method);
    if (m == "turnOn") return turnOn(L);
    else if (m == "shutdown") return shutdown(L);
    else if (m == "reboot") return reboot(L);
    else if (m == "getID") return getID(L);
    else if (m == "isOn") return isOn(L);
    else if (m == "getLabel") return getLabel(L);
    else return 0;
}

static luaL_Reg computer_reg[] = {
    {"turnOn", NULL},
    {"shutdown", NULL},
    {"reboot", NULL},
    {"getID", NULL},
    {"isOn", NULL},
    {"getLabel", NULL},
    {NULL, NULL}
};

library_t computer::methods = {"computer", computer_reg, nullptr, nullptr};