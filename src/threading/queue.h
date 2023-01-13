#ifndef __QUEUE_HEADER__
#define __QUEUE_HEADER__

#include <algorithm>
#include <deque>
#include <list>
#include <memory>
#include <mutex>

// Thread-safe, type-safe event manager

namespace pd2hook
{
class IEventQueue
{
public:
	virtual ~IEventQueue() {}
	virtual void ProcessEvents() = 0;
};

class EventQueueMaster
{
	template<typename T>
	friend class EventQueue;

private:
	EventQueueMaster() = default;

public:
	static EventQueueMaster& GetSingleton();

	void ProcessEvents();

private:
	void registerQueue(IEventQueue *queue);

	std::list<IEventQueue *> queues;
};

template<typename DataT>
class EventQueue : public IEventQueue
{
public:
	typedef void(*EventFunction)(DataT);

	class EventItem
	{
	public:
		EventItem(EventFunction runFunction, DataT data);
		EventItem(EventItem&&); // VC++ 2013 doesn't let you default this. On VC++ 2015 don't even bother declaring.
		void operator()();

	private:
		EventItem(const EventItem&) = delete;

		EventFunction mFunc;
		DataT mData;
	};

	static EventQueue& GetSingleton();

protected:
	EventQueue();

public:
	virtual void ProcessEvents() override;
	void AddToQueue(EventItem item);
	void AddToQueue(EventFunction runFunction, DataT data);

private:
	std::deque<EventItem> eventQueue;
	std::mutex lock;
};

template<typename DataT>
struct EventQueueRuntimeRegisterer
{
	EventQueueRuntimeRegisterer() { EventQueue<DataT>::GetSingleton(); }
};

#define PD2HOOK_CONCAT_IMPL(x, y) x ## y
#define PD2HOOK_CONCAT(x, y) PD2HOOK_CONCAT_IMPL(x, y)

// Not strictly necessary, but is a nice chance to make sure the static instance is initialised before there's a chance for multithreaded calls
#define PD2HOOK_REGISTER_EVENTQUEUE(DataT, Name) \
	namespace { EventQueueRuntimeRegisterer<DataT> PD2HOOK_CONCAT(staticRegisterer, __COUNTER__); EventQueue<DataT>& Get##Name##Queue() { return EventQueue<DataT>::GetSingleton(); } }
#define PD2HOOK_REGISTER_EVENTQUEUE_EASY(DataT) PD2HOOK_REGISTER_EVENTQUEUE(DataT, DataT)

#pragma region Implementation

template<typename DataT>
EventQueue<DataT>::EventQueue()
{
	EventQueueMaster::GetSingleton().registerQueue(this);
}

template<typename DataT>
EventQueue<DataT>::EventItem::EventItem(EventFunction runFunction, DataT data) :
	mFunc(runFunction), mData(std::move(data))
{}

template<typename DataT>
EventQueue<DataT>::EventItem::EventItem(EventItem&& mv) :
	mFunc(mv.mFunc), mData(std::move(mv.mData))
{}

template<typename DataT>
void EventQueue<DataT>::EventItem::operator()()
{
	mFunc(std::move(mData));
}

template<typename DataT>
EventQueue<DataT>& EventQueue<DataT>::GetSingleton()
{
	static EventQueue<DataT> instance;
	return instance;
}

template<typename DataT>
void EventQueue<DataT>::ProcessEvents()
{
	decltype(eventQueue) localQueue;
	{
		std::lock_guard<std::mutex> locker(lock);
		// localQueue = std::move(eventQueue); standard is a little iffy on what happens to eventQueue after this, so do it manually
		while (!eventQueue.empty())
		{
			localQueue.push_back(std::move(eventQueue.front()));
			eventQueue.pop_front();
		}
	}

	std::for_each(localQueue.begin(), localQueue.end(), [](EventItem& e) { e(); });
}

template<typename DataT>
void EventQueue<DataT>::AddToQueue(EventItem item)
{
	std::lock_guard<std::mutex> locker(lock);
	eventQueue.push_back(std::move(item));
}

template<typename DataT>
void EventQueue<DataT>::AddToQueue(EventFunction runFunction, DataT data)
{
	std::lock_guard<std::mutex> locker(lock);
	eventQueue.emplace_back(runFunction, std::move(data));
}

#pragma endregion

}

#endif // __QUEUE_HEADER__