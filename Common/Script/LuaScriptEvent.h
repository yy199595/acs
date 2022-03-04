
namespace LuaScriptEvent
{
    template<typename... Args>
    class ScriptEvent
    {
    public:
        typedef std::function<void(Args...)> EventCallback;

        static void Add(EventCallback callback)
        {
            if (callback)
            {
                callbackList.push_back(callback);
            }
        }

        static bool Invoke(Args... args)
        {
            if (callbackList.size() == 0)
            {
                return false;
            }
            for (size_t index = 0; index < callbackList.size(); index++)
            {
                EventCallback &callback = callbackList.at(index);
                callback(std::forward<Args>(args)...);
            }
            return true;
        }

    private:
        static std::vector<EventCallback> callbackList;
    };

    template<typename... Args>
    typename std::vector<std::function<void(Args...)>>
            ScriptEvent<Args...>::callbackList;

#define DEF_SCRIPT_EVENT(EventName, ...)              \
    class EventName : public ScriptEvent<__VA_ARGS__> \
    {                                                 \
    };

#define PUSH_LUA_SCRIPT_EVENT(lua, scriptEvent, name)                                    \
    {                                                                                    \
        ClassProxyHelper<scriptEvent> helper = ClassProxyHelper<scriptEvent>(lua, name); \
        helper.PushStaticFunction("Save", scriptEvent::Add);                              \
        helper.PushStaticFunction("OnResponse", scriptEvent::Invoke);                        \
    }

#define PUSH_LUA_SCRIPT_EVENT(lua, scriptEvent)                                                  \
    {                                                                                            \
        ClassProxyHelper<scriptEvent> helper = ClassProxyHelper<scriptEvent>(lua, #scriptEvent); \
        helper.PushStaticFunction("Save", scriptEvent::Add);                                      \
        helper.PushStaticFunction("OnResponse", scriptEvent::Invoke);                                \
    }
}// namespace LuaScriptEvent
