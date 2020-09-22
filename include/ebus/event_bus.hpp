#pragma once

#include <list>
#include <typeinfo>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <stdexcept>

namespace ebus {
    class Object {
    public:
        Object() { }

        virtual ~Object() { }

        Object(const Object& other) { }
    };

    class Event : public Object {
    public:
        Event(Object& sender)
                :
                sender(sender),
                canceled(false)
        {
        }

        virtual ~Event() { }

        Object& getSender()
        {
            return sender;
        }

        bool getCanceled()
        {
            return canceled;
        }

        void setCanceled(bool canceled)
        {
            this->canceled = canceled;
        }

    private:
        Object& sender;
        bool canceled;
    };

    template<class T>
    class EventHandler {
    public:
        EventHandler()
        {
            static_assert(std::is_base_of<Event, T>::value, "EventHandler<T>: T must be a class derived from Event");
        }

        virtual ~EventHandler() { }

        virtual void onEvent(T&) = 0;

        void dispatch(Event& e)
        {
            onEvent(dynamic_cast<T&>(e));
        }
    };

    class HandlerRegistration : public Object {
    public:
        virtual ~HandlerRegistration() { }

        virtual void removeHandler() = 0;
    };

    class EventBus : public Object {
    public:
        EventBus() { }
        virtual ~EventBus() { }
        static EventBus* const GetInstance()
        {
            if (instance==nullptr) {
                instance = new EventBus();
            }

            return instance;
        }
        template<class T>
        static HandlerRegistration* const AddHandler(EventHandler<T>& handler, Object& sender)
        {
            EventBus* instance = GetInstance();
            Registrations* registrations = instance->handlers[typeid(T)];
            if (registrations==nullptr) {
                registrations = new Registrations();
                instance->handlers[typeid(T)] = registrations;
            }
            EventRegistration* registration = new EventRegistration(static_cast<void*>(&handler), registrations,
                    &sender);
            registrations->push_back(registration);
            return registration;
        }
        template<class T>
        static HandlerRegistration* const AddHandler(EventHandler<T>& handler)
        {
            EventBus* instance = GetInstance();
            Registrations* registrations = instance->handlers[typeid(T)];
            if (registrations==nullptr) {
                registrations = new Registrations();
                instance->handlers[typeid(T)] = registrations;
            }
            EventRegistration* registration = new EventRegistration(static_cast<void*>(&handler), registrations,
                    nullptr);
            registrations->push_back(registration);
            return registration;
        }
        static void FireEvent(Event& e)
        {
            EventBus* instance = GetInstance();
            Registrations* registrations = instance->handlers[typeid(e)];
            if (registrations==nullptr) {
                return;
            }
            for (auto& reg : *registrations) {
                if ((reg->getSender()==nullptr) || (reg->getSender()==&e.getSender())) {
                    static_cast<EventHandler<Event>*>(reg->getHandler())->dispatch(e);
                }
            }
        }

    private:
        static EventBus* instance;
        class EventRegistration : public HandlerRegistration {
        public:
            typedef std::list<EventRegistration*> Registrations;
            EventRegistration(void* const handler, Registrations* const registrations, Object* const sender)
                    :
                    handler(handler),
                    registrations(registrations),
                    sender(sender),
                    registered(true) { }
            virtual ~EventRegistration() { }
            void* const getHandler()
            {
                return handler;
            }
            Object* const getSender()
            {
                return sender;
            }
            virtual void removeHandler()
            {
                if (registered) {
                    registrations->remove(this);
                    registered = false;
                }
            }

        private:
            void* const handler;
            Registrations* const registrations;
            Object* const sender;

            bool registered;
        };

        typedef std::list<EventRegistration*> Registrations;
        typedef std::unordered_map<std::type_index, std::list<EventRegistration*>*> TypeMap;

        TypeMap handlers;

    };

};
