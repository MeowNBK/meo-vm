#include "meow_vm.h"
#include "operator_dispatcher.h"
#include "common/pch.h"

void MeowVM::opBinary() {
    auto proto = currentFrame->closure->proto;
    auto currentInst = &proto->code[(currentFrame->ip) - 1];

    Int dst = currentInst->args[0],
        r1 = currentInst->args[1],
        r2 = currentInst->args[2];

    auto& left = stackSlots[currentBase + r1];
    auto& right = stackSlots[currentBase + r2];

    Value result;
    if (auto func = opDispatcher.find(currentInst->op, left, right)) {
        result = (*func)(left, right);
    } else {
        throwVMError("Unsupported binary operator");
    }

    stackSlots[currentBase + dst] = result;
}

void MeowVM::opUnary() {
    auto proto = currentFrame->closure->proto;
    auto inst = &proto->code[(currentFrame->ip) - 1];

    Int dst = currentInst->args[0], src = currentInst->args[1];
    auto& val = stackSlots[currentBase + src];

    Value result;
    if (auto func = opDispatcher.find(inst->op, val)) {
        result = (*func)(val);
    } else {
        std::ostringstream os;
        throwVMError("Unsupported unary operator");
    }

    stackSlots[currentBase + dst] = result;
}