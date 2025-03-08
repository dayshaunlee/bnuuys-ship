#pragma once

/*
*   We create a decision tree node with a custom context. 
*   This context allows you to pass any struct or any data type.
*   That way you save time from computing the same thing over and over.
*/
#include <functional>
#include <memory>

#include <memory>
#include <functional>

template <typename Context>
class DecisionNode {
private:
    std::function<bool(Context&)> condition;
    std::shared_ptr<DecisionNode<Context>> yesBranch;
    std::shared_ptr<DecisionNode<Context>> noBranch;
    std::function<void(Context&)> performAction;

public:
    DecisionNode(std::function<bool(Context&)> cond)
        : condition(cond), performAction(nullptr), yesBranch(nullptr), noBranch(nullptr) {}

    void setBranches(std::shared_ptr<DecisionNode<Context>> yes, std::shared_ptr<DecisionNode<Context>> no) {
        yesBranch = yes;
        noBranch = no;
    }

    void setAction(std::function<void(Context&)> action) {
        performAction = action;
    }

    std::shared_ptr<DecisionNode<Context>> decide(Context& ctx) {
        if (performAction)
            execute(ctx);
        return condition(ctx) ? yesBranch : noBranch;
    }

    void execute(Context& ctx) {
        if (performAction) {
            performAction(ctx);
        }
    }
};
