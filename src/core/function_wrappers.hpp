
#ifndef CALLBACK_PTR_HPP
#define CALLBACK_PTR_HPP

#include <functional>
#include <memory>

#include <QPointer>

#include <OpenLibrary/putils/ts_resource.hpp>



// Internal struct with data shared between safe_callback and safe_callback_ctrl
struct safe_callback_data
{
    std::mutex mutex;
    bool callbackAlive;

    safe_callback_data(): mutex(), callbackAlive(true) {}
};

// safe_callback is generated by safe_callback_ctrl.
// it will be a valid functor as long as safe_callback_ctrl exists.
// When safe_callback_ctrl is destroyed, all its safe_callbacks will become
// empty. operator() will do nothing.
// Usefull when calling object's method from another thread
// as we are sure target object exists (or nothing happens)
template<typename... Args>
class safe_callback
{
    public:
        safe_callback(const std::shared_ptr<safe_callback_data>& data, const std::function<void(Args...)>& callback): m_data(data), m_callback(callback) {}
        safe_callback(const safe_callback<Args...> &) = default;

        safe_callback& operator=(const safe_callback<Args...> &) = default;

        virtual ~safe_callback() = default;

        void operator()(Args... args) const
        {
            std::lock_guard<std::mutex> lock(m_data->mutex);

            if (is_valid())
                m_callback(std::forward<Args>(args)...);
        }

        bool is_valid() const
        {
            return m_data->callbackAlive == true && m_callback;
        }

    private:
        std::shared_ptr<safe_callback_data> m_data;

        std::function<void(Args...)> m_callback;
};


// safe_callback_ctrl controls and creates safe_callbacks
class safe_callback_ctrl final
{
    public:
        safe_callback_ctrl(): m_data()
        {
            setup();
        }

        safe_callback_ctrl(const safe_callback_ctrl &) = delete;

        ~safe_callback_ctrl()
        {
            // safe_callback_ctrl should be reset before its destruction.
            //
            // Object owning instance of this class should call invalidate()
            // as a first action in its destructor.
            // Otherwise owner class may be partialy destructed before
            // safe_callback_ctrl is reseted which will allow
            // callbacks to work on a broken object
            assert(m_data.get() == 0 || m_data.use_count() == 1);  // no data, or only one client - us

            reset();
        }

        template<typename... R, typename T>
        auto make_safe_callback(const T& callback) const
        {
            safe_callback<R...> callbackPtr(m_data, callback);

            return callbackPtr;
        }

        safe_callback_ctrl& operator=(const safe_callback_ctrl &) = delete;

        void invalidate()
        {
            reset();      // dissolve all connections
            setup();      // create new one
        }

    private:
        template<typename...>
        friend class safe_callback;

        std::shared_ptr<safe_callback_data> m_data;

        void setup()
        {
            m_data = std::make_shared<safe_callback_data>();
        }

        void reset()
        {
            // mark all safe callbacks as invalid
            {
                // lock resource
                std::lock_guard<std::mutex> lock(m_data->mutex);

                // mark resource as dead
                m_data->callbackAlive = false;
            }

            // detach from existing safe callbacks
            m_data.reset();
        }
};


// extends QMetaObject::invokeMethod by version with arguments
template<typename Obj, typename F, typename... Args>
void invokeMethod(Obj* object, const F& method, Args&&... args)
{
    static_assert(std::is_base_of<QObject, Obj>::value, "Obj must be QObject");
    QMetaObject::invokeMethod(object, [object, method, args...]()
    {
        (object->*method)(args...);
    });
}


// call_from_this_thread uses Qt mechanisms to invoke function in another thread
// (thread of 'object' object)
template<typename F, typename... Args>
void call_from_this_thread(QPointer<QObject> object, const F& function, Args&&... args)
{
    if (object.data() != nullptr)
        QMetaObject::invokeMethod(object.data(), [function, args...]()
        {
            function(args...);
        });
}


// construct a functor which invoked will invoke encapsulated
// functor in another thread
template<typename... Args, typename F>
auto make_cross_thread_function(QObject* object, const F& function)
{
    std::function result = [=](Args&&... args)
    {
        call_from_this_thread(QPointer<QObject>(object), function, std::forward<Args>(args)...);
    };

    return result;
}


// construct a functor which invoked will invoke a method
// (slot) of given object. Will do nothing when given object is destroyed.
// Similar to safe_callback_ctrl (but method will be invoked in target's thread)
template<typename ObjT, typename R, typename ...Args>
auto queued_slot(ObjT* obj, R(ObjT::*method)(Args...))
{
    static_assert(std::is_base_of<QObject, ObjT>::value, "ObjT must be QObject");

    QPointer<ObjT> objPtr(obj);

    return [objPtr, method](Args... args)
    {
        ObjT* object = objPtr.data();

        if (object)
            invokeMethod(object, method, std::forward<Args>(args)...);
    };
}

// construct a functor which invoked will invoke a method
// (slot) of given object. Will do nothing when given object is destroyed.
// Similar to safe_callback_ctrl (but uses Qt mechanism to guarantee (?) threadsafety)
// Instead of queued_slot() method is invoked in caller's thread
template<typename ObjT, typename R, typename ...Args>
std::function<void(Args...)> direct_slot(ObjT* obj, R(ObjT::*method)(Args...))
{
    static_assert(std::is_base_of<QObject, ObjT>::value, "ObjT must be QObject");

    QPointer<ObjT> objPtr(obj);

    return [objPtr, method](Args... args)
    {
        ObjT* object = objPtr.data();

        if (object)
            (object->*method)(std::forward<Args>(args)...);
    };
}

#endif
