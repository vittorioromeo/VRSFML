// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Android/Activity.hpp"

#include "ZancleBase/Assert.hpp"

#include <android/log.h>


#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_INFO, "zancle-error", __VA_ARGS__))

std::streambuf::int_type LogcatStream::overflow(std::streambuf::int_type c)
{
    if (c == '\n')
    {
        m_message.push_back(static_cast<char>(c));
        LOGE("%s", m_message.c_str());
        m_message.clear();
    }

    m_message.push_back(static_cast<char>(c));

    return traits_type::not_eof(c);
}

namespace za::priv
{

ActivityStates*& getActivityStatesPtr()
{
    static ActivityStates* states = nullptr;
    return states;
}

void resetActivity(ActivityStates* initializedStates)
{
    getActivityStatesPtr() = initializedStates;
}

ActivityStates& getActivity()
{
    ActivityStates* const states = getActivityStatesPtr();
    ZB_ASSERT(states != nullptr &&
              "Cannot dereference null activity states pointer. Call priv::resetActivity() to initialize it.");

    return *states;
}

} // namespace za::priv
