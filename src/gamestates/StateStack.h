#pragma once

#include "IState.h"
#include <memory>
#include <vector>
#include <cassert>

class StateStack
{
public:
    template<typename T>
    bool push()
    {
        std::unique_ptr<IState> pState = std::make_unique<T>(*this);
        bool ok = pState && pState->init();
        if (ok) m_states.push_back(std::move(pState));
        return ok;
    }

    std::unique_ptr<IState> pop()
    {
        std::unique_ptr<IState> pState = std::move(m_states.back());
        m_states.pop_back();
        return pState;
    }

    void popDeferred() { m_popDeferredCount++; }

    IState* getCurrentState() { return m_states.empty() ? nullptr : m_states.back().get(); }

    void performDeferredPops()
    {
        while (m_popDeferredCount-- > 0)
        {
            assert(!m_states.empty());
            if (m_states.empty())
                break;
            m_states.pop_back();
        }
        m_popDeferredCount = 0;
    }

private:
    std::vector<std::unique_ptr<IState>> m_states;
    size_t m_popDeferredCount = 0;
};
